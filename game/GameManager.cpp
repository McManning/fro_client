
#ifdef WIN32
#include <windows.h> //for Winamp support
#endif

#include "GameManager.h"
//#include "Achievements.h"
#include "IrcNetListeners.h"
#include "CollisionBlob.h"
#include "../map/BasicMap.h"
#include "../entity/LocalActor.h"
#include "../entity/ExplodingEntity.h"
#include "../entity/Avatar.h"
#include "../core/widgets/Console.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/Scrollbar.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/OpenUrl.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/net/IrcNet2.h"
#include "../core/io/BoltFile.h"
#include "../core/io/FileIO.h"

#include "../interface/LoginDialog.h"
#include "../interface/Inventory.h"
#include "../interface/UserList.h"
#include "../interface/AvatarFavoritesDialog.h"
#include "../interface/OptionsDialog.h"
//#include "../interface/MyAchievements.h"
#include "../interface/AvatarCreator.h"
//#include "../interface/MiniMenu.h"
//#include "../interface/LunemParty.h"
//#include "../interface/WorldsViewer.h"
//#include "../interface/ScreenText.h"

//#include "../backpack/Backpack.h"

GameManager* game;

/*
// Load the file, assuming it's a 32x64 framed avatar with 5 rows and 2 columns, will 
// replace the file with a new version that is 40x80
void ResizeAvagenFile(const string& file)
{
	int x, y, dx, dy;
	Image* img;
	Image* result;
	
	img = resman->LoadImg(file);
	if (!img) return;
	
	// assuming they're all pink backgrounded
	
	result = resman->NewImage(40*2, 80*5, color(0,0,0), true);
	if (!result) { resman->Unload(img); return; }

	result->DrawRect(rect(0, 0, result->Width(), result->Height()), color(255,0,255), true);

	dy = 16;
	for (y = 0; y < img->Height(); y+=64)
	{
		dx = 4;
		for (x = 0; x < img->Width(); x+=32)
		{
			img->Render(result, dx, dy, rect(x, y, 32, 64));
			dx += 40;
		}
		dy += 80;
	}

	result->SavePNG(file);
	
	resman->Unload(img);
	resman->Unload(result);
}

// Resize everything from 32x64 frames to 40x80
void ResizeAllAvagenParts()
{
	// Grab a list of all the files we're about to fuck with
	vString files;
	string pattern = "assets/ava/*.png";
	
	getFilesMatchingPattern(files, pattern);

	for (int i = 0; i < files.size(); ++i)
	{
		files.at(i) = "assets/ava/" + files.at(i);
		ResizeAvagenFile(files.at(i));
	}
}
*/

// Console Command Callbacks

void callback_privmsgNoCommand(Console* c, string s)
{
	if (!game->mNet || !game->mNet->IsConnected())
	{
		c->AddMessage("\\c900 * Not Connected!");
		return;
	}
	
	string nick = c->mId.substr(4); //strip out 'priv' from beginning

	game->mNet->Privmsg(nick, s);
	c->AddMessage(game->mPlayer->mName + ": " + s);
}

void callback_privmsgCommandWhois(Console* c, string s)
{
	if (!game->mNet || !game->mNet->IsConnected())
	{
		c->AddMessage("\\c900 * Not Connected!");
		return;
	}
	
	string nick = c->mId.substr(4); //strip out 'priv' from beginning

	game->mNet->Whois(nick);
}

void callback_consoleNetInfo(Console* c, string s)
{
	string ss;
	game->mNet->StateToString(ss);
	c->AddMessage(ss);	
}

void callback_consoleOutputAvatar(Console* c, string s)
{
	game->mPlayer->GetAvatar()->ToFiles();
}

void callback_consoleTestMap(Console* c, string s) //test <file>
{
	if (s.length() < 10)
	{
		c->AddMessage("Syntax: test id");
		return;
	}
	
	string id = s.substr(5);
	
	ASSERT(game);
	game->LoadTestWorld(id);
}

