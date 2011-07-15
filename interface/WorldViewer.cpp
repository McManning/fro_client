
#include <algorithm>
#include "WorldViewer.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/widgets/WidgetList.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Input.h"
#include "../core/io/FileIO.h"
#include "../game/GameManager.h"

void callback_reloadWorlds(Button* b)
{
	WorldViewer* viewer = (WorldViewer*)b->GetParent();

	if (viewer)
	{
		viewer->RequestWorldList(viewer->mCurrentListType);
	}
}

void callback_joinWorld(Button* b)
{
	// b->mId = world ID to join
	game->LoadOnlineWorld(b->mId);

	Widget* wv = gui->Get("WorldViewer");
	if (wv)
		wv->Die();
}

void callback_tryClose(Button* b)
{
	// only let them close it if we're not between worlds
	if (game->mMap)
		b->GetParent()->Die();
}

void callback_sortByName(Button* b)
{
	WorldViewer* viewer = (WorldViewer*)b->GetParent();
	if (viewer)
		viewer->SortWorldData(WorldViewer::SORT_NAME);
}

void callback_sortByUsers(Button* b)
{
	WorldViewer* viewer = (WorldViewer*)b->GetParent();
	if (viewer)
		viewer->SortWorldData(WorldViewer::SORT_USERS);
}

void callback_filterChanged(Input* i)
{
	WorldViewer* viewer = (WorldViewer*)i->GetParent();
	if (viewer)
		viewer->UpdateWorldList();
}

bool worldDataSortByUsers( WorldViewer::worldData a, WorldViewer::worldData b )
{
	return a.users > b.users;
}

bool worldDataSortByName( WorldViewer::worldData a, WorldViewer::worldData b )
{
	return a.name < b.name;
}

void dlCallback_worldDataSuccess(downloadData* data)
{
	WorldViewer* viewer = (WorldViewer*)data->userData;

	if (viewer)
	{
		viewer->LoadWorldDataFromFile(data->filename);
	}

	removeFile(data->filename);
}

void dlCallback_worldDataFailure(downloadData* data)
{
	WorldViewer* viewer = (WorldViewer*)data->userData;

	if (viewer)
	{
		viewer->FailedToLoadWorldData("Could not download list");
	}
}

WorldViewer::WorldViewer()
	: Frame(gui, "WorldViewer", rect(0, 0, 400, 400), "Pick A World, Any World...", true, false, true, true)
{
	Label* l;
	Button* b;

	mRecommendedFrame = NULL;

	// override our close callback to check if it really is legal to close this
	mClose->onClickCallback = callback_tryClose;

	new Label(this, "", rect(5, 30), "Recommended World:");

	mWorldList = new WidgetList(this, rect(5, 105, Width() - 10, Height() - (105+35)));

	mReloadButton = new Button(this, "", rect(5, Height() - 25, 20, 20), "", callback_reloadWorlds);
		mReloadButton->mHoverText = "Reload List";
		mReloadButton->SetImage("assets/buttons/reload_worlds.png");

	b = new Button(this, "", rect(30, Height() - 25, 20, 20), "", callback_sortByName);
		b->mHoverText = "Sort by name";
		b->SetImage("assets/buttons/sort_name.png");

	b = new Button(this, "", rect(50, Height() - 25, 20, 20), "", callback_sortByUsers);
		b->mHoverText = "Sort by users";
		b->SetImage("assets/buttons/sort_users.png");

	/*
	b = new Button(this, "", rect(50, Height() - 25, 20, 20), "", callback_sortByUsers);
		b->mHoverText = "All Worlds";
		b->SetImage("assets/buttons/go.png");

	b = new Button(this, "", rect(50, Height() - 25, 20, 20), "", callback_sortByUsers);
		b->mHoverText = "Official Worlds";
		b->SetImage("assets/buttons/go.png");

	b = new Button(this, "", rect(50, Height() - 25, 20, 20), "", callback_sortByUsers);
		b->mHoverText = "User Created Worlds";
		b->SetImage("assets/buttons/go.png");
	*/

	l = new Label(this, "", rect(80, Height() - 25), "Filter:");

	mFilterInput = new Input(this, "filter", rect(80 + l->Width()+5, Height() - 25, 150, 20), "", 0, true, NULL);
		mFilterInput->onChangeCallback = callback_filterChanged;
		mFilterInput->mHoverText = "Use * as a wildcard";
		//mFilterInput->SetText("*");

	RequestWorldList(LIST_ALL);

	Center();
}

WorldViewer::~WorldViewer()
{
	downloader->NullMatchingUserData(this);
}

