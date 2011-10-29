
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


#include "YesNoPopup.h"

#include "Button.h"
#include "Multiline.h"
#include "Scrollbar.h"
#include "Label.h"

#include "../GuiManager.h"
#include "../ResourceManager.h"

void callback_YesNoPopupYes(Button* b)
{
	YesNoPopup* m = (YesNoPopup*)b->GetParent();
	
	if (m->onYesCallback)	
		m->onYesCallback(m);
		
	m->Die();
}

void callback_YesNoPopupNo(Button* b)
{
	YesNoPopup* m = (YesNoPopup*)b->GetParent();
	
	if (m->onNoCallback)	
		m->onNoCallback(m);
		
	m->Die();
}

YesNoPopup::YesNoPopup(string id, string title, string msg, bool useMultiline)
	: Frame(gui, id, rect(0,0,300,100), title, true, false, false, true)
{
	onYesCallback = NULL;
	onNoCallback = NULL;

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
		l->mMaxWidth = YESNOPOPUP_MAX_WIDTH - 10;
		l->SetCaption(msg);
		
		//set width based on label caption, up to max
		w = l->Width();
		h = l->Height();
		
		w += 10;
		h += 30 + 30;
		
		if (w < YESNOPOPUP_MIN_WIDTH)
			w = YESNOPOPUP_MIN_WIDTH;
		
		SetSize(w, h);
		
		//center label
		l->Center();
	}

	mYes = new Button(this, "", rect(10,Height()-25,20,20), "", callback_YesNoPopupYes);
		mYes->mHoverText = "Yes";
		mYes->SetImage("assets/buttons/okay.png");
	
	mNo = new Button(this, "", rect(Width()-30,Height()-25,20,20), "", callback_YesNoPopupNo);
		mNo->mHoverText = "No";
		mNo->SetImage("assets/buttons/no.png");
	
	ResizeChildren();
	Center();
	DemandFocus();
}

YesNoPopup::~YesNoPopup()
{
	
}

void YesNoPopup::Render()
{
	//if (gui->GetDemandsFocus() == this)
	//	gui->RenderDarkOverlay();
	
	Frame::Render();
}