void callback_consoleScreenDraw(Console* c, string s) //screendraw
{
	Screen::Instance()->mNoDraw = !Screen::Instance()->mNoDraw;
}

void callback_consoleMakeCol(Console* c, string s) // makecol filename
{
	vString v;
	explode(&v, &s, " ");

	if (s.size() > 8)
	{
		createCollisionBlob(s.substr(8), true);
	}
	else
	{
		console->AddMessage("Syntax: makecol <filename>");
	}
}

void callback_consolePlayerFlags(Console* c, string s)
{
	ASSERT(game && game->mPlayer);
	game->mPlayer->PrintFlags();
}

// GameManager callbacks

uShort timer_gameManagerProcess(timer* t, uLong ms)
{
	GameManager* g = (GameManager*)t->userData;
	ASSERT(g);
	g->Process(ms);	
	return TIMER_CONTINUE;
}

uShort timer_DestroyInfoBar(timer* t, uLong ms)
{
	if (t->userData)
	{
		Frame* f = (Frame*)t->userData;
		
		MessageData md("INFOBAR");
		md.WriteString("text", f->mId);
		messenger.Dispatch(md);
		
		f->Die();
	}
	
	return TIMER_DESTROY;
}

GameManager::GameManager()
	: Frame( gui, "GameManager", rect(0,0,gui->Width(),gui->Height()) )
{
	mMap = NULL;
	mPlayer = NULL;
	mNet = NULL;
	mLoader = NULL;
	//mParty = NULL;
	
	PRINT("[GM] Starting");
	
	game = this;
	
	buildDirectoryTree(DIR_PROFILE);
	
	UpdateAppTitle();
	
	//TODO: this is temp, to ensure we don't have multiple GMs running.
	ASSERT( gui->Get("GameManager") == this );

	PRINT("[GM] Loading Player Data");
		
	LoadPlayerData();

	mMap = NULL;
	mShowJoinParts = mPlayerData.GetParamInt("map", "joinparts");
	mShowAddresses = mPlayerData.GetParamInt("map", "addresses");

	PRINT("[GM] Loading Network");
	mNet = new IrcNet();
	mNet->mRealname = "guest";
	
	PRINT("[GM] Hooking Network");
	hookNetListeners();

	PRINT("[GM] Loading Local Actor");
	mPlayer = new LocalActor();
	mPlayer->mName = mPlayerData.GetParamString("user", "nick");
	if (mPlayer->mName.empty())
		mPlayer->mName = "fro_user";

	timers->AddProcess("gameproc", 
						timer_gameManagerProcess, 
						NULL, 
						this);

	PRINT("[GM] Loading Inventory");

	//mParty = new LunemParty;
	//mParty->SetVisible(false);

	PRINT("[GM] Loading HUD");
	_buildHud();
	
	ResizeChildren();

	PRINT("[GM] Bringing up Login");

	new LoginDialog();

	PRINT("[GM] Finished");
	
	ToggleGameMode(MODE_ACTION);

	//Backpack* pack = new Backpack();

	console->HookCommand("net_info", callback_consoleNetInfo);
	console->HookCommand("avatarout", callback_consoleOutputAvatar);
	console->HookCommand("test", callback_consoleTestMap);
	console->HookCommand("screendraw", callback_consoleScreenDraw);
	console->HookCommand("makecol", callback_consoleMakeCol);
	console->HookCommand("player_flags", callback_consolePlayerFlags);
}

GameManager::~GameManager()
{
	PRINT("~GameManager 1");
	
	UnloadMap();

	TiXmlElement* e = mPlayerData.mDoc.FirstChildElement("data")->FirstChildElement("chat");
	mPlayerData.SetParamRect(e, "position", mChat->GetPosition());
	
	PRINT("~GameManager 2");

	//erase all entities before it gets deleted, so we can unlink localactor
	if (mMap)
		mMap->FlushEntities();

	SAFEDELETE(mLoader);
	
	PRINT("~GameManager 4");
	
	if (mPlayer)
	{
		mPlayer->mMap = NULL;
		SAFEDELETE(mPlayer);
	}
	
	PRINT("~GameManager 5");

	SavePlayerData();

	unhookNetListeners();
	SAFEDELETE(mNet);

	PRINT("~GameManager 7");

	game = NULL;
}

