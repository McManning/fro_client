
#include "MessagePopup.h"

#include "Button.h"
#include "Multiline.h"
#include "Scrollbar.h"
#include "Label.h"

#include "../GuiManager.h"
#include "../ResourceManager.h"

void callback_MessagePopupOkay(Button* b)
{
	MessagePopup* m = (MessagePopup*)b->GetParent();
	
	if (m->onCloseCallback)	
		m->onCloseCallback(m);
		
	m->Die();
}

MessagePopup::MessagePopup(string id, string title, string msg, bool useMultiline)
	: Frame(gui, id, rect(0,0,300,100), title, true, false, false, true)
{
	onCloseCallback = NULL;

	//set width based on label caption, up to max
	uShort w;
	uShort h;

	if (useMultiline)
	{
		SetSize(300, 300);
		
		Multiline* m;
		m = new Multiline(this, "msg", rect(5,30,Width()-10,Height()-60));
		//TODO: m->mScrollbar->mTabImage->mDst.h = 10; //shrink our scrollbar a bit to give more room
		m->AddMessage(msg);	
		m->SetTopLine(0); //set it to display the top of the list.
	}
	else //label version, resizes itself based on the label size.
	{
		Label* l;
		l = new Label(this, "caption", rect(5, 30, 0, 0));
		l->mMaxWidth = MESSAGEPOPUP_MAX_WIDTH - 10;
		l->SetCaption(msg);
		
		//set width based on label caption, up to max
		w = l->Width();
		h = l->Height();
		
		w += 10;
		h += 30 + 30;
		
		if (w < MESSAGEPOPUP_MIN_WIDTH)
			w = MESSAGEPOPUP_MIN_WIDTH;
		
		SetSize(w, h);
		
		//center label
		l->Center();
	}

	mOkay = new Button(this, "", rect(Width()-43,Height()-25,20,20), "", callback_MessagePopupOkay);
		mOkay->mHoverText = "Okay";
		mOkay->SetImage("assets/buttons/okay.png");

	ResizeChildren();
	Center();
	DemandFocus();
}

MessagePopup::~MessagePopup()
{
	
}

void MessagePopup::Render()
{
	if (gui->GetDemandsFocus() == this)
		gui->RenderDarkOverlay();
	
	Frame::Render();
}

/*
void MessagePopup::ResizeChildren() //overridden so we can move things around properly
{
	mMessage->SetPosition( rect(10, 30, Width() - 20, Height() - 60) );
	mOkay->SetPosition( rect(Width()-43,Height()-25,20,20) );	

	Frame::ResizeChildren(); //takes care of titlebar stuff (close button, sizer, caption, etc)
}
*/
