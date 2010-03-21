
#ifdef WIN32
#include <windows.h> //for Winamp support
#endif

#include "GameManager.h"
#include "Achievements.h"
#include "IrcNetListeners.h"
#include "Weapon.h"
#include "../map/CollectionMap.h"
#include "../entity/LocalActor.h"
#include "../entity/ExplodingEntity.h"
#include "../entity/Avatar.h"
#include "../core/widgets/Console.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/Scrollbar.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/OpenUrl.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/net/IrcNet2.h"
#include "../core/io/BoltFile.h"

#include "../interface/AvatarFavoritesDialog.h"
#include "../interface/LoginDialog.h"
#include "../interface/Inventory.h"
#include "../interface/Shortcuts.h"
#include "../interface/Radar.h"
#include "../interface/OptionsDialog.h"
#include "../interface/UserList.h"
#include "../interface/AvatarViewer.h"
#include "../interface/ItemTrade.h"
#include "../interface/MyAchievements.h"
#include "../interface/AvatarCreator.h"
#include "../interface/MiniMenu.h"

GameManager* game;

TimeProfiler gameProcessProfiler("Game::Process");
TimeProfiler gameRenderProfiler("Game::Render");

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
	c->AddFormattedMessage(ss);	
}

void callback_consoleToggleMapInfo(Console* c, string s)
{
	game->mMap->mShowInfo = !game->mMap->mShowInfo;
}

void callback_consoleOutputAvatar(Console* c, string s)
{
	game->mPlayer->GetAvatar()->ToFiles();
}

void callback_consoleAvatarInfo(Console* c, string s)
{
	string ss;
	if (game->mPlayer->GetAvatar())
		game->mPlayer->GetAvatar()->mImage->StateToString(ss);
	c->AddFormattedMessage(ss);
}

void callback_consoleManualServer(Console* c, string s)
{
	vString v;
	explode(&v, &s, " ");
	
	if (v.size() < 3)
	{
		c->AddMessage("Invalid. server addr:port world");
		return;
	}
	
	if (loginDialog)
		loginDialog->Die();

	game->mNet->Quit("Manual Server Input");
	game->mNet->mServerList.clear();
	game->mNet->mServerList.push_back(v.at(1));
	game->mQueuedMapId = v.at(2);
	game->mNet->TryNextServer();
}

void callback_consoleTestMap(Console* c, string s) //test_map
{
	if (s.length() < 10)
	{
		c->AddMessage("Invalid. test_map id");
		return;
	}
	
	string id = s.substr(9);
	
	game->mLoader.LoadOfflineWorld(id, point2d());
}

void callback_consolePlayerFlags(Console* c, string s)
{
	game->mPlayer->PrintFlags();
}

void callback_chatNoCommand(Console* c, string s)
{
	if (s == "/mini")
	{
		if (game->mPlayer->GetAvatar())
		{
			game->mPlayer->GetAvatar()->Modify(Avatar::MOD_MINI);
			game->mPlayer->NetSendAvatarMod();
			game->mPlayer->UpdateCollisionAndOrigin();
		}
	}
	else if (s == "/normalize")
	{
		if (game->mPlayer->GetAvatar())
		{
			game->mPlayer->GetAvatar()->Modify(Avatar::MOD_NONE);
			game->mPlayer->NetSendAvatarMod();
			game->mPlayer->UpdateCollisionAndOrigin();
		}
	}
	else if (s == "/ghost")
	{
		if (game->mPlayer->GetAvatar())
		{
			game->mPlayer->GetAvatar()->Modify(Avatar::MOD_GHOST);
			game->mPlayer->NetSendAvatarMod();
			game->mPlayer->UpdateCollisionAndOrigin();
		}
	}
#ifdef DEBUG
	else if (s == "/giant")
	{
		if (game->mPlayer->GetAvatar())
		{
			game->mPlayer->GetAvatar()->Modify(Avatar::MOD_GIANT);
			game->mPlayer->NetSendAvatarMod();
			game->mPlayer->UpdateCollisionAndOrigin();
		}
	}
#endif
	else
	{
		if (!game->mMap)
			c->AddMessage(s);
		else
			netSendSay(s);
	}
}

void callback_chatCommandMe(Console* c, string s)
{
	if (s.length() < 5)
		return;

	netSendMe(s.substr(4));
}

