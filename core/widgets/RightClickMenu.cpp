
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


#include "RightClickMenu.h"
#include "Button.h"
#include "../Screen.h"
#include "../Image.h"
#include "../GuiManager.h"
#include "../FontManager.h"
#include "../ResourceManager.h"
#include "../TimerManager.h"
#include "Console.h"

const int RCM_EDGE_BUFFER_SIZE = 5;
const int RCM_WIDTH = 100;

void callback_rightClickMenu(Button* b)
{
	RightClickMenu* m = (RightClickMenu*)b->GetParent();
	
	if (m)
	{
		int index = sti(b->mId);
		if (m->mCallbacks.size() > index && m->mCallbacks.at(index).func != NULL)
		{
			m->mCallbacks.at(index).func(m, m->mCallbacks.at(index).userdata);
			m->Die();
		}
		/*else
		{
			WARNING("Invalid RightClickMenu Index " + b->mId + " Caption: " + b->GetCaption());
		}*/
	}
}

uShort timer_SetTempWidget(timer* t, uLong u)
{
	Widget* w = (Widget*)t->userData;
	w->SetTemporary(true);
	return TIMER_DESTROY;
}

RightClickMenu::RightClickMenu()
{
	mType = WIDGET_FRAME;
	onCloseCallback = NULL;
	
	//mTemporary = true; // if we do anything outside this widget, it'll destroy itself
	
	//Mouse may move outside our frame while dragging. TODO: Find a way to fix that issue!
	gui->AddGlobalEventHandler(this);

	SetImage("assets/gui/rcm_bg.png");

	SetPosition( gui->GetMouseRect() );

	gui->Add(this);
	
	// Very VERY gross hack to activate temporary state after the event we were created on
	timers->Add("", 1, false, timer_SetTempWidget, NULL, this);
}

RightClickMenu::~RightClickMenu()
{
	if (onCloseCallback)
		onCloseCallback(this);	
}

void RightClickMenu::AddOption(string text, void (*callback)(RightClickMenu*, void*), void* userdata)
{
	Button* b = new Button(this, its(mCallbacks.size()), rect(0, mCallbacks.size() * 20, RCM_WIDTH, 20), 
							text, callback_rightClickMenu);
			if (callback)
				b->SetImage("assets/gui/rcm_button.png");
			else
				b->SetImage("assets/gui/rcm_spacer.png");
				
			b->mCenterAlign = false;
		
	sCallback c;
	c.func = callback;
	c.userdata = userdata;
			
	mCallbacks.push_back(c);
	
	rect r = GetScreenPosition();
	r.h = mCallbacks.size() * 20;
	
	//if the button caption is wider than our shit, resize our shit
	if (b->mCaptionImage->Width() + RCM_EDGE_BUFFER_SIZE * 2 > r.w)
		r.w = b->mCaptionImage->Width() + RCM_EDGE_BUFFER_SIZE * 2;
	
	if (r.w < RCM_WIDTH) 
		r.w = RCM_WIDTH;

	DEBUGOUT("Adding " + text + " with width: " + its(b->mCaptionImage->Width()) + " to width " + its(r.w));

	// constrain to screen 
	if (r.x + r.w > SCREEN_WIDTH)
		r.x = SCREEN_WIDTH - r.w;
	if (r.y + r.h > SCREEN_HEIGHT)
		r.y = SCREEN_HEIGHT - r.h;
	SetPosition(r);
	
	// resize all buttons
	for (int i = 0; i < mChildren.size(); ++i)
	{
		mChildren.at(i)->SetSize(r.w, 20);
	}
	
	DemandFocus();
}

void RightClickMenu::Render()
{
	rect r = GetScreenPosition();
	Image* scr = Screen::Instance();

	scr->PushClip(r);
	
	if (mImage)
		mImage->RenderBox(scr, rect(0, 0, 24, 24), r);

	scr->PopClip();

	//draw children
	Widget::Render();
}
