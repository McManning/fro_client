
#include <SDL/SDL.h>
#include "GuiManager.h"
#include "TimeProfiler.h"
#include "widgets/Console.h"
#include "widgets/Input.h"
#include "Image.h"
#include "TimerManager.h"
#include "ResourceManager.h"
#include "FontManager.h"
#include "sound/SoundManager.h"
#include "Screen.h"
#include "net/DownloadManager.h"
#include "MessageManager.h"
#include "io/FileIO.h"
#include "io/XmlFile.h"

GuiManager* gui;

TimeProfiler guiProcessProfiler("Gui::Process");
TimeProfiler guiRenderProfiler("Gui::Render");

uShort timer_GuiThinkInSeconds(timer* t, uLong ms)
{
	gui->ThinkInSeconds(ms);
	return TIMER_CONTINUE;
}

uShort timer_killGuiAlert(timer* t, uLong ms)
{
	gui->mAlert.clear();
	return TIMER_DESTROY;
}

void callback_ToggleMessengerDebug(Console* c, string s)
{
	messenger.mDebugMode = !messenger.mDebugMode;	
}

GuiManager::GuiManager()
{
	gui = this;
	mConstrainChildrenToRect = false;
	mCustomCursorSourceY = 0;
	
	PRINT("--------------------------------------------------");
	PRINT("-Begin Gui Initialization-------------------------\n  *\n *\n*\n");
	
	Image* scr = Screen::Instance(); //Start up SDL before anything else
	
//SDL Settings
	SDL_EnableUNICODE(SDL_ENABLE); //TODO: unicode has an overhead, however it handles shift/capslock so keep till I write in a fix.. or actually USE unicode
	SDL_ShowCursor(SDL_DISABLE); //We have our own cursor

	seedRnd();
	loadGlobalConfig();

//ResourceManager
	PRINT("Loading ResMan");
	resman = new ResourceManager();
	
//TimerManager
	PRINT("Loading TimerMan");
	timers = new TimerManager();
	timers->Add("GuiSec", 1000, true, timer_GuiThinkInSeconds);
	
//FontManager
	PRINT("Loading FontMan");
	fonts = new FontManager();
	if (!fonts->Initialize())
		FATAL("FontMan Init Fail");

//SoundManager
	PRINT("Loading SoundMan");
	sound = new SoundManager();
	if (!sound->Initialize())
		WARNING("Failed to Initialize Sound Manager");
		
	sound->SetVolume( config.GetParamInt("sound", "volume") );

	sound->Load("blip", "assets/sfx/chat.wav");

//DownloadManager
	PRINT("Loading DlMan");
	downloader = new DownloadManager();
	if (!downloader->Initialize(5))
		FATAL("DlMan Init Fail");

//Configurations/Settings

	hasMouseFocus 
		= hasKeyFocus 
		= previousMouseFocus = NULL;
	mNoFpsLimit = config.GetParamInt("system", "nolimit");
	mUseLowCpu = config.GetParamInt("system", "lowcpu");
	mFpsCap = config.GetParamInt("system", "fps");
	mGetUserAttention = false;
	mTitleFlashOn = false;
	mShowStats = true;
	mTick = 0;
	mNextRenderTick = 0;
	mBeatCounter = 0;
	mFrameCounter = 0;
	mCursorImage = resman->LoadImg("assets/gui/gui.png");
	mPosition = scr->GetClip();
	mFont = fonts->Get();
	
	mDarkOverlay = resman->NewImage(scr->Width(), scr->Height(), color(255,255,255), true);
	mDarkOverlay->DrawRect(mDarkOverlay->GetClip(), color(0,0,0,200));

//Consoles
	PRINT("Loading Console");
	console = new Console("console", "Build " + string(APP_VERSION), "assets/gui/console/", "log_", false, true);
	
	PRINT("Configuring Console");
	
	TiXmlElement* e = config.GetChild(config.mXmlPos, "console");
	rect r = config.GetParamRect(e, "position");
	if (isDefaultRect(r))
	{
		console->SetSize(SCREEN_WIDTH - 160, SCREEN_HEIGHT - 120);
		console->Center();
	}
	else
	{
		console->SetPosition(r);
	}
	
	console->HookCommand("messenger_debug", callback_ToggleMessengerDebug);

	console->mBackgroundColor = color(85,90,100);
	console->ResizeChildren();
	
	Add(console);
	console->SetVisible(false);
	
	PRINT("GuiManager Finished");
}

