
#include <SDL/SDL.h>
#include "GuiManager.h"
#include "widgets/Console.h"
#include "widgets/Input.h"
#include "widgets/HintBalloon.h"
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
	SetAppTitle(mAppTitle);

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

// Gui Stuff
    mHoverTextHintBalloon = new HintBalloon(this);
	
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
	
	mHoverTextHintBalloon = NULL;

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
	switch (event->type)
	{
		case SDL_KEYDOWN:
		{
			if (hasKeyFocus)
				hasKeyFocus->Event(event);
			_sendToGlobalEventHandlers(event, hasKeyFocus);
		} break;
		case SDL_KEYUP:
		{

			if (event->key.keysym.sym == SDLK_PRINT)
			{
				Screenshot();
			}
			else if (event->key.keysym.sym == SDLK_HOME)
			{
				console->SetVisible(!console->IsVisible());
				console->MoveToTop();
			}

			if (hasKeyFocus)
				hasKeyFocus->Event(event);
			_sendToGlobalEventHandlers(event, hasKeyFocus);
		} break;
		case SDL_MOUSEBUTTONDOWN:
		{
			SetHasMouseFocus(GrabWidgetUnderXY(this, event->button.x, event->button.y));
			
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
					}
					else
					{

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
			SetHasMouseFocus(GrabWidgetUnderXY(this, event->button.x, event->button.y));
			//if (hasMouseFocus) hasMouseFocus->Event(event);
			//if (previousMouseFocus) previousMouseFocus->Event(event);
			
			if (hasKeyFocus)
				hasKeyFocus->Event(event);
			_sendToGlobalEventHandlers(event, hasKeyFocus);
		} break;
		case SDL_MOUSEMOTION:
		{
			SetMousePosition(event->motion.x, event->motion.y);
		
            SetHasMouseFocus(GrabWidgetUnderXY(this, event->motion.x, event->motion.y));

			if (hasMouseFocus) hasMouseFocus->Event(event);
			if (previousMouseFocus) previousMouseFocus->Event(event);
			_sendToGlobalEventHandlers(event, hasMouseFocus);

		} break;
		default: //don't know what it is, just pass it through globals.
		{
			_sendToGlobalEventHandlers(event, NULL);
		} break;
	}
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
	rect r = GetMouseRect();
	
	//If the cursor is over an input widget, render a caret version
	if (hasMouseFocus && hasMouseFocus->mType == WIDGET_INPUT)
	{
		mCursorImage->Render(scr, r.x, r.y, rect(17, 0, r.w, r.h));
	}
	else //render normal cursor
	{
		mCursorImage->Render(scr, r.x, r.y, rect(1, mCustomCursorSourceY, r.w, r.h));
	}
}

rect GuiManager::GetMouseRect()
{
	rect r;
	//If the cursor is over an input widget, create a caret version
	if (hasMouseFocus && hasMouseFocus->mType == WIDGET_INPUT)
	{
		r.x = GetMouseX() - 4;
		r.y = GetMouseY() - 10;
		r.w = 7;
		r.h = 22;
	}
	else //render normal cursor
	{
		r.x = GetMouseX();
		r.y = GetMouseY();
		r.w = 16;
		r.h = 22;
	}
	return r;
}

// TODO: Widget-ify this
void GuiManager::_renderCursorTipText(Image* scr, string& text)
{
}

