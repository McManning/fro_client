
/*
 * Copyright (c) 2011 Chase McManning
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */


#ifndef _GUIMANAGER_H_
#define _GUIMANAGER_H_

#include "Common.h"
#include "widgets/Widget.h"

#define HIGHLIGHT_COLOR color(155,185,230)

#define GUI_ALERT_DISPLAY_MS 10000

enum { //copying SDL_BUTTON masks for easy conversion
	MOUSE_BUTTON_LEFT = 1,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_WHEELUP,
	MOUSE_BUTTON_WHEELDOWN
};

class Font;
class HintBalloon;
class Label;
class GuiManager : public Widget
{
  public:
	GuiManager();
	~GuiManager();

	/*	Pump events from SDL's event queue and distribute to our widgets. 
		Also handle Process() calls for other related modules 
	*/
	void Process();
	
	/*	The applications absolute MAIN heartbeat */
	void MainLoop();

	Widget* GrabWidgetUnderXY(Widget* root, sShort x, sShort y) const;

	void Render(); //Overloaded from Widget
	void RenderDarkOverlay();
	
	bool IsMouseButtonDown(byte button, int* x = NULL, int* y = NULL) const;
	bool IsKeyDown(int key) const;

	sShort GetMouseVelocityX() const { return mMousePosition.x - mLastMousePosition.x; };
	sShort GetMouseVelocityY() const { return mMousePosition.y - mLastMousePosition.y; };

	uShort GetMouseX() const { return mMousePosition.x; };
	uShort GetMouseY() const { return mMousePosition.y; };

	uShort GetPreviousMouseX() const { return mLastMousePosition.x; };
	uShort GetPreviousMouseY() const { return mLastMousePosition.y; };
	
	rect GetMouseRect();
	
	void SetMousePosition(int x, int y);

	//Widgets in this list will get Event() even if it's not for them directly.
	std::vector<Widget*> mGlobalEventHandlers;
	void AddGlobalEventHandler(Widget* w);
	void RemoveGlobalEventHandler(Widget* w);

	/*	Queue a widget to be deleted. Will not actually delete the instance itself, 
		but will remember for when _cleanDeletionStack is called. This way we don't
		arbitrarily break pointers. It's kinda gross, but necessary.
	*/
	void RemoveWidget(Widget* w);

	/*	Makes sure the specified widget doesn't have any sort of focus (key, mouse, demanding, etc) */
	void DereferenceWidget(Widget* w);
	
	void SetHasMouseFocus(Widget* w);
	
	/*	Flashes the applications title bar if the application doesn't have key focus, and until it gains said focus. 
		Will also play a sound if settings allow it
	*/
	void GetUserAttention();
	
	/*	Timer callback every second. Resets various statistics and flashes the titlebar if necessary */
	void ThinkInSeconds(uLong ms);
	
	void SetAppTitle(string caption);
	
	uLong GetTick() const { return mTick; };

	void SetAlert(string msg);

	void Screenshot();
	
	void PrecacheColorizedAssets();
	void UnloadColorizedAssets();
	void ColorizeGui(color c);

	Image* mCursorImage;

	Widget* hasMouseFocus;
	Widget* hasKeyFocus;
	Widget* previousMouseFocus;

	Widget* GetDemandsFocus() const;
	void RemoveFromDemandFocusStack(Widget* w); 
	void AddToDemandFocusStack(Widget* w) { mDemandFocusStack.push_back(w); };
	
	//List of widgets that demand client focus. Last one in the stack gets it.
	std::vector<Widget*> mDemandFocusStack;

	uLong mTick;
	uLong mNextRenderTick;
	uShort mFpsCap;
	bool mUseLowCpu;
	
	uShort mFrameCounter;
	uShort mBeatCounter;
	
	//The actual values holding our results
	uShort mFps; //Frames per second
	uShort mBps; //Heartbeats per second
	uShort mRenderTime; //time it takes for a single Render() call

	/*
		disables all SDL_Delays in the main loop and frame caps to get maximum performance
		Quite unnecessary but useful for testing limits
	*/
	bool mNoFpsLimit; 

	string mAppTitle;
	
	//Used by a lot of widgets, and it takes a while to rebuild, so it's stored here.
	Image* mDarkOverlay;
	
	string mAlert;

	// Replace default cursor with another image
	int mCustomCursorSourceY;
	
	bool mAppInputFocus;
	bool mInputFlashStateOn;
	bool mPendingScreenshot;
	
	color mBaseColor; // for use with colorization of assets
	
	int mSystemAlertType;
	
  private:
	rect mMousePosition;
	rect mLastMousePosition;
	
	/*	Sends the event to various widgets, depending on the type of event fired. (Keyboard, mouse button, mouse movement, etc)
		Will also send the event to all global handlers.
	*/
	void _distributeEvent(SDL_Event* event);
		
	/*	Pass the event to all global listeners excluding the one defined. This is to prevent double-firing certain events.
		(For example, if it's a mousemove and it hasMouseFocus is in the global handlers)
	*/
	void _sendToGlobalEventHandlers(SDL_Event* event, Widget* excludingA = NULL, Widget* excludingB = NULL);

	/*	Free memory of all widgets queued to delete. */
	void _cleanDeletionStack();
	
	void _renderCursor(Image* scr);
	
		/*	Recalculate the next tick we should do a full render call on */
	void _getNextRenderTick(uLong ms);

	void _updateStatsLabel();

	std::vector<Widget*> mDeletionStack;
	
	HintBalloon* mHoverTextHintBalloon;
	Label* mStatsLabel;
};

extern GuiManager* gui;

#endif //_GUIMANAGER_H_