GuiManager::~GuiManager()
{
	PRINT("--------------------------------------------------");
	PRINT("-Begin Gui Shutdown-------------------------------\n  *\n *\n*\n");

	mGlobalEventHandlers.clear();
	_cleanDeletionStack();
	
	TiXmlElement* e = config.GetChild(config.mXmlPos, "console");
	config.SetParamRect(e, "position", console->GetPosition());
	
	PRINT("Burning Widget Tree");
	
	//Delete all children widgets before deleting any managers they may require
	for (uShort i = 0;  i < mChildren.size(); i++)
	{
		mChildren.at(i)->mParent = NULL;
		SAFEDELETE(mChildren.at(i));
	}
	mChildren.clear();

	resman->Unload(mCursorImage);

	PRINT("Flushing Timers");

	/*	Erase all timers before we start deleting classes that may
		need to be accessed during timer deletion
	*/
	timers->FlushTimers();
	
	PRINT("Deleting DlMan");
	SAFEDELETE(downloader);
	
	PRINT("Deleting Sound");
	SAFEDELETE(sound);
	
	PRINT("Deleting FontMan");
	SAFEDELETE(fonts);
	
	PRINT("Deleting TimerMan");
	SAFEDELETE(timers);
	
	PRINT("Deleting ResMan");
	SAFEDELETE(resman);

	PRINT("Destroying Screen");
	Screen::Destroy();
	
	PRINT("Closing SDL");
	SDL_Quit(); //must be the last call after all SDL-related calls
	
	PRINT("Gui Shutdown Complete");
}

//Perform recursive search for children that have the coords in their rect
Widget* GuiManager::GrabWidgetUnderXY(Widget* root, sShort x, sShort y) const
{
	if (!root || !root->IsVisible())
		return NULL;

	Widget* grabbed = NULL;
	Widget* result = (isPointInRect(root->GetScreenPosition(), x, y)) ? root : NULL;

	//if we're currently not in root, and it doesn't allow outside children, then return NULL
	if (!result && root->mConstrainChildrenToRect)
		return NULL;

	//Go through each child from bottom to top, checking if they have the point
	for (uShort i = 0; i < root->mChildren.size(); i++)
	{
		//recurse children to find out which one has the point instead of the parent
		grabbed = GrabWidgetUnderXY(root->mChildren.at(i), x, y);
		if (grabbed)
			result = grabbed;
	}

	return result;
}

void GuiManager::_distributeEvent(SDL_Event* event)
{
//	PRINT("Gui::Event");

	switch (event->type)
	{
		case SDL_KEYDOWN:
		{
//			PRINT("Gui::EventKD");
			if (hasKeyFocus)
				hasKeyFocus->Event(event);
			_sendToGlobalEventHandlers(event, hasKeyFocus);
		} break;
		case SDL_KEYUP:
		{
//			PRINT("Gui::EventKU");
			
			if (event->key.keysym.sym == SDLK_PRINT)
			{
				Screenshot();
			}
			else if (event->key.keysym.sym == SDLK_HOME)
			{
				console->SetVisible(!console->IsVisible());
			}

			if (hasKeyFocus)
				hasKeyFocus->Event(event);
			_sendToGlobalEventHandlers(event, hasKeyFocus);
		} break;
		case SDL_MOUSEBUTTONDOWN:
		{
//			PRINT("Gui::EventMD");
			if (hasMouseFocus)
			{
				if (hasMouseFocus->IsActive())
				{
					hasKeyFocus = hasMouseFocus; //Clicking a widget preps it for keyboard input in most cases

					//if we have a focused and they try to access something outside our dialog, stop it
					if (GetDemandsFocus() && !GetDemandsFocus()->HasKeyFocusInTree())
					{
						if (GetDemandsFocus()->mTemporary)
						{
							GetDemandsFocus()->Die();
							hasKeyFocus->MoveToTop();
							hasKeyFocus->Event(event);
							_sendToGlobalEventHandlers(event, hasKeyFocus);
						}
						else
						{
							hasKeyFocus = NULL;
						}
//						PRINT("Gui Demands Focus: " + pts(GetDemandsFocus()));
					}
					else
					{
//						PRINT("Gui KeyFocus: " + pts(hasKeyFocus));

						hasKeyFocus->MoveToTop();
						hasKeyFocus->Event(event);
						_sendToGlobalEventHandlers(event, hasKeyFocus);
					}
				}
				//If the gui system has key focus, null it out. (guiManager doesn't need it)
				if (hasKeyFocus == this)
					hasKeyFocus = NULL;
				
			} //TODO: Should globals handle this as an else case?
		} break;
		case SDL_MOUSEBUTTONUP:
		{
//			PRINT("Gui::EventMU " + pts(hasKeyFocus));
			if (hasKeyFocus)
				hasKeyFocus->Event(event);
			_sendToGlobalEventHandlers(event, hasKeyFocus);
		} break;
		case SDL_MOUSEMOTION:
		{
//			PRINT("Gui::EventMM");
			mLastMousePosition = mMousePosition;
			mMousePosition.x = event->motion.x;
			mMousePosition.y = event->motion.y;
		
			previousMouseFocus = hasMouseFocus;
			hasMouseFocus = GrabWidgetUnderXY(this, GetMouseX(), GetMouseY());

			if (hasMouseFocus) hasMouseFocus->Event(event);
			_sendToGlobalEventHandlers(event, hasMouseFocus);

		} break;
		default: //don't know what it is, just pass it through globals.
		{
			_sendToGlobalEventHandlers(event, NULL);
		} break;
	}
//	PRINT("Gui::EventEnd");
}

