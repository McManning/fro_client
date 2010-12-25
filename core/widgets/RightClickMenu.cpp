
#include "RightClickMenu.h"
#include "Button.h"
#include "../Screen.h"
#include "../Image.h"
#include "../GuiManager.h"
#include "../FontManager.h"
#include "../ResourceManager.h"
#include "../TimerManager.h"

const int RCM_EDGE_BUFFER_SIZE = 3;
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
		}
		else
		{
			WARNING("Invalid RightClickMenu Index " + b->mId + " Caption: " + b->GetCaption());
		}
		m->Die();
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

	mImage = resman->LoadImg("assets/gui/rcm_bg.png");

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
	SetSize(RCM_WIDTH, (mCallbacks.size() + 1) * 20);
	
	Button* b = new Button(this, its(mCallbacks.size()), rect(0, mCallbacks.size() * 20, RCM_WIDTH, 20), 
							text, callback_rightClickMenu);
			b->SetImage("assets/gui/rcm_button.png");
			b->mCenterAlign = false;
		
	sCallback c;
	c.func = callback;
	c.userdata = userdata;
			
	mCallbacks.push_back(c);
	
	rect r = GetScreenPosition();
	
	//if the button caption is wider than our shit, resize our shit
//	if (b->mCaptionImage->Width() > r.w + RCM_EDGE_BUFFER_SIZE * 2)
//		r.w = b->mCaptionImage->Width() + RCM_EDGE_BUFFER_SIZE * 2;
	
//	b->SetSize(b->mCaptionImage->Width() + RCM_EDGE_BUFFER_SIZE * 2, 20);
	
	// constrain to screen 
	if (r.x + r.w > SCREEN_WIDTH)
		r.x = SCREEN_WIDTH - r.w;
	if (r.y + r.h > SCREEN_HEIGHT)
		r.y = SCREEN_HEIGHT - r.h;
	SetPosition(r);
	
	DemandFocus();
}

void RightClickMenu::Render()
{
	rect r = GetScreenPosition();
	Image* scr = Screen::Instance();

	rect oldclip = scr->GetClip();
	scr->SetClip(r);
	
	if (mImage)
		mImage->RenderBox(scr, rect(0, 0, 24, 24), r);

	scr->SetClip(oldclip);

	//draw children
	Widget::Render();
}