void callback_chatCommandStamp(Console* c, string s)
{
	if (s.length() < 8)
		return;

	netSendStamp(s.substr(7));
}

void callback_chatCommandEmote(Console* c, string s)
{
	if (s.length() < 6)
		return;

	netSendEmote(sti(s.substr(5)));
}

#ifdef WIN32
BOOL CALLBACK callback_findWinamp(HWND hwnd, LPARAM lParam) //for the below code
{
	char buffer[255];
	 
	GetWindowText(hwnd, buffer, sizeof(buffer));
    string title = buffer;
	
	int pos = title.find(" - Winamp", 0);
	if (pos != string::npos)
	{
		//found it, clean it and send it
		title.erase(pos, title.size() - pos);
		netSendMusic(title);
		return FALSE;
	}

	return TRUE; //keep searching
}
#endif //WIN32

void callback_chatCommandListeningTo(Console* c, string s)
{
	//they manually specified a song
	if (s.length() > 7)
	{
		netSendMusic(s.substr(7));
		return;
	}
	
	//otherwise, let's try to find Winamp!
#ifdef WIN32
	if (EnumWindows(callback_findWinamp, 0) == TRUE) //didn't find it
	{
		c->AddMessage("\\c900 * Can't find Winamp! Do /music <title> instead!");
	}
#else
	c->AddMessage("\\c900 * Only supported in Windows! Do /music <title> instead!");
#endif

}

void callback_chatCommandLolitrollu(Console* c, string s) { netSendEmote(1); }
void callback_chatCommandBrofist(Console* c, string s) { netSendEmote(2); }
void callback_chatCommandSpoilereyes(Console* c, string s) { netSendEmote(3); }
void callback_chatCommandBaww(Console* c, string s) { netSendEmote(4); }
void callback_chatCommandDerp(Console* c, string s) { netSendEmote(5); }
void callback_chatCommandHappy(Console* c, string s) { netSendEmote(6); }
void callback_chatCommandOmg(Console* c, string s) { netSendEmote(7); }
void callback_chatCommandFFFUUU(Console* c, string s) { netSendEmote(8); }
void callback_chatCommandHeart(Console* c, string s) { netSendEmote(9); }
void callback_chatCommandAwesome(Console* c, string s) { netSendEmote(10); }

void callback_chatCommandJoin(Console* c, string s)
{
	if (s.length() < 7)
		return;

	if (!game->mNet->IsConnected())
	{
		game->mChat->AddMessage("\\c900* Not connected!");
		return;
	}
	
	if (game->mLoader.mState != WorldLoader::WORLD_ACTIVE
		&& game->mLoader.mState != WorldLoader::FAILED)
	{
		c->AddMessage("\\c900* Already loading a world!");
		return;	
	}
	
#ifndef DEBUG
	timer* t = timers->Find("joinwait");
	if (t)
	{
		int seconds = (t->lastMs + t->interval - gui->GetTick()) / 1000;
		c->AddMessage("\\c900 * You must wait " + its(seconds) + " seconds.");
		return;
	}
#endif

	s = stripCodes(s.substr(6));
	c->AddMessage("\\c090* Loading " + s);
	game->mLoader.LoadOnlineWorld(s, point2d());

#ifndef DEBUG
	//limit the number of times they can change channels
	timers->Add("joinwait", JOIN_INTERVAL_MS, false, NULL, NULL, NULL);
#endif
}

void callback_chatCommandNick(Console* c, string s)
{	
	if (s.length() < 7)
		return;
		
	s = s.substr(6);
	
	if (game->mNet->IsConnected())
		game->mNet->ChangeNick(s);
	else
		game->mPlayer->mName = s;
		
	game->mPlayerData.SetParamString("user", "nick", s);
}

void callback_chatCommandMsg(Console* c, string s) // /msg nick message
{
	vString v;
	explode(&v, &s, " ");
	if (v.size() < 3)
	{
		c->AddMessage("Invalid. /msg nick message");
		return;
	}
	
	if (game->mNet)
		game->mNet->Privmsg(v.at(1), s.substr( s.find(v.at(2))) );
}

void callback_chatCommandListEmotes(Console* c, string s)
{
	c->AddFormattedMessage("\\c990Emotes:\\n  /troll, /brofist, /spoilereyes, /sad, /derp, /happy, /omg, /fff, /heart, /awesome");
}

