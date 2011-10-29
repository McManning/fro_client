
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


#include "SmallSelect.h"
#include "Button.h"
#include "../GuiManager.h"
#include "../FontManager.h"
#include "../Screen.h"
#include "../ResourceManager.h"

void callback_smallSelectLeft(Button* b)
{
	SmallSelect* s = (SmallSelect*)b->GetParent();

	if (s->mSelectedIndex == 0)
		s->mSelectedIndex = s->mItems.size() - 1;
	else
		s->mSelectedIndex--;
		
	s->FlagRender();
		
	if (s->onChangeCallback)
		s->onChangeCallback(s);
}

void callback_smallSelectRight(Button* b)
{
	SmallSelect* s = (SmallSelect*)b->GetParent();
	
	if (s->mSelectedIndex + 1 >= s->mItems.size())
		s->mSelectedIndex = 0;
	else
		s->mSelectedIndex++;
	
	s->FlagRender();
	
	if (s->onChangeCallback)
		s->onChangeCallback(s);
}

SmallSelect::SmallSelect()
{
	onChangeCallback = NULL;
	mType = WIDGET_SMALLSELECT;
	mLeft = NULL;
	mRight = NULL;
	mBackgroundImage = NULL;
	mSelectedIndex = 0;
}

SmallSelect::SmallSelect(Widget* wParent, string sId, rect rPosition,
			void (*cbOnChange)(SmallSelect*))
{
	onChangeCallback = cbOnChange;
	mType = WIDGET_SMALLSELECT;
	mSelectedIndex = 0;

	mFont = fonts->Get();
	mId = sId;

	//create buttons and add
	mLeft = new Button(this, "left", rect(0,0,15,15), "", callback_smallSelectLeft);
	mLeft->SetImage("assets/gui/smallselect_left.png");
	
	mRight = new Button(this, "right", rect(0,0,15,15), "", callback_smallSelectRight);
	mRight->SetImage("assets/gui/smallselect_right.png");
	
	mBackgroundImage = resman->LoadImg("assets/gui/smallselect_bg.png");
	
	SetPosition(rPosition);
	if (wParent)
		wParent->Add(this);
}

SmallSelect::~SmallSelect()
{
	resman->Unload(mBackgroundImage);
}

void SmallSelect::Render()
{
	rect r = GetScreenPosition();
	Image* scr = Screen::Instance();
	
	r.w += 15; //HACK TODO: Figure out why r.w is too short here!
	
	if (mBackgroundImage)
		mBackgroundImage->RenderHorizontalEdge(scr, rect(0, 0, 20, 20), r);
	
	//draw mItems.at(mSelectedIndex), clipped between the two buttons
	if (mSelectedIndex < mItems.size() && mFont)
	{
		if (mLeft)
		{
			r.x += mLeft->Width() + SMALLSELECT_BUFFER;
			r.w -= (mLeft->Width() + SMALLSELECT_BUFFER);
		}
		if (mRight)
		{
			r.w -= (mRight->Width() + SMALLSELECT_BUFFER);
		}
		scr->PushClip(r);

		//draw text
		mFont->Render(scr, r.x, r.y + (r.h / 2 - mFont->GetHeight() / 2), mItems.at(mSelectedIndex), mFontColor);
		
		scr->PopClip();
	}
	
	Widget::Render(); //draws children
}

void SmallSelect::Event(SDL_Event* event)
{
	//we don't really care atm
}

void SmallSelect::SetPosition(rect r)
{
	r.h = 20;
	
	Widget::SetPosition(r);
	
	if (mLeft)
		mLeft->SetPosition( rect(0,2,mLeft->Width(),mLeft->Height()) );
	if (mRight)
		mRight->SetPosition( rect(r.w - mRight->Width(),2,mRight->Width(),mRight->Height()) );
}

string SmallSelect::GetSelectedItem() const
{
	if (mItems.empty() || mSelectedIndex >= mItems.size())
		return string();
		
	return mItems.at(mSelectedIndex);
}