void GuiManager::Render()
{
//	PRINT("GuiMan::Render");
	uLong ms;
	string text;
	
	rect r;
	uLong renderStart = SDL_GetTicks(); //keep track of how long this render takes
	Screen* scr = Screen::Instance();

	scr->PreRender();

	//Render widget tree
	Widget::Render();

	if (!mAlert.empty())
		mFont->Render(scr, 5, 5, mAlert, color(255,0,0));
		
	_renderStats(scr);
	
	if (SDL_GetAppState() & SDL_APPMOUSEFOCUS)
	{
		_renderCursor(scr);
	}

	scr->PostRender();

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

void GuiManager::SetMousePosition(int x, int y)
{
	g_screen->AddRect(GetMouseRect()); // Store old rect
	
	mLastMousePosition = mMousePosition;
	
//	SDL_WarpMouse(x, y);

	mMousePosition.x = x;
	mMousePosition.y = y;

	g_screen->AddRect(GetMouseRect()); // Store new rect
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
	if (hasMouseFocus == w) SetHasMouseFocus(NULL);
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

/**
	Called during SDL_MOUSEMOTION, SDL_MOUSEBUTTONDOWN/UP events and DereferenceWidget() 
*/
void GuiManager::SetHasMouseFocus(Widget* w)
{
	previousMouseFocus = hasMouseFocus;
	hasMouseFocus = w;
	
	// update hint balloon for widget hover text
    if (mHoverTextHintBalloon)
	{
        if (w && !w->mHoverText.empty())
        {
            mHoverTextHintBalloon->SetCaption(w->mHoverText);
            mHoverTextHintBalloon->SetVisible(true);
            
            
            rect r = mHoverTextHintBalloon->GetPosition();
            r.x = GetMouseX();
            r.y = GetMouseY() - r.h;
            mHoverTextHintBalloon->SetPosition(r);
        }
        else
        {
            mHoverTextHintBalloon->SetVisible(false);
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

	if (mGetUserAttention) // toggle alert
	{
		flashWindowState(true);
		
		//mTitleFlashOn = !mTitleFlashOn;
		/*if (mTitleFlashOn)
			SDL_WM_SetCaption("--------------------------------", NULL);
		else
			SDL_WM_SetCaption(mAppTitle.c_str(), NULL);*/
			
		mTitleFlashOn = true;
	}
	else if (mTitleFlashOn) // turn off alert
	{
		flashWindowState(false);
		mTitleFlashOn = false;
		
		SetAppTitle(mAppTitle); 
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
	
	// TODO: fps/bps is temp for testing purposes!
	string s = mAppTitle;
#ifdef DEBUG
	if (downloader)
	{
	   s += " [DL " + its(downloader->mQueued.size()); // Count of queued files to download
	   s += ">" + its(downloader->CountActiveDownloads()); // Count of active downloads
	   s += ">" + its(downloader->mCompleted.size()) + "] "; // Count of completed files
    }
    
	s += " [FPS:" + its(mFps); //frames rendered per second
	s += " BPS:" + its(mBps); //heartbeats a second
	s += " rMS:" + its(mRenderTime) + "]"; //time it took to render last frame
#endif

	SDL_WM_SetCaption(s.c_str(), NULL);
}

void GuiManager::Process()
{
	//If we're focused, don't worry about getting their attention
	if (SDL_GetAppState() & SDL_APPINPUTFOCUS)
	{
		mGetUserAttention = false;
		
		if (!mAppInputFocus)
		{
			mAppInputFocus = true;
			
			/* HACK: 
				http://bugzilla.libsdl.org/show_bug.cgi?id=659
				SDL does not like alt+tab on Windows, it sticks the alt modifier.
			*/ 
			SDL_SetModState(KMOD_NONE);
		}
	}
	else
	{
		mAppInputFocus = false;
	}
	
	//Poll and distribute events from SDL
	SDL_Event event;
	rect r;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				appState = APPSTATE_CLOSING;
				SetAppTitle("Shutting down, please wait..");
				break;
			case SDL_VIDEORESIZE:
			    r.w = event.resize.w;
				r.h = event.resize.h;
				Screen::Instance()->Resize(event.resize.w, event.resize.h);
				
				SetPosition(r);
				_sendToGlobalEventHandlers(&event, NULL);
				break;
			default:
				_distributeEvent(&event);
				break;
		}
		
		// A little sanity checking (TODO: A better placement)
		if (hasKeyFocus && !hasKeyFocus->IsVisible())
			hasKeyFocus = NULL;
		
		if (hasMouseFocus && !hasMouseFocus->IsVisible())
			hasMouseFocus = NULL;
	}

	messenger.Process(mTick);

	timers->Process(mTick);

	_cleanDeletionStack();

	//make sure focused window is always on top the other
	if (GetDemandsFocus())
		GetDemandsFocus()->MoveToTop();
		
	// Has absolute topmost priority (other than the mouse)
    if (mHoverTextHintBalloon->IsVisible())
        mHoverTextHintBalloon->MoveToTop();
}

void GuiManager::MainLoop()
{
	PRINT("--------------------------------------------------");
	PRINT("-Begin Main Loop----------------------------------\n  *\n *\n*\n");

	while (!isAppClosing())
	{
		mTick = SDL_GetTicks();

		Process();

		//Only render if: We've waited enough ticks or there's no wait time, AND the app isn't minimized
		if ( (mTick >= mNextRenderTick || mNoFpsLimit)
				&& (SDL_GetAppState() & SDL_APPACTIVE) )
		{
			Render();
#ifdef DEBUG
			SetAppTitle(mAppTitle); // so we can get fast-updated render info
#endif
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