void GuiManager::_renderStats(Image* scr)
{
	if (!mShowStats)
		return;

	string s;
	
	s += " [DL " + its(downloader->mQueued.size()); // Count of queued files to download
	s += ">" + its(downloader->CountActiveDownloads()); // Count of active downloads
	s += ">" + its(downloader->mCompleted.size()) + "] "; // Count of completed files
	
	s += " [FPS:" + its(mFps); //frames rendered per second
	s += " BPS:" + its(mBps); //heartbeats a second
	s += " rMS:" + its(mRenderTime) + "]"; //time it took to render last frame
	
	rect r;
	r.w = mFont->GetWidth(s);
	r.h = mFont->GetHeight();
	r.x = 3;
	r.y = scr->Height() - r.h - 3;

	//scr->DrawRect(r, color());
	mFont->Render(scr, r.x, r.y, s, color(128, 128, 0));
}

void GuiManager::_renderCursor(Image* scr)
{
	//If the cursor is over an input widget, render a caret version
	if (hasMouseFocus && hasMouseFocus->mType == WIDGET_INPUT)
	{
		mCursorImage->Render(scr, GetMouseX()-4, GetMouseY() - 10, rect(17, 0, 7, 22));
	}
	else //render normal cursor
	{
		mCursorImage->Render(scr, GetMouseX(), GetMouseY(), rect(1, mCustomCursorSourceY, 16, 22));
	}
}

void GuiManager::_renderCursorTipText(Image* scr, string& text)
{
	rect pos;
	
	//TODO: WRAP!
	
	int w, h;
	mFont->GetTextSize(text, &w, &h);
	pos.h = h + 2;
	pos.w += w + 8;
	pos.x = GetMouseX();
	pos.y = GetMouseY() - pos.h;

	//constrain to screen
	if (pos.x + pos.w > scr->Width())
		pos.x = scr->Width() - pos.w;
	if (pos.y + pos.h > scr->Height())
		pos.y = scr->Height() - pos.h;
	if (pos.y < 0)
		pos.y = 0;
	if (pos.x < 0)
		pos.x = 0;

	if (mCursorImage)
		mCursorImage->RenderBox(scr, rect(24, 0, 5, 5), pos);

	mFont->Render(scr, pos.x+4, pos.y+1, hasMouseFocus->mHoverText, color(0, 0, 0));
}

void GuiManager::Render()
{
//	PRINT("GuiMan::Render");
	uLong ms;
	string text;
	
	rect r;
	uLong renderStart = SDL_GetTicks(); //keep track of how long this render takes
	Screen* scr = Screen::Instance();

	//TODO: Do pre-rendering crap
	scr->DrawRect(scr->GetClip(), color(237, 236, 235));

	//Render widget tree
	Widget::Render();

	if (!mAlert.empty())
		mFont->Render(scr, 5, 5, mAlert, color(255,0,0));
		
	_renderStats(scr);
	
	if (SDL_GetAppState() & SDL_APPMOUSEFOCUS)
	{
		_renderCursor(scr);
		
		//if we have text to be drawn on hover, do it.
		if (mFont && hasMouseFocus)
		{
			text = stripCodes(hasMouseFocus->mHoverText);
			if (!text.empty())
			{
				_renderCursorTipText(scr, text);
			}
		}	
	}

	//Finally, display the changes to the user
	scr->Flip();

	ms = SDL_GetTicks();

	//Track how long that frame took to draw.
	mRenderTime = ms - renderStart;

	_getNextRenderTick(ms);
	
	mFrameCounter++;

//	PRINT("Done");
}

