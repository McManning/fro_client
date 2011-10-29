
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


#include "MessagePopup.h"

#include "../core/widgets/Button.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/Scrollbar.h"
#include "../core/widgets/Label.h"

void callback_MessagePopupOkay(Button* b)
{
	if (b && b->GetParent())
		b->GetParent()->Die();
	//TODO: callback or something?
}

MessagePopup::MessagePopup(string id, string title, string msg)
	: Frame(gui, id, rect(0,0,300,100), title, true, false, false, true)
{
	/*mMessage = new Multiline(this, "", rect());
		mMessage->mScrollbar->mTabImage->mDst.h = 10; //shrink our scrollbar a bit to give more room
		mMessage->AddAdvMessage(msg);
		mMessage->SetTopLine(0); //set it to display the top of the list.
		
	mOkay = new Button(this, "", rect(0,0,20,20), "", callback_MessagePopupOkay);
	mOkay->mHoverText = "Okay";
		makeImage(mOkay, "", "assets/button20.png", rect(60,0,20,20), 
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);

	ResizeChildren();
	Center();
	DemandFocus();*/

	Label* l;
	l = new Label(this, "caption", rect(5, 30, 0, 0));
	l->mMaxWidth = MESSAGEPOPUP_MAX_WIDTH - 10;
	l->SetCaption(msg);

	//set width based on label caption, up to max
	uShort w = l->Width();
	uShort h = l->Height();

	w += 10;
	h += 30 + 30;
	
	if (w < MESSAGEPOPUP_MIN_WIDTH)
		w = MESSAGEPOPUP_MIN_WIDTH;

	SetSize(w, h);
	
	//center label
	l->Center();
	//l->SetPosition( rect(l->GetPosition().x, 30, l->Width(), l->Height()) );
		
	mOkay = new Button(this, "", rect(Width()-43,Height()-25,20,20), "", callback_MessagePopupOkay);
	mOkay->mHoverText = "Okay";
		makeImage(mOkay, "", "assets/button20.png", rect(60,0,20,20), 
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);

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