void GameManager::SetPosition(rect r)
{
	if (r.w > 100 && r.h > 100)
		Frame::SetPosition(r);
}

void GameManager::ResizeChildren()
{
	if (mMap)
		mMap->SetPosition( rect(0, 0, Width(), Height()) );

	Frame::ResizeChildren();
}

void callback_gameHudSubButton(Button* b)
{
	switch (b->mId.at(0))
	{
		case 'a': //avatar favorites
			if (!gui->Get("avyfavs"))
				new AvatarFavorites();
			break;
		case 'o': 
			if (!gui->Get("optionsdialog"))
			{
				OptionsDialog* o = new OptionsDialog();
				o->DemandFocus(true);
			}
			break;
		/*case 'i': //inventory
			ASSERT(inventory);
			inventory->SetVisible(true);
			inventory->MoveToTop();
			break;
		case 'c': //achievements
			if (!gui->Get("achievements"))
				new MyAchievements();
			break;*/
		case 'u': //userlist
			if (!gui->Get("userlist"))
				new UserList();
			break;
		/*case 'p': //party
			game->mParty->SetVisible(true);
			game->mParty->SetMenuMode(ActorStats::PARTY_VIEW_MENU);
			game->mParty->MoveToTop();
			break;*/
		default: break;
	}
}

void GameManager::_buildHud()
{
	string file = "assets/hud_controls.png";
	uShort x = 0, sx = 0;
	Button* b;

	mHud = new Frame(this, "", rect(12,12,0,0));

	b = new Button(mHud, "o", rect(x,0,35,35), "", callback_gameHudSubButton);
		b->mHoverText = "Options";
		b->SetImage("assets/hud/options.png");
	x += 40;
	sx += 35;

	b = new Button(mHud, "u", rect(x,0,35,35), "", callback_gameHudSubButton);
		b->mHoverText = "Userlist";
		b->SetImage("assets/hud/userlist.png");
	x += 40;
	sx += 35;

	/*b = new Button(mHud, "c", rect(x,0,35,35), "", callback_gameHudSubButton);
		b->mHoverText = "My Achievements";
		b->SetImage("assets/hud/achievements.png");
	x += 40;
	sx += 35;*/

	b = new Button(mHud, "a", rect(x,0,35,35), "", callback_gameHudSubButton);
		b->mHoverText = "My Avatars";
		b->SetImage("assets/hud/avatars.png");
	x += 40;
	sx += 35;
/*	
	b = new Button(mHud, "i", rect(x,0,35,35), "", callback_gameHudSubButton);
		b->mHoverText = "My Backpack";
		b->SetImage("assets/hud/inventory.png");
	x += 40;
	sx += 35;
	
	b = new Button(mHud, "p", rect(x,0,35,35), "", callback_gameHudSubButton);
		b->mHoverText = "My Party";
		b->SetImage("assets/hud/party.png");
	x += 40;
	sx += 35;
*/
	
	
	mHud->SetSize(x, 35);
}

void GameManager::LoadTestWorld(string luafile)
{
	SAFEDELETE(mLoader);
	mLoader = new WorldLoader();
	mLoader->LoadTestWorld(luafile);
}

void GameManager::LoadOnlineWorld(string id, point2d target, string targetObjectName)
{
	SAFEDELETE(mLoader);
	mLoader = new WorldLoader();
	mLoader->LoadOnlineWorld(id, target, targetObjectName);
}

