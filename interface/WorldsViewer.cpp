
#include "WorldsViewer.h"
#include "../game/GameManager.h"
#include "../core/net/IrcNet2.h"
#include "../core/io/FileIO.h"
#include "../core/widgets/MessagePopup.h"
#include "../map/Map.h"
#include "../entity/LocalActor.h"

/* 
<worlds>
	<!-- 
		title: User-created ID for the world
		id: Channel ID when we request a connection to the world
		rate: Rating of the map as voted by its users (can vote while on a map)
		users: Current online users, with accuracy of around 10min or so, depending on how often users phone home
		description: Whatever the user wrote to describe his world. It should also force embed a Created by Username\n in it
					with some different color
	-->
	<world title="The Library" id="library" rate="5" users="2">Description</world>
	...
</worlds>

	TODO: A way to click & join.
		Can either grab mouse position on a click within this widget and calculate the infobar we're current residing..
		or make each bar its own widget
			Good: Position info done for us when updating the screen
			Bad: have to manually toggle visibility for the widgets, change positions on scroller update, 
				

*/
int callback_worldsXmlParser(XmlFile* xf, TiXmlElement* e, void* userData)
{
	WorldsViewer* viewer = (WorldsViewer*)userData;
	if (!viewer)
		return XMLPARSE_SUCCESS; //silent ignore
		
	string id = e->Value();

	if (id == "world")
	{
		WorldsViewer::worldInfo i;
		i.title = xf->GetParamString(e, "title");
		i.id = xf->GetParamString(e, "id");
		i.usercount = xf->GetParamInt(e, "users");
		i.rating = xf->GetParamInt(e, "rate");
		i.description = xf->GetText(e);
		i.icon = NULL;
		viewer->mWorlds.push_back(i);
	}
	
	return XMLPARSE_SUCCESS;
}

void dlCallback_worldsXmlSuccess(downloadData* data)
{
	WorldsViewer* viewer = (WorldsViewer*)data->userData;
	
	string error;
	
	XmlFile xf;
	xf.SetParser(callback_worldsXmlParser);

	if (!xf.LoadFromFile(data->filename))
	{
		error = "Invalid worlds xml file.";
	}
	else
	{
		xf.SetEntryPoint("worlds");
		if (xf.Parse(data->userData) != XMLPARSE_SUCCESS)
			error = "Error while parsing worlds Xml.";
	}
	if (viewer)
		viewer->mRefresh->SetActive(true);

	removeFile(data->filename);
	
	if (!error.empty())
	{
		console->AddMessage(error);
		new MessagePopup("worldserror", "Worlds List Error", error);
	}
}

void dlCallback_worldsXmlFailure(downloadData* data)
{
	WorldsViewer* viewer = (WorldsViewer*)data->userData;
	if (!viewer)
		return;
		
	viewer->mRefresh->SetActive(true);
	
	string error = "Could not retrieve world information. Error was that the ";

	switch (data->errorCode)
	{
		case DEC_BADHOST:
			error += "server address was invalid.";
			break;
		case DEC_CONNECTFAIL:
			error += "network could not connect to the server.";
			break;
		case DEC_FILENOTFOUND:
			error += "xml could not be located.";
			break;
		default:
			error += "downloader experienced an unknown error.";
			break;
	}

	console->AddMessage(error);
	new MessagePopup("worldserror", "Worlds List Error", error);
}

void callback_RefreshWorlds(Button* b)
{
	WorldsViewer* viewer = (WorldsViewer*)b->GetParent();
	if (viewer)
		viewer->RefreshWorlds(viewer->mCurrentView);
}

void callback_LoadOffical(Button* b)
{
	WorldsViewer* viewer = (WorldsViewer*)b->GetParent();
	if (viewer)
		viewer->RefreshWorlds(WorldsViewer::VIEW_OFFICIAL);
}

void callback_LoadPersonals(Button* b)
{
	WorldsViewer* viewer = (WorldsViewer*)b->GetParent();
	if (viewer)
		viewer->RefreshWorlds(WorldsViewer::VIEW_PERSONAL);
}

WorldsViewer::WorldsViewer() :
	Frame(gui, "worldsviewer", rect(0, 0, 500, 400), "[INDEV] Worlds Viewer", true, false, true, true)
{
	Center();

	mDefaultIcon = NULL; //resman->LoadImg("assets/world_unknown.png");
	
	mOfficialList = new Button(this, "", rect(5, 30, 70, 20), "Official", NULL);
		mOfficialList->onClickCallback = callback_LoadOffical;
	mPersonalsList = new Button(this, "", rect(80, 30, 70, 20), "Personals", NULL);
		mPersonalsList->onClickCallback = callback_LoadPersonals;
	//mFavoritesList = new Button(this, "Favorites", rect(0,0,20,20), "", NULL);
	
	mListFrame = new Frame(this, "", rect(5, 55, Width()-30, Height()-85));
	mRefresh = new Button(this, "", rect(5, Height() - 25, 20, 20), "", NULL);
		mRefresh->mHoverText = "Refresh List";
		mRefresh->SetImage("assets/buttons/refresh.png");
		mRefresh->onClickCallback = callback_RefreshWorlds;

	mScroller = new Scrollbar(this, "", rect(5+mListFrame->Width(), 30, 20, mListFrame->Height()), 
								VERTICAL, 0, 1, 0, NULL);
	
	ResizeChildren(); //get them into position

	RefreshWorlds(VIEW_OFFICIAL);
}