void callback_chatCommandListCommands(Console* c, string s)
{
	c->AddFormattedMessage("\\c990Commands:\\n  /emotes - Display emotes list\\n  /save - Save chat log to an html file\\n"
							"  /clear - Clear the chatbox\\n  /exit - Close the program\\n  /ss - Save a screenshot (same as PRTSCRN)\\n"
							"  /stamp <text> - Stamp the text onto the map\\n  /nick <name> - Change your nickname\\n"
							"  /music <text> - Tell everyone the song you're listening to\\n  /join <world> - Try to join specified world\\n"
							"  /me <text> - Tell everyone what you're currently doing");
}

// GameManager callbacks

void callback_toggleHud(Button* b)
{
	game->mHudControls->SetVisible(!game->mHudControls->IsVisible());	
	game->ToggleHudSubMenu(""); //hide any visible sub menu
}

uShort callback_gameManagerProcess(timer* t, uLong ms)
{
	GameManager* g = (GameManager*)t->userData;
	ASSERT(g);
	g->Process(ms);	
	return TIMER_CONTINUE;
}

GameManager::GameManager(bool forceLogin)
	: Frame( gui, "GameManager", rect(0,0,gui->Width(),gui->Height()) )
{
	game = this;
	
	UpdateAppTitle();
	
	//TODO: this is temp, to ensure we don't have multiple GMs running.
	ASSERT( gui->Get("GameManager") == this );

	if (!mConfig.LoadFromFile(GAME_CONFIG_FILENAME))
	{
		FATAL(string(GAME_CONFIG_FILENAME) + " read error: " + mConfig.GetError());
	}

	mConfig.mXmlPos = mConfig.mDoc.FirstChildElement("configuration");
	
	if (!mConfig.mXmlPos)
	{
		FATAL("Invalid game_config");
	}

	mConfig.mAutoSave = true;
		
	LoadPlayerData();

	mMap = NULL;
	mShowJoinParts = mPlayerData.GetParamInt("map", "joinparts");
	mShowAddresses = mPlayerData.GetParamInt("map", "addresses");

	mChat = new Console("chat", "", "assets/console_blue.png", "chat_", false, true);
	mChat->mInput->mAllowSpecialKeys = false;
	Add(mChat);
	
	TiXmlElement* e = mConfig.GetChild(mConfig.mXmlPos, "chat");
	rect r = mConfig.GetParamRect(e, "position");
	if (isDefaultRect(r))
	{
		mChat->SetPosition( rect(mPosition.w - mChat->Width(), 
								mPosition.h - mChat->Height(),
								mChat->Width(), mChat->Height()) 
							);
	}
	else
	{
		mChat->SetPosition(r);
	}

	mLoaderImage = resman->LoadImg("assets/maploadbar.png");
	mLevelHudImage = resman->LoadImg("assets/hud_level.png");
		
	mNet = new IrcNet();
	mNet->mRealname = "fro";
	hookNetListeners();

	console->HookCommand("net_info", callback_consoleNetInfo);
	console->HookCommand("avatar_info", callback_consoleAvatarInfo);
	console->HookCommand("server", callback_consoleManualServer);
	console->HookCommand("avatarout", callback_consoleOutputAvatar);
	console->HookCommand("map_info", callback_consoleToggleMapInfo);
	console->HookCommand("test_map", callback_consoleTestMap);
	console->HookCommand("player_flags", callback_consolePlayerFlags);
	
	mPlayer = new LocalActor();
	mPlayer->mName = mPlayerData.GetParamString("user", "nick");
	if (mPlayer->mName.empty())
		mPlayer->mName = "fro_user";

	//Hook console commands
	mChat->HookCommand("", callback_chatNoCommand);
	mChat->HookCommand("/me", callback_chatCommandMe);
	mChat->HookCommand("/stamp", callback_chatCommandStamp);
	mChat->HookCommand("/music", callback_chatCommandListeningTo);
	mChat->HookCommand("/join", callback_chatCommandJoin);
	mChat->HookCommand("/nick", callback_chatCommandNick);
	mChat->HookCommand("/msg", callback_chatCommandMsg);
	mChat->HookCommand("/emo", callback_chatCommandEmote);
	mChat->HookCommand("/pos", consoleCommand::POINT2D, (void*)&mPlayer->mPosition);

	//Emotes
	mChat->HookCommand("/troll", callback_chatCommandLolitrollu);
	mChat->HookCommand("/brofist", callback_chatCommandBrofist);
	mChat->HookCommand("/spoilereyes", callback_chatCommandSpoilereyes);
	mChat->HookCommand("/sad", callback_chatCommandBaww);
	mChat->HookCommand("/derp", callback_chatCommandDerp);
	mChat->HookCommand("/happy", callback_chatCommandHappy);
	mChat->HookCommand("/omg", callback_chatCommandOmg);
	mChat->HookCommand("/fff", callback_chatCommandFFFUUU);
	mChat->HookCommand("/heart", callback_chatCommandHeart);
	mChat->HookCommand("/awesome", callback_chatCommandAwesome);
	
	mChat->HookCommand("/emotes", callback_chatCommandListEmotes);
	mChat->HookCommand("/commands", callback_chatCommandListCommands);
	

	timers->AddProcess("gameproc", 
						callback_gameManagerProcess, 
						NULL, 
						this);
	
	mMasterUrl = mConfig.GetParamString("connection", "master");

	inventory = new Inventory();
	inventory->SetVisible(false);
	
	shortcuts = new Shortcuts();
	shortcuts->SetVisible(false);

	CreateHud();

	userlist = NULL;

	ResizeChildren();
	
	//load our level
	mPlayerData.mXmlPos = mPlayerData.mDoc.FirstChildElement("data");
	mPlayerLevel = mPlayerData.GetParamInt("level", "base");

	if (forceLogin)
		new LoginDialog();
	else
		mLoader.LoadOfflineWorld("default", point2d());

}