void GuiManager::RenderDarkOverlay()
{
	Screen* scr = Screen::Instance();
	
	if (mDarkOverlay)
		mDarkOverlay->Render(scr, 0, 0);
}

void GuiManager::_getNextRenderTick(uLong ms)
{
	if (mNoFpsLimit)
		return;

	//Slow rendering speed if this window doesn't have focus (If that feature is enabled). Also make sure fps cap isn't set below 5.
	int fps = (((SDL_GetAppState() & SDL_APPINPUTFOCUS) || !mUseLowCpu) && mFpsCap > 5) ? mFpsCap : 5;
	if (fps < 1)
		fps = 1;

	//TODO: Bug here, idk wtf is going on. Sometimes goes negative at startup.
	//I threw in a signed long and checked for negative before sending to the uLong.
	signed long sl = ms + (1000 / fps) - mRenderTime; //Subtract render time to create a more constant FPS
	if (sl < 0)
		sl = 0;
		
	mNextRenderTick = sl;
}

bool GuiManager::IsKeyDown(int key) const
{
	Uint8 *keystate = SDL_GetKeyState(NULL); //grab SDLs internal keyboard snapshot array
	return keystate[key];
}

bool GuiManager::IsMouseButtonDown(byte button, int* x, int* y) const
{
	return SDL_GetMouseState(x, y) & SDL_BUTTON(button);
}

void GuiManager::SetMouseXY(uShort x, uShort y)
{
	mLastMousePosition = mMousePosition;
	
	SDL_WarpMouse(x, y);

	mMousePosition.x = x;
	mMousePosition.y = y;

	//OPTIMIZETODO: flag rendering crap here
}

void GuiManager::AddGlobalEventHandler(Widget* w) 
{ 
	mGlobalEventHandlers.push_back(w); 
}

void GuiManager::RemoveGlobalEventHandler(Widget* w)
{
//	PRINT("Gui::RemoveGlobalEventHandler: " + w->mId);
	
	//Null them instead of deleting, in case we're iterating the list elsewhere
	for (int i = 0; i < mGlobalEventHandlers.size(); ++i)
	{
		if (mGlobalEventHandlers.at(i) == w)
			mGlobalEventHandlers.at(i) = NULL;
	}
}

void GuiManager::_sendToGlobalEventHandlers(SDL_Event* event, Widget* excluding)
{
//	PRINT("Gui::_sendToGlobalEventHandlers");

	for (int i = 0; i < mGlobalEventHandlers.size(); ++i)
	{
		if (!mGlobalEventHandlers.at(i))
		{
			mGlobalEventHandlers.erase(mGlobalEventHandlers.begin() + i);
			i--;
		}
		else if (mGlobalEventHandlers.at(i) != excluding)
		{
			if (!GetDemandsFocus() || GetDemandsFocus()->HasKeyFocusInTree()) //TODO: What about mouse movement? Should they really still get it?
			{
				mGlobalEventHandlers.at(i)->Event(event);
			}
		}
	}
}

void GuiManager::RemoveWidget(Widget* w)
{
	if (w)
	{
		for (int i = 0; i < mDeletionStack.size(); ++i)
		{
			if (mDeletionStack.at(i) == w)
				return;
		}
		mDeletionStack.push_back(w);
	}
}

void GuiManager::_cleanDeletionStack()
{
	for (int i = 0; i < mDeletionStack.size(); ++i)
	{
		SAFEDELETE(mDeletionStack.at(i));
	}
	mDeletionStack.clear();
}

void GuiManager::DereferenceWidget(Widget* w)
{
	if (hasKeyFocus == w) hasKeyFocus = NULL;
	if (hasMouseFocus == w) hasMouseFocus = NULL;
	if (previousMouseFocus == w) previousMouseFocus = NULL;

	RemoveGlobalEventHandler(w);
	RemoveFromDemandFocusStack(w);
	
	if (timers)
		timers->RemoveMatchingUserData(w);
}

Widget* GuiManager::GetDemandsFocus() const
{
	if (mDemandFocusStack.empty())
		return NULL;
	
	return mDemandFocusStack.at(mDemandFocusStack.size() - 1);
}