void WorldViewer::SortWorldData(sortType sort)
{
	mCurrentSortType = sort;

	if (!mWorldData.empty())
	{
		switch (sort)
		{
			case SORT_USERS:
				stable_sort(mWorldData.begin(), mWorldData.end(), worldDataSortByUsers);
				break;
			default:
				stable_sort(mWorldData.begin(), mWorldData.end(), worldDataSortByName);
				break;
		}
		UpdateWorldList();
	}

}

/*
	File Format:
	name:id:count:desc\n
	name:id:count:desc\n
*/
bool WorldViewer::LoadWorldDataFromFile(string filename)
{
	string data;
	vString v;
	vString line;
	worldData wd;

	mWorldData.clear();

	if (!fileToString(data, filename))
	{
		FailedToLoadWorldData("Could not load data file");
		return false;
	}

	replace(&data, "\r", "");
	explode(&v, &data, "\n");

	for (int i = 0; i < v.size(); ++i)
	{
		line.clear();
		explode(&line, &v.at(i), ":");
		if (line.size() > 3)
		{
			wd.name = line.at(0);
			wd.id = line.at(1);
			wd.users = sti(line.at(2));
			wd.description = line.at(3);
			mWorldData.push_back(wd);

			if (i == 0) // first item is our "recommended world"
			{
				if (mRecommendedFrame)
					mRecommendedFrame->Die();

				mRecommendedFrame = CreateWorldInfoFrame(Width() - 10, mWorldData.at(i));
				Add(mRecommendedFrame);
				mRecommendedFrame->SetPosition(rect(5, 50, Width() - 10, mRecommendedFrame->Height()));
			}
		}
	}

	SortWorldData(mCurrentSortType);
	UpdateWorldList();
	SetControlState(true);

	return true;
}

void WorldViewer::FailedToLoadWorldData(string reason)
{
	SetControlState(true);
}

Frame* WorldViewer::CreateWorldInfoFrame(int width, worldData& data)
{
	Frame* f;
	Button* b;
	Label* l;

	f = new Frame(NULL, "", rect(0, 0, width, 50));

	l = new Label(f, "", rect(5, 5), "");
		l->mFont = fonts->Get("", 14, TTF_STYLE_BOLD);
		l->SetCaption(data.name); // + " @ " + mWorldData.at(i).id);

	l = new Label(f, "", rect(0, 5), "");
		l->mFont = fonts->Get("", 12, TTF_STYLE_NORMAL);
		l->SetCaption("(Users: " + its(data.users) + ")");
		l->SetPosition(rect(f->Width() - l->Width() - 5, 5, l->Width(), l->Height()));

	l = new Label(f, "", rect(5, 30), "");
		l->mFont = fonts->Get("", 12, TTF_STYLE_NORMAL);
		l->SetCaption(data.description);

	b = new Button(f, data.id, rect(f->Width() - 25, f->Height() - 25, 20, 20),
					"", callback_joinWorld);
		b->mHoverText = "Join " + data.name;
		b->SetImage("assets/buttons/join_world.png");

	return f;
}

void WorldViewer::UpdateWorldList()
{
	// clear mWorldList, sort our list, display all worlds that match current filters

	mWorldList->RemoveAll();

	Frame* f;
	string filter = "*" + lowercase(mFilterInput->GetText()) + "*";

	for (int i = 0; i < mWorldData.size(); ++i)
	{
		if (wildmatch(filter.c_str(), lowercase(mWorldData.at(i).name).c_str())
			|| wildmatch(filter.c_str(), lowercase(mWorldData.at(i).description).c_str()))
		{
			f = CreateWorldInfoFrame(mWorldList->GetMaxChildWidth(), mWorldData.at(i));
			mWorldList->Add(f);
		}
	}
}

void WorldViewer::RequestWorldList(listType type)
{
	string query;

	mCurrentListType = type;

	//send http get: worlds.php?ver=1.2.3&id=test&pass=test&type=???

	query = "http://sybolt.com/drm-svr/"; // TODO: less hardcoding
	query += "worldlist.php?ver=";
	query += VER_STRING;
	query += "&user=" + htmlSafe(game->mUsername);
	query += "&pass=" + htmlSafe(game->mPassword);
	query += "&type=" + its(mCurrentListType);

	downloader->QueueDownload(query, getTemporaryCacheFilename(),
									this, dlCallback_worldDataSuccess,
									dlCallback_worldDataFailure, true);
	SetControlState(false);
}

void WorldViewer::SetControlState(bool active)
{
	mReloadButton->SetActive(active);
	mFilterInput->SetActive(active);
}
