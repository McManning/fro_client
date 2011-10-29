
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


#include "Label.h"
#include "../Screen.h"
#include "../GuiManager.h"
#include "../FontManager.h"
#include "Console.h"

Label::Label(Widget* wParent, string sId, rect rPosition, string sCaption)
{
	mType = WIDGET_LABEL;
	mMaxWidth = 0;
	
	mFont = fonts->Get();
	mId = sId;
	
	SetPosition(rPosition);
	SetCaption(sCaption);

	if (wParent)
		wParent->Add(this);
}

Label::~Label()
{

}

void Label::Render()
{
	rect pos = GetScreenPosition();

	if (mFont && !mCaption.empty())
		mFont->Render(Screen::Instance(), pos.x, pos.y, mCaption, mFontColor, mMaxWidth);

	Widget::Render();
}

void Label::SetCaption(string text)
{
    if (mCaption != text)
    {    
    	mCaption = text;
    
    	if (!mFont) return;
    	
    	rect r = mPosition;
	
    	if (!text.empty())
    	{
    		r.w = mFont->GetWidth(text, true);
    		if (mMaxWidth != 0 && r.w > mMaxWidth)
    			r.w = mMaxWidth;
    			
    		r.h = mFont->GetHeight(text, mMaxWidth);
    	}
    	
    	SetPosition(r);
    	FlagRender(); //force a redraw, even if it doesn't change in size (content changed)
    	
    }
}