void GuiManager::RemoveFromDemandFocusStack(Widget* w)
{
	for (int i = 0; i < mDemandFocusStack.size(); ++i)
	{
		if (mDemandFocusStack.at(i) == w)
		{
			mDemandFocusStack.erase(mDemandFocusStack.begin() + i);
			i--;
		}
	}
}

void GuiManager::GetUserAttention()
{
	int i = config.GetParamInt("system", "alerts");
	if ( i == 1 || (i == 2 && !(SDL_GetAppState() & SDL_APPINPUTFOCUS)) )
	{
		if (sound)
			sound->Play("blip");
		mGetUserAttention = true;
	}
}

void GuiManager::ThinkInSeconds(uLong ms)
{
	if (isAppClosing())
		return;

	mTitleFlashOn = !mTitleFlashOn;

	//if we're flashing and focused, do flash
	if (mTitleFlashOn && mGetUserAttention)
		SDL_WM_SetCaption("--------------------------------", NULL);
	else
	{
		string title = mAppTitle; // + " [FPS: " + its(mFps) + " BPS: " + its(mBps) + "]";
		SDL_WM_SetCaption(title.c_str(), NULL);
	}
	
	
	//Also recalculate FPS since this is ran every minute anyway
	mFps = mFrameCounter;
	mBps = mBeatCounter;
	mFrameCounter = 0;
	mBeatCounter = 0;
}

void GuiManager::SetAppTitle(string caption)
{
	mAppTitle = caption;
	SDL_WM_SetCaption(mAppTitle.c_str(), NULL);
}

void GuiManager::Process()
{
//	PRINT("Pump");
	//If we're focused, don't worry about getting their attention
	if (SDL_GetAppState() & SDL_APPINPUTFOCUS)
		mGetUserAttention = false;

	//Poll and distribute events from SDL
	SDL_Event event;
	rect r;
#ifdef OPTIMIZED
	bool gotEvent = false;
#endif
	while (SDL_PollEvent(&event))
	{
		
#ifdef OPTIMIZED
		gotEvent = true;
#endif
		switch (event.type)
		{
			case SDL_QUIT:
				appState = APPSTATE_CLOSING;
				SDL_WM_SetCaption("Shutting down, please wait..", NULL);
				break;
			case SDL_VIDEORESIZE:
				r.w = event.resize.w;
				r.h = event.resize.h;
				Screen::Instance()->Resize(event.resize.w, event.resize.h);
				
				SetPosition(r);
				_distributeEvent(&event); //let our global handlers know we resized
				break;
			default:
				_distributeEvent(&event);
				break;
		}
		
	}
	
#ifdef OPTIMIZED
	if (gotEvent)
		Screen::Instance()->Update(); //events almost always mean something changed.
#endif
	
//	PRINT("MSGR:PROC");
	messenger.Process(mTick);
	
//	PRINT("TIMER:PROC");
	timers->Process(mTick);

//	PRINT("STACK:CLEAN");
	_cleanDeletionStack();
	
	//make sure console stays on top
	console->MoveToTop();
	
	//make sure focused window is always on top the other
	if (GetDemandsFocus())
		GetDemandsFocus()->MoveToTop();
	
//	PRINT("PumpDone");
}

void GuiManager::MainLoop()
{
	PRINT("--------------------------------------------------");
	PRINT("-Begin Main Loop----------------------------------\n  *\n *\n*\n");

	while (!isAppClosing())
	{
		mTick = SDL_GetTicks();

		guiProcessProfiler.Start();
		Process();
		guiProcessProfiler.Stop();

		//Only render if: We've waited enough ticks or there's no wait time, AND the app isn't minimized
		if ( (mTick >= mNextRenderTick || mNoFpsLimit)
				&& (SDL_GetAppState() & SDL_APPACTIVE) 
				&& Screen::Instance()->NeedsUpdate() )
		{
			guiRenderProfiler.Start();
			Render();
			guiRenderProfiler.Stop();
		}

		mBeatCounter++;

		if (!mNoFpsLimit)
			SDL_Delay(10);
	}
}

void GuiManager::Screenshot()
{
	string s = "saved/ss_" + timestamp(true) + ".png";
	buildDirectoryTree(s);
	
	IMG_SavePNG(s.c_str(), Screen::Instance()->Surface());

	SetAlert("\\c090* Screenshot saved to: " + s);
}

void GuiManager::SetAlert(string msg)
{
	mAlert = msg;
	
	console->AddMessage(msg);
	
	timers->Remove("KillGuiAlert");
	timers->Add("KillGuiAlert", GUI_ALERT_DISPLAY_MS, false, timer_killGuiAlert);
}