WorldsViewer::~WorldsViewer()
{
	for (uShort i = 0; i < mWorlds.size(); i++)
		resman->Unload(mWorlds.at(i).icon);	

	resman->Unload(mDefaultIcon);

	downloader->NullMatchingUserData(this);
}

void WorldsViewer::RefreshWorlds(int type)
{
	// Don't let them repeat the download over and over
//	if (downloader->CountMatchingUserData(this) > 0)
//		return;

	// Don't let them refresh if it's already refreshing
	if (!mRefresh->IsActive())
		return;

	mWorlds.clear();
	mCurrentView = type;
	
	mRefresh->SetActive(false);
	
	string url;
	string file;
	XmlFile xf;
	TiXmlElement* e;
	
	if (!xf.LoadFromFile("assets/connections.cfg"))
	{
		FATAL(xf.GetError());	
	}
	
	e = xf.mDoc.FirstChildElement();
	if (e)
		e = e->FirstChildElement("master");
	
	if (e)
		url = xf.GetText(e);
	
	if (url.empty() || url.find("http://", 0) != 0)
	{
		FATAL("Invalid master address");
	}

	//generate the request url  MASTER_URL?list=TYPE&loc=worldname&nick=something&user=something&pass=something
	url += "?list=" + its(type);
	if (game->mMap)
		url += "&loc=" + htmlSafe(game->mMap->mId);
	url += "&nick=" + htmlSafe(game->mPlayer->mName);
	
	if (!game->mUsername.empty())
		url += "&user=" + htmlSafe(game->mUsername) + "&pass=" + htmlSafe(game->mPassword);

	file = DIR_CACHE + string("worlds.xml");
	downloader->QueueDownload(url, file, 
								this, 
								dlCallback_worldsXmlSuccess, 
								dlCallback_worldsXmlFailure,
								true, true);
}

const int ICON_SIZE = 30;

// Returns the y position of where to render the next achievement under this
int WorldsViewer::_renderSingle(rect r, int index)
{
	Image* scr = Screen::Instance();
	string msg;
	
	//Render background color
	if (index % 2)
		scr->DrawRect( r, color(200,200,200) );
	else
		scr->DrawRect( r, color(220,220,220) );
	
	if (mWorlds.at(index).icon)
		mWorlds.at(index).icon->Render( scr, r.x+5, r.y+10, rect(0,0,ICON_SIZE,ICON_SIZE) );
	else if (mDefaultIcon)
		mDefaultIcon->Render( scr, r.x+5, r.y+10, rect(0,0,ICON_SIZE,ICON_SIZE) );	

	sShort y = r.y + 5;
	
	// Render name
	Font* titleFont = fonts->Get("", 18, TTF_STYLE_BOLD);
	color c;
	if (titleFont)
	{
		// TODO: Render stars or something, instead of text for the rating
		msg = mWorlds.at(index).title + " \\c333(Rating: ";
		msg += its(mWorlds.at(index).rating);
		msg += ")";
			
		titleFont->Render( scr, r.x + ICON_SIZE + 10, y, msg, color() );
		y += titleFont->GetHeight() + 3;
	}
	
	// Render description 
	Font* descFont = fonts->Get("", 10);
	if (descFont)
	{
		descFont->Render( scr, r.x + ICON_SIZE + 10, y, mWorlds.at(index).description, 
							color(100,100,100), r.w - (ICON_SIZE + 15) );
		y += descFont->GetHeight(mWorlds.at(index).description, r.w - (ICON_SIZE + 15));
	}
	
	// Render number of users, if we know it
	if (descFont)
	{
		msg = "(Users: ";
		msg += its(mWorlds.at(index).usercount);
		msg += ")";
		
		descFont->Render( scr, r.x + ICON_SIZE + 10, y, msg, color(100, 100, 100) );
		y += descFont->GetHeight();
	}
	
	return y + 5;
}

void WorldsViewer::Render()
{
	Frame::Render();
	
	if (mResizing)
		return;
	
	Image* scr = Screen::Instance();
	rect oldclip = scr->GetClip();
	rect r = mListFrame->GetScreenPosition();

	//contain our achievements listing
	scr->SetClip(r);
	
	if (mWorlds.empty())
	{
		if (mFont)
			mFont->Render( scr, r.x + 10, r.y + 10, "Fetching worlds list from the Master...", color(128) );	
	}
	else
	{
		//render each visible one
		for (int i = mScroller->GetValue(); i < mWorlds.size(); ++i)
		{
			if (i < 0) continue;
			
			r.y = _renderSingle(r, i);
			
			//if we rendered them off the available area, we're done.
			if (r.y >= mListFrame->GetScreenPosition().y + r.h)
				break;
		}
	}

	scr->SetClip(oldclip);
}
