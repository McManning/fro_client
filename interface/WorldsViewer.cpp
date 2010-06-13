
#include "WorldsViewer.h"
#include "../game/GameManager.h"
#include "../core/net/IrcNet2.h"
#include "../core/io/FileIO.h"
#include "../core/widgets/MessagePopup.h"
#include "../map/Map.h"
#include "../entity/LocalActor.h"

/* 
<worlds>
	<world id="wonderland" channel="#drm.wonderland">Description</world>
	<world id="library" channel="#drm.wonderland">Description</world>
	...
</worlds>
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
		i.channel = xf->GetParamString(e, "channel");
		i.name = xf->GetParamString(e, "id");
		i.description = xf->GetText(e);
		i.usercount = 0;
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
	
	// Now ask the irc server to update us
	if (viewer)
	{
		viewer->RefreshUserCounts();
		viewer->mRefresh->SetActive(true);
	}
	
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

void listener_NetChannelCount(MessageListener* ml, MessageData& md, void* sender)
{
	WorldsViewer* viewer = (WorldsViewer*)ml->userdata;
	if (viewer)
	{
		console->AddMessage("Channel: " + md.ReadString("channel") 
							+ " Count " + its(md.ReadInt("count")));
		viewer->UpdateChannelCount(md.ReadString("channel"), md.ReadInt("count"));
	}
}

void callback_RefreshWorlds(Button* b)
{
	WorldsViewer* viewer = (WorldsViewer*)b->GetParent();
	if (viewer)
		viewer->RefreshWorlds();
}

WorldsViewer::WorldsViewer() :
	Frame(gui, "worldsviewer", rect(0, 0, 500, 400), "Worlds Viewer", true, true, true, true)
{
	Center();

	mDefaultIcon = NULL; //resman->LoadImg("assets/world_unknown.png");
	
	mListFrame = new Frame(this, "", rect(5, 30, Width()-30, Height()-60));

	mRefresh = new Button(this, "", rect(0,0,20,20), "", NULL);
		mRefresh->mHoverText = "Refresh List";
		mRefresh->SetImage("assets/buttons/refresh.png");
		mRefresh->onClickCallback = callback_RefreshWorlds;
		
	int max = mWorlds.size()-1;
	if (max < 0)
		max = 0;
	mScroller = new Scrollbar(this, "", rect(5+mListFrame->Width(), 30, 20, mListFrame->Height()), 
								VERTICAL, max, 1, 0, NULL);
	
	ResizeChildren(); //get them into position
	
	messenger.AddListener("NET_CHANNEL_COUNT", listener_NetChannelCount, NULL, this);
	
	RefreshWorlds();
}

WorldsViewer::~WorldsViewer()
{
	for (uShort i = 0; i < mWorlds.size(); i++)
		resman->Unload(mWorlds.at(i).icon);	

	resman->Unload(mDefaultIcon);
	
	messenger.RemoveListener("NET_CHANNEL_COUNT", this);
	downloader->NullMatchingUserData(this);
}

void WorldsViewer::RefreshWorlds()
{
	// Don't let them repeat the download over and over
//	if (downloader->CountMatchingUserData(this) > 0)
//		return;
	mWorlds.clear();
	
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

	//generate the request url  MASTER_URL?list=1&loc=worldname&nick=something&user=something&pass=something
	url += "?list=1";
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

void WorldsViewer::UpdateChannelCount(string channel, int count)
{
	for (int i = 0; i < mWorlds.size(); ++i)
	{
		if (mWorlds.at(i).channel == channel)
		{
			mWorlds.at(i).usercount = count;
			return;
		}
	}
}

void WorldsViewer::RefreshUserCounts()
{
	// Construct a LIST command for all channels
	if (game->mNet && game->mNet->IsConnected())
	{
		string channels;
		for (int i = 0; i < mWorlds.size(); ++i)
			channels += mWorlds.at(i).channel + ",";
			
		console->AddMessage("LIST " + channels);
		game->mNet->Rawmsg("LIST " + channels);	
	}
}

const int ICON_SIZE = 30;

// Returns the y position of where to render the next achievement under this
uShort WorldsViewer::_renderSingle(rect r, uShort index)
{
	Image* scr = Screen::Instance();

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
		string msg = mWorlds.at(index).name + " \\c333(Users: ";
		msg += its(mWorlds.at(index).usercount);
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
	/*if (descFont)
	{
		
		descFont->Render( scr, r.x + ICON_SIZE + 10, y, msg, color(100, 100, 100) );
		y += descFont->GetHeight();
	}*/
	
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

void WorldsViewer::ResizeChildren()
{
	mListFrame->SetPosition( rect(5, 30, Width()-30, Height()-60) );
	mScroller->SetPosition( rect(5+mListFrame->Width(), 30, 20, mListFrame->Height()) );
	mRefresh->SetPosition( rect(5, Height() - 25, 20, 20) );
	
	Frame::ResizeChildren();
}

void WorldsViewer::SetPosition(rect r)
{
	if (r.h >= 200 && r.w >= 300)
		Frame::SetPosition(r);	
}