GameManager::~GameManager()
{
	PRINT("~GameManager 1");
	
	resman->Unload(mLoaderImage);
	resman->Unload(mLevelHudImage);
	
	TiXmlElement* e = mConfig.GetChild(mConfig.mXmlPos, "chat");
	mConfig.SetParamRect(e, "position", mChat->GetPosition());
	
	PRINT("~GameManager 2");

	//erase all entities before it gets deleted, so we can unlink localactor
	if (mMap)
		mMap->FlushEntities();
	
	PRINT("~GameManager 4");
	
	if (mPlayer)
	{
		mPlayer->mMap = NULL;
		SAFEDELETE(mPlayer);
	}
	
	PRINT("~GameManager 5");
	
	if (inventory)
		inventory->Save();

	PRINT("~GameManager 6");

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

	
void callback_gameHudButton(Button* b)
{
	game->ToggleHudSubMenu(b->mId);
}

void callback_gameHudSubButton(Button* b)
{
	b->GetParent()->SetVisible(false);
	switch (b->mId.at(0))
	{
		case 'o': //options
			if (!gui->Get("optionsdialog"))
				new OptionsDialog();
			break;
		case 's': //shortcuts
			ASSERT(shortcuts);
			shortcuts->SetVisible(true);
			shortcuts->MoveToTop();
			break;
		case 'a': //avatar favorites
			if (!gui->Get("avyfavs"))
				new AvatarFavorites();
			break;
		case 'i': //inventory
			ASSERT(inventory);
			inventory->SetVisible(true);
			inventory->MoveToTop();
			break;
		case 'c': //achievements
			if (!gui->Get("achievements"))
				new MyAchievements();
			break;
		case 'r': //radar
			if (!gui->Get("radar"))
				new Radar();
			break;
		case 'u': //userlist
			if (!gui->Get("userlist"))
				new UserList();
			break;
		case 'v': //avatar viewer
			new AvatarViewer();
			break;
		case 'b': //report a bug
			new OpenUrl("http://sybolt.com/tracker/bug_report_page.php");
			break;
		case 'd': //design avatar
			new AvatarCreator();
			break;
		default: break;
	}
}

void GameManager::ToggleHudSubMenu(string id)
{
	mFrameSystem->SetVisible( (id == mFrameSystem->mId && !mFrameSystem->IsVisible()) );
	mFrameTools->SetVisible( (id == mFrameTools->mId && !mFrameTools->IsVisible()) );
	mFrameUser->SetVisible( (id == mFrameUser->mId && !mFrameUser->IsVisible()) );
}
	
void GameManager::CreateHud()
{
	string file = "assets/hud_controls.png";
	uShort x, y, sx;
	Button* b;

	b = new Button(this, "", rect(5,5,27,60), "", callback_toggleHud);
		b->mHoverText = "Toggle HUD Mode";
		makeImage(b, "", "assets/menubutton.png", rect(0,0,27,60),
					rect(0,0,27,60), WIDGETIMAGE_FULL, true, false);	

/*	MAIN HUD CONTROLS */

	mHudControls = new Frame(this, "", rect(40,12,0,0));
	
	x = 0; sx = 0;
	b = new Button(mHudControls, "System", rect(x,0,35,35), "", callback_gameHudButton);
		makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		b->mHoverText = "System";
	sx += 35;
	x += 40;
		
	b = new Button(mHudControls, "Tools", rect(x,0,35,35), "", callback_gameHudButton);
		makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		b->mHoverText = "Tools";
	sx += 35;
	x += 40;
			
	b = new Button(mHudControls, "User", rect(x,0,35,35), "", callback_gameHudButton);
		makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		b->mHoverText = "User";
	sx += 35;
	x += 40;

	mHudControls->SetSize(x, 35);
	mHudControls->SetVisible(false);

/*	SYSTEM SUB FRAME */

	rect r(40,52,35,500);
	mFrameSystem = new Frame(this, "System", r);
	mFrameSystem->SetVisible(false);
		y = 0;
		sx = 105;
		b = new Button(mFrameSystem, "o", rect(0,y,35,35), "", callback_gameHudSubButton);
			b->mHoverText = "Options";
			makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		y += 35;
		sx += 35;
		b = new Button(mFrameSystem, "b", rect(0,y,35,35), "", callback_gameHudSubButton);
			b->mHoverText = "Report A Bug";
			makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		y += 35;
		sx += 35;
		mFrameSystem->SetSize(35, y);
		
/*	TOOLS SUB FRAME */
		
	r.x += 40;
	mFrameTools = new Frame(this, "Tools", r);
	mFrameTools->SetVisible(false);
		y = 0;
		sx = 175;
		b = new Button(mFrameTools, "r", rect(0,y,35,35), "", callback_gameHudSubButton);
			b->mHoverText = "Radar";
			makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		y += 35;
		sx += 35;
		b = new Button(mFrameTools, "u", rect(0,y,35,35), "", callback_gameHudSubButton);
			b->mHoverText = "Userlist";
			makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		y += 35;
		sx += 35;
		b = new Button(mFrameTools, "s", rect(0,y,35,35), "", callback_gameHudSubButton);
			b->mHoverText = "Shortcuts";
			makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		y += 35;
		sx += 35;
		b = new Button(mFrameTools, "v", rect(0,y,35,35), "", callback_gameHudSubButton);
			b->mHoverText = "Avatar Viewer";
			makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		y += 35;
		sx += 35;
		mFrameTools->SetSize(35, y);
		
/*	USER SUB FRAME */
	
	r.x += 40;
	mFrameUser = new Frame(this, "User", r);
	mFrameUser->SetVisible(false);
		y = 0;
		sx = 315;
		b = new Button(mFrameUser, "a", rect(0,y,35,35), "", callback_gameHudSubButton);
			b->mHoverText = "Avatar Favorites";
			makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		y += 35;
		sx += 35;
		b = new Button(mFrameUser, "i", rect(0,y,35,35), "", callback_gameHudSubButton);
			b->mHoverText = "Inventory";
			makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		y += 35;
		sx += 35;
		b = new Button(mFrameUser, "c", rect(0,y,35,35), "", callback_gameHudSubButton);
			b->mHoverText = "Achievements";
			makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		y += 35;
		sx += 35;
		b = new Button(mFrameUser, "d", rect(0,y,35,35), "", callback_gameHudSubButton);
			b->mHoverText = "Design Avatar";
			makeImage(b, "", file, rect(sx,0,35,35), rect(0,0,35,35), WIDGETIMAGE_FULL, true, false);
		y += 35;
		sx += 35;
		
		mFrameUser->SetSize(35, y);

}

void GameManager::Process(uLong ms)
{
	gameProcessProfiler.Start();

	if (mLoader.mState == WorldLoader::WORLD_READY)
	{
		mLoader.ActivateWorld();
		ResizeChildren();
		//Hide loading animation thing
	}
	else if (mLoader.mState == WorldLoader::FAILED)
	{
		//if they're not on a server, and there's no map, bring login back up
		if (!mMap && (!mNet || !mNet->IsConnected()))
		{
			if (!loginDialog)
				new LoginDialog();
			mLoader.SetState(WorldLoader::IDLE);
		}
		
	}
	
	//Keep map at lowest level of our widgets
	if (!IsMapLoading() && mMap)
	{
		if (loginDialog)
			loginDialog->Die();
			
		mMap->MoveToBottom();
	}
	
	if (gui->hasKeyFocus == gui || gui->hasKeyFocus == NULL)
		gui->hasKeyFocus = mChat->mInput;
	
	MoveToBottom(); //keep game at the bottom of the screen.
	
	gameProcessProfiler.Stop();
}

void GameManager::_renderMapLoader(uLong ms)
{
	Image* scr = Screen::Instance();

//	mFont->Render( scr, 5, 5, dts(mLoader.Progress()) + "% " + its(mLoader.mTotalResources) 
//					+ ":" + its(mLoader.mCompletedResources), color(255) );
	
	rect r = GetScreenPosition();
	
	//render progress bar, etc
	string msg;
	switch (mLoader.mState)
	{
		case WorldLoader::IDLE:
			msg = "Idle";
			break;
		case WorldLoader::FAILED:
			msg = "\\c900Failed";
			break;
		case WorldLoader::GETTING_CONFIG:
			msg = "Getting Config";
			break;
		case WorldLoader::GETTING_RESOURCES:
			msg = "Getting Resource " + its(mLoader.mTotalResources) + " / " + its(mLoader.mCompletedResources);
			break;
		case WorldLoader::BUILDING_WORLD:
			msg = "Building World";
			break;
		case WorldLoader::JOINING_WORLD:
			msg = "Joining World";
			break;
		case WorldLoader::WORLD_READY:
			msg = "World Ready";
			break;
		case WorldLoader::WORLD_ACTIVE:
			msg = "World Active";
			break;
		default: break;
	}
	
	//top bar
	mLoaderImage->RenderPattern(scr, rect(10,0,50,22), rect(0,0,Width(),25));
	
	mFont->Render( scr, r.x + 5, 4, "You are on your way to: " + mQueuedMapId, color(50,50,50) );
	
	//progress background
	mLoaderImage->RenderBox(scr, rect(0,22,10,10), rect(0,22,Width(),30));
	
	uShort w = (uShort)((double)Width() * mLoader.Progress());
	
	//progress foreground
	if (w > 0)
		mLoaderImage->RenderBox(scr, rect(30,22,10,10), rect(0,22,w,30));
	
	//bottom bar
	mLoaderImage->RenderPattern(scr, rect(0,52,25,25), rect(r.x,r.y+52,275,25));
	
	//bottom bar slanted edge
	mLoaderImage->Render(scr, r.x+275, r.y+52, rect(35,52,25,25));
	
	mFont->Render( scr, r.x + 5, r.y + 57, msg, color(50,50,50) );

}

void GameManager::UnloadWorld()
{
	//TODO: THIS!
	if (mMap)
	{
		mMap->Die();
		mMap = NULL;
	}
	
	mPlayer->mMap = NULL;
}

void GameManager::Render(uLong ms)
{
	gameRenderProfiler.Start();
	
	rect r = GetScreenPosition();
	Image* scr = Screen::Instance();
	
	Frame::Render(ms);
	
	if (IsMapLoading())
	{
		//render some sort of color overlay surface over mMap?
		_renderMapLoader(ms);
	}
	else
	{
		if (!mHudControls->IsVisible() && mLevelHudImage)
		{
			string s = its(mPlayerLevel);
			
			//render Level: text
			mLevelHudImage->Render( scr, r.x + 40, r.y + 16, rect(0, 0, 78, 26) );
	
			//render each number
			sShort yOffset = 1;
			sShort xOffset = 40 + 78;
			
			for (uShort i = 0; i < s.size(); i++)
			{
				mLevelHudImage->Render( scr, xOffset, r.y + 16 + yOffset, rect(78 + (s.at(i) - '0') * 21, 0, 21, 26) );
	
				xOffset += 18;
				yOffset *= -1; //invert offset	
			}
		}		
	}
	
	gameRenderProfiler.Stop();
}

void GameManager::DoActionKeyEvent()
{
	DEBUGOUT("DoActionKeyEvent 1");
	if (IsMapLoading() || mPlayer->mIsLocked || !mChat->mInput->GetText().empty()) //<- will always be empty, input gets event b4 us!
		return;
		
	DEBUGOUT("DoActionKeyEvent 2");

	//Make sure player input is directed toward the map		
	if (!mChat->HasKeyFocusInTree() && !mMap->HasKeyFocus())
		return;
		
	DEBUGOUT("DoActionKeyEvent 3");

	MessageData md("PLAYER_ACTION");
	messenger.Dispatch(md, this);
/*	
	if (!mPlayer->mWeapon)
	{
		mPlayer->mWeapon = new Sword();
		mPlayer->mWeapon->mOwner = mPlayer;
	}
	mPlayer->mWeapon->Use();
*/
	DEBUGOUT("DoActionKeyEvent end");
}

void GameManager::Event(SDL_Event* event)
{
	switch (event->type)
	{
		case SDL_VIDEORESIZE:
			SetSize(gui->Width(), gui->Height());
			ResizeChildren();
			break;
		case SDL_KEYUP:
			if (event->key.keysym.sym == SDLK_RSHIFT || event->key.keysym.sym == SDLK_LSHIFT)
			{
				achievement_StickyKeys();
			}
			break;
		case SDL_KEYDOWN:
			if (event->key.keysym.mod & KMOD_ALT) //handle alt+? shortcuts
			{
				/*switch (event->key.keysym.sym) 
				{
					case SDLK_o: //options
						if (!gui->Get("optionsdialog"))
							new OptionsDialog();
						break;
					case SDLK_s: //shortcuts
						ASSERT(shortcuts);
						shortcuts->SetVisible(true);
						shortcuts->MoveToTop();
						break;
					case SDLK_a: //avatar favorites
						if (!gui->Get("avyfavs"))
							new AvatarFavorites();
						break;
					case SDLK_i: //inventory
						ASSERT(inventory);
						inventory->SetVisible(true);
						inventory->MoveToTop();
						break;
					case SDLK_r: //radar
						if (!gui->Get("radar"))
							new Radar();
						break;
					case SDLK_u: //userlist
						if (!gui->Get("userlist"))
							new UserList();
						break;
					case SDLK_v: //avatar viewer
						new AvatarViewer();
						break;
					default: break;	
				}*/
			}
			else
			{
				//F1 through F8 assigned to shortcuts 
				if (event->key.keysym.sym >= SDLK_F1 && event->key.keysym.sym <= SDLK_F8)
				{
					if (shortcuts)
						shortcuts->Run(event->key.keysym.sym - SDLK_F1);
				}
				else if (event->key.keysym.sym == SDLK_ESCAPE)
				{
					if (!gui->Get("MiniMenu"))
						new MiniMenu();
				}
			/*	else if (event->key.keysym.sym == SDLK_HOME)
				{
					if (!mChat->mInput->mReadOnly)
					{
						mChat->mInput->mReadOnly = true;
						mChat->mInput->SetText("Hit HOME to toggle mode");
					}
					else
					{
						mChat->mInput->mReadOnly = false;
						mChat->mInput->Clear();
					}
				}*/
			/*	else if (event->key.keysym.sym == SDLK_RETURN)
				{
					DoActionKeyEvent();
				}*/
			}
			break;
		default: break;	
	}
	
	Frame::Event(event);
}

Console* GameManager::GetPrivateChat(string nick)
{
	Console* c = (Console*)Get("priv" + nick);
	if (!c)
	{
		c = new Console("priv" + nick, "Private: " + nick, "assets/console_green.png",
						stripCodes(nick) + "_", true, true);
		
		//Change some defaults
		c->mBackgroundColor = color(0, 25, 0, config.GetParamInt("console", "alpha"));
		c->mOutput->mHighlightBackground = color(0, 120, 0);

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
		<class name="Storybook Character" />
		<level base="1" />
		<flags/>
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

	e = mPlayerData.AddChildElement(root, "class");
	mPlayerData.SetParamString(e, "name", "Storybook Character");
	
	e = mPlayerData.AddChildElement(root, "level");
	mPlayerData.SetParamInt(e, "base", 1);
	
	mPlayerData.AddChildElement(root, "flags");
	mPlayerData.AddChildElement(root, "inventory");
	
	//user set game configurations
	e = mPlayerData.AddChildElement(root, "map");
	mPlayerData.SetParamInt(e, "trading", 1);
	mPlayerData.SetParamInt(e, "shownames", 0);
	mPlayerData.SetParamInt(e, "privmsg", 1);
	mPlayerData.SetParamInt(e, "joinparts", 1);
	mPlayerData.SetParamInt(e, "addresses", 0);
}

void GameManager::LoadPlayerData()
{
	if (!fileExists(PLAYERDATA_FILE))
	{
		GenerateDefaultPlayerData();
		mPlayerData.mXmlPos = mPlayerData.mDoc.FirstChildElement("data");
		return;	
	}
	
	BoltFile bf;
	bf.mPassword = PLAYERDATA_ENCRYPTION_KEY;
	bf.mEncryptLength = 0;
	
	bf.Load(PLAYERDATA_FILE);

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
	mPlayerData.SetParamInt("level", "base", mPlayerLevel);

	if (mPlayer)
		mPlayer->SaveFlags();

	if (mPlayerData.SaveToFile(PLAYERDATA_FILE)) 
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

void GameManager::DisplayAchievement(string title)
{
	//TODO: Popup and AWESOME STUFFS!
	
	game->mChat->AddMessage("\\c139 * " + mPlayer->mName + "\\c999 achieved: \\c080 " + title);
	mPlayer->Emote(11);
	
	//Tell everyone!
	netSendAchievement(title);
	
	achievement_OverAchiever();
}

void GameManager::_addNewAchievement(string title, string desc, int max, string file)
{
	TiXmlElement* top = mPlayerData.mDoc.FirstChildElement();
	TiXmlElement* e;
	
	ASSERT(top);
	
	e = top->FirstChildElement("achievements");
	if (!e) //make a new one
	{
		e = mPlayerData.AddChildElement(top, "achievements");
	}
	
	e = mPlayerData.AddChildElement(e, "ach");
	
	mPlayerData.SetParamString(e, "title", title);
	mPlayerData.SetParamString(e, "desc", desc);
	mPlayerData.SetParamString(e, "file", file);
	mPlayerData.SetParamInt(e, "max", max);
	mPlayerData.SetParamInt(e, "total", 1);

	SavePlayerData();

	//We earned the achievement if we don't have more than 1 total. Display it
	if (max == 1)
		DisplayAchievement(title);
}
	
int GameManager::EarnAchievement(string title, string desc, int max, string file)
{
	if (title.empty() || max < 1)
		return 0;
		
	//if we already have it, update it
		
	//Get master element
	TiXmlElement* top = mPlayerData.mDoc.FirstChildElement();
	TiXmlElement* e;
	
	ASSERT(top);

	e = top->FirstChildElement("achievements");
	if (e)
	{
		e = e->FirstChildElement();
		while (e)
		{
			if (mPlayerData.GetParamString(e, "title") == title)
			{
				int total = mPlayerData.GetParamInt(e, "total");
				max = mPlayerData.GetParamInt(e, "max"); //ignore the new max, can't change it
				
				if (total + 1 < max) //just get closer to that max
				{
					mPlayerData.SetParamInt(e, "total", total+1);
					SavePlayerData();
					return total+1;
				}
				else if (total + 1 == max)
				{
					mPlayerData.SetParamInt(e, "total", max);
					SavePlayerData();
					DisplayAchievement(title);
					return max;
				}
				else
				{
					return max;
				}
			}
			e = e->NextSiblingElement();
		}
	}
	
	//not found, add it.
	_addNewAchievement(title, desc, max, file);
	
	return 1;
}
	
void GameManager::UpdateAppTitle()
{
	string title = "fro [Build ";
	title += APP_VERSION;
	title += " BETA]";
	
	if (mNet && mNet->IsConnected())
	{
		title += " (" + mNet->mHost + ":" + its(mNet->mPort);
		if (mNet->GetState() == ONCHANNEL && mNet->GetChannel())
			title += " - " + mNet->GetChannel()->mId;
			
		title += ")";
	}
	
	gui->SetAppTitle(title);
}

