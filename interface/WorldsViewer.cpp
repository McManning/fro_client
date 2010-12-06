
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
		viewer->AddWorld(i);
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
	{
		viewer->mRefresh->SetActive(true);
		viewer->mList->SetVisible(true);
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
	viewer->mList->SetVisible(true);
	
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

	mOfficialList = new Button(this, "", rect(5, 30, 70, 20), "Official", NULL);
		mOfficialList->onClickCallback = callback_LoadOffical;
	mPersonalsList = new Button(this, "", rect(80, 30, 70, 20), "Personals", NULL);
		mPersonalsList->onClickCallback = callback_LoadPersonals;
	//mFavoritesList = new Button(this, "Favorites", rect(0,0,20,20), "", NULL);

	mList = new WidgetList(this, rect(5, 55, Width()-30, Height()-85));
	
	mRefresh = new Button(this, "", rect(5, Height() - 25, 20, 20), "", NULL);
		mRefresh->mHoverText = "Refresh List";
		mRefresh->SetImage("assets/buttons/refresh.png");
		mRefresh->onClickCallback = callback_RefreshWorlds;

	ResizeChildren(); //get them into position

	RefreshWorlds(VIEW_OFFICIAL);
}

WorldsViewer::~WorldsViewer()
{
	for (uShort i = 0; i < mWorlds.size(); i++)
		resman->Unload(mWorlds.at(i).icon);	

	downloader->NullMatchingUserData(this);
}

void WorldsViewer::RefreshWorlds(int type)
{
	// Don't let them refresh if it's already refreshing
	if (!mRefresh->IsActive())
		return;

	mWorlds.clear();
	mCurrentView = type;
	
	mRefresh->SetActive(false);
	
	//mRefreshingNotice->SetVisible(true);
	mList->SetVisible(false);
	
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
