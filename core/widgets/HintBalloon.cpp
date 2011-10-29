
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


#include "HintBalloon.h"
#include "../Screen.h"
#include "../FontManager.h"

HintBalloon::HintBalloon(Widget* wParent)
{
	mType = WIDGET_HINTBALLOON;
	mMaxWidth = 0;
	
	mFont = fonts->Get();
	SetImage("assets/gui/hint.png");

	if (wParent)
		wParent->Add(this);
}

HintBalloon::~HintBalloon()
{

}

void HintBalloon::Render()
{
	rect pos = GetScreenPosition();
	Screen* scr = Screen::Instance();
	
	// draw background
	if (mImage)
		mImage->RenderBox(scr, rect(0, 0, 5, 5), pos);
	
	// draw caption
	if (mFont && !mCaption.empty())
		mFont->Render(scr, pos.x + 5, pos.y + 5, mCaption, mFontColor, mMaxWidth);

	Widget::Render();
}

void HintBalloon::SetCaption(string text)
{
    if (mCaption != text)
    {
    	mCaption = text;
    
    	if (mFont && !text.empty())
    	{
    		mPosition.w = mFont->GetWidth(text, true) + 10;
    		if (mMaxWidth != 0 && mPosition.w > mMaxWidth)
    			mPosition.w = mMaxWidth + 10;
    		
    		mPosition.h = mFont->GetHeight(text, mMaxWidth) + 10;
    	}
    
    	FlagRender();
    }
}