void GameManager::Process(uLong ms)
{
	if (mLoader && mLoader->m_state == WorldLoader::WORLD_READY)
	{
		mLoader->DisplayWorld();
		ResizeChildren();
	}

	//Keep map at lowest level of our widgets
	if (!IsMapLoading() && mMap)
	{
		if (loginDialog)
			loginDialog->Die();
			
		mMap->MoveToBottom();
		mMap->Process();
	}

	MoveToBottom(); //keep game at the bottom of the screen.
}

void GameManager::UnloadMap()
{
	if (mMap)
	{
		//kill the map class itself, gracefully
		mMap->Die();
		mMap = NULL;
	}
	
	mPlayer->mMap = NULL;
	
	if (userlist)
		userlist->mOutput->Clear();
}

void GameManager::Render()
{
	rect r = GetScreenPosition();
	Image* scr = Screen::Instance();

	if (IsMapLoading() && mLoader)
	{
		//render some sort of color overlay surface over mMap?
		mLoader->Render();
	}
	
	Frame::Render();
}

void GameManager::Event(SDL_Event* event)
{
	MessageData md;
	switch (event->type)
	{
		case SDL_VIDEORESIZE:
			SetSize(gui->Width(), gui->Height());
			ResizeChildren();
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (mMap && mMap->HasMouseFocus())
			{
				md.SetId("MAP_MOUSEDOWN");
				md.WriteInt("id", event->button.button);
				messenger.Dispatch(md);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (mMap && mMap->HasMouseFocus())
			{
				md.SetId("MAP_MOUSEUP");
				md.WriteInt("id", event->button.button);
				messenger.Dispatch(md);
			}
			break;
		case SDL_MOUSEMOTION:
			if (mMap && mMap->HasMouseFocus())
			{
				md.SetId("MAP_MOUSEMOVE");
				md.WriteInt("x", event->motion.x);
				md.WriteInt("y", event->motion.y);
				messenger.Dispatch(md);	
			}
			break;
		case SDL_KEYDOWN:
			if (HasKeyFocusInTree())
			{
				md.SetId("MAP_KEYDOWN");
				md.WriteInt("id", event->key.keysym.sym);
				messenger.Dispatch(md);
				
				if (event->key.keysym.sym == SDLK_TAB && mGameMode != MODE_DUEL)
				{
					ToggleGameMode( (mGameMode == MODE_ACTION) ? MODE_CHAT : MODE_ACTION );
				}
			}
			break;
		case SDL_KEYUP:
			if (HasKeyFocusInTree())
			{
				/*if (event->key.keysym.sym == SDLK_RSHIFT || event->key.keysym.sym == SDLK_LSHIFT)
				{
					achievement_StickyKeys();
				}*/
			
				md.SetId("MAP_KEYUP");
				md.WriteInt("id", event->key.keysym.sym);
				messenger.Dispatch(md);
			}
			break;
		default: break;	
	}
	
	Frame::Event(event);
	
	if (!gui->hasKeyFocus || gui->hasKeyFocus == mMap)
	{
		if (mGameMode != MODE_ACTION)
			mChat->mInput->SetKeyFocus();
		else if (mMap)
			mMap->SetKeyFocus(true);
	}
}

Console* GameManager::GetPrivateChat(string nick)
{
	Console* c = (Console*)Get("priv" + nick);
	if (!c)
	{
		c = new Console("priv" + nick, "Private: " + nick, "assets/gui/privmsg/",
						stripCodes(nick) + "_", true, true);
		
		//Change some defaults
		//c->mBackgroundColor = color(0, 25, 0, config.GetParamInt("console", "alpha"));
		//c->mOutput->mHighlightBackground = color(0, 120, 0);

		Add(c);
		
		//Position and focus
		c->Center();
		c->ResizeChildren(); //reloads background to proper color
		c->mInput->SetKeyFocus();
		
		//Hook our input functions
		c->HookCommand("", callback_privmsgNoCommand);
		c->HookCommand("/whois", callback_privmsgCommandWhois);
	}
	c->MoveToTop();

	return c;
}

/*	<data>
		<cash amount="100" />
		<login id="" pass="" remember="0" />
		<avatar url="" pass="" w="" h="" delay="" loopsit="" loopstand="" />
		<map trading="1" shownames="0" privmsg="1" joinparts="1" addresses="0" />
		<chat position=""/>
		<flags/>
		<achievements/>
		<inventory/>
	</data>
*/
void GameManager::GenerateDefaultPlayerData()
{
	TiXmlElement* e;
	TiXmlElement* root;
	
	root = new TiXmlElement("data");
	mPlayerData.mDoc.LinkEndChild(root);

	e = mPlayerData.AddChildElement(root, "user");
	mPlayerData.SetParamString(e, "nick", "fro_user");

	e = mPlayerData.AddChildElement(root, "cash");
	mPlayerData.SetParamInt(e, "amount", 100);

	mPlayerData.AddChildElement(root, "flags");
	//mPlayerData.AddChildElement(root, "inventory");
	//mPlayerData.AddChildElement(root, "achievements");

	// <chat position=""/>
	e = mPlayerData.AddChildElement(root, "chat");
	mPlayerData.SetParamString(e, "position", "");

	// <login id="" pass="" remember="0" />
	e = mPlayerData.AddChildElement(root, "login");
	mPlayerData.SetParamString(e, "id", "");
	mPlayerData.SetParamString(e, "pass", "");
	mPlayerData.SetParamInt(e, "remember", 0);
	
	// <avatar url="" pass="" w="" h="" delay="" loopsit="" loopstand="" />
	e = mPlayerData.AddChildElement(root, "avatar");
	mPlayerData.SetParamString(e, "url", "");
	mPlayerData.SetParamString(e, "pass", "");
	mPlayerData.SetParamInt(e, "delay", 0);
	mPlayerData.SetParamInt(e, "loopsit", 0);
	mPlayerData.SetParamInt(e, "loopstand", 0);
	
	//user set game configurations
	e = mPlayerData.AddChildElement(root, "map");
	mPlayerData.SetParamInt(e, "trading", 1);
	mPlayerData.SetParamInt(e, "shownames", 0);
	mPlayerData.SetParamInt(e, "privmsg", 1);
	mPlayerData.SetParamInt(e, "joinparts", 1);
	mPlayerData.SetParamInt(e, "addresses", 0);
	mPlayerData.SetParamString(e, "lastid", "");
	mPlayerData.SetParamInt(e, "lastx", 0);
	mPlayerData.SetParamInt(e, "lasty", 0);
	
}

void GameManager::LoadPlayerData()
{
	string file = DIR_PROFILE;
	file += PLAYERDATA_FILE;
	
	if (!fileExists(file))
	{
		GenerateDefaultPlayerData();
		mPlayerData.mXmlPos = mPlayerData.mDoc.FirstChildElement("data");
		return;	
	}
	
	BoltFile bf;
	bf.mPassword = PLAYERDATA_ENCRYPTION_KEY;
	bf.mEncryptLength = 0;
	
	bf.Load(file);

	/*if (bf.Decrypt() == 0)
	{
		FATAL("Could not decrypt Player Data");
	}*/

	//Load file contents into XML elements
	if ( !mPlayerData.LoadFromMemory(bf.mData, bf.mLength) )
	{
		FATAL(mPlayerData.GetError());
	}

	mPlayerData.mAutoSave = true;
	
	mPlayerData.mXmlPos = mPlayerData.mDoc.FirstChildElement("data");

}

void GameManager::SavePlayerData()
{
	mPlayerData.mXmlPos = mPlayerData.mDoc.FirstChildElement("data");

	if (mPlayer)
		mPlayer->SaveFlagsToXml();

	string file = DIR_PROFILE;
	file += PLAYERDATA_FILE;

	if (mPlayerData.SaveToFile(file)) 
	{
		/*BoltFile bf;
		bf.mPassword = PLAYERDATA_ENCRYPTION_KEY;
		bf.mEncryptLength = 0;
		bf.Load(PLAYERDATA_FILE);
		if (bf.Encrypt() == 0)
		{
			FATAL("Could not encrypt Player Data");
		}
		else
		{
			bf.Save(PLAYERDATA_FILE);
		}
		*/
	}
	else
	{
		FATAL("Failed to save Player Data");
	}
}

void GameManager::UpdateAppTitle()
{
	string title = "fro [Build ";
	title += APP_VERSION;
	title += "]";
	
	if (mNet && mNet->IsConnected())
	{
		title += " (" + ((mNet->mRealServerAddress.empty()) ? mNet->mHost : mNet->mRealServerAddress)
				+ ":" + its(mNet->mPort);
		//if (mNet->GetState() == ONCHANNEL && mNet->GetChannel())
		//	title += " - " + mNet->GetChannel()->mId;
		
		if (mMap)
			title += " - " + mMap->mId;
		
		
		if (mNet->GetState() != ONCHANNEL)
			title += " (local)";	
			
		title += ")";
	}
	
	PRINT("Setting Title: " + title);
	
	gui->SetAppTitle(title);
}

void GameManager::ToggleGameMode(gameMode mode)
{	
	/*if (mode == MODE_DUEL && mGameMode != MODE_DUEL)
		EnableDuelMode();
	else if (mGameMode == MODE_DUEL && mode != MODE_DUEL) //if we're switching FROM duel
		DisableDuelMode();
	*/
	
	mGameMode = mode;
	
	Console* c = GetChat();
	
	if (mode == MODE_ACTION)
	{
		c->mInput->mReadOnly = true;
		c->mInput->SetText("Hit TAB to enable or disable chat mode");
	}
	else if (mChat->IsVisible()) //has to be visible for chat mode
	{
		c->mInput->mReadOnly = false;
		c->mInput->Clear();
	}
	
	// Reset our speed
	if (mPlayer->GetSpeed() == SPEED_RUN)
		mPlayer->SetSpeed(SPEED_WALK);

	// Send an event
	MessageData md("GAME_MODE");
	md.WriteInt("mode", mode);
	messenger.Dispatch(md);
}

void GameManager::ShowInfoBar(string id, string msg, int duration, string imageFile)
{
	rect r;

	//if (mInfoBar)
//		mInfoBar->Die();

//	mInfoBar 
	Frame* f = new Frame(this, id, rect(175, 0, 450, 30), "", false, false, false, true);
		f->mBoxRender = false;
		f->SetImage("assets/infobar.png");
	Label* l = new Label(f, "", rect(0, 8), msg);
		l->mFontColor = color(255, 255, 255);
		
		r = l->GetPosition();
		r.x = f->Width()/2 - r.w/2;
		if (!imageFile.empty())
			r.x += 25;
		r.y = f->Height()/2 - r.h/2 + 2;
		l->SetPosition( r );
	
	if (!imageFile.empty())
	{
		r.x -= 25;
		r.w = 20;
		r.h = 20;
		r.y = f->Height()/2 - 8;
		
		// Add a button that just acts as an image
		Button* b = new Button(f, "", r, "", NULL);
			b->mUsesImageOffsets = false;
			b->SetImage(imageFile);
	}
	
	timers->Add("", duration, false, timer_DestroyInfoBar, NULL, f);
}

void GameManager::ToggleHud(bool visible)
{
	//mChat->SetVisible(visible);
	if (mMap && mMap->mHud)
	   mMap->mHud->SetVisible(visible);
	
	//kill any dialogs that may exist
	Widget* w;
	w = gui->Get("avyfavs");
	if (w) w->Die();
	
	w = gui->Get("userlist");
	if (w) w->Die();
}

Console* GameManager::GetChat()
{
    if (mMap && mMap->mChat)
        return mMap->mChat;
       
    return NULL;   
}
