
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


#include "Button.h"
#include "../GuiManager.h"
#include "../Screen.h"
#include "../FontManager.h"
#include "../ResourceManager.h"
#include "RightClickMenu.h"

void callback_closeFrame(Button* b)
{
	if (b->GetParent())
		b->GetParent()->Die();
}

Button::Button(Widget* wParent, string sId, rect rPosition, string sCaption,
			void (*cbOnClick)(Button*))
{
	mType = WIDGET_BUTTON;
	mCaptionImage = NULL;
	mImage = NULL;
	mCenterAlign = true;
	
	mFont = fonts->Get();
	mId = sId;
	
	if (!sCaption.empty())
	{
		SetImage("assets/gui/button.png");
		SetCaption(sCaption);
	}
	
	onClickCallback = cbOnClick;
	onRightClickCallback = NULL;
	SetPosition(rPosition);
	if (wParent)
		wParent->Add(this);
}	

Button::~Button()
{
	resman->Unload(mCaptionImage);
}

void Button::Render()
{
	rect r = GetScreenPosition();
	Image* scr = Screen::Instance();
	
	scr->PushClip(r);
	
	if (mImage)
	{
		if (mCaptionImage) //gotta do a hedge render
		{
			mImage->RenderHorizontalEdge(scr, rect(0, CalculateImageOffset(20), 5, 20), r);
		}
		else //regular button
		{
			mImage->Render(scr, r.x, r.y, rect(0, CalculateImageOffset(Height()), r.w, r.h));
		}
	}

	if (mCaptionImage)
	{
		if (mCenterAlign)
			mCaptionImage->Render(Screen::Instance(), 
								r.x + (r.w / 2) - (mCaptionImage->Width() / 2),
								r.y + (r.h / 2) - (mCaptionImage->Height() / 2)
							);
		else
			mCaptionImage->Render(Screen::Instance(), 
								r.x + 5,
								r.y + (r.h / 2) - (mCaptionImage->Height() / 2)
							);
	}
	Widget::Render();
	
	scr->PopClip();
}

void Button::SetCaption(string text)
{
	mCaption = stripCodes(text);
	resman->Unload(mCaptionImage);
	mCaptionImage = NULL;
	
	if (!mFont || mCaption.empty()) return;

	mCaptionImage = resman->ImageFromSurface( mFont->RenderToSDL(mCaption.c_str(), mFontColor) );
	
/*	if (mCaptionImage) //resize if the caption doesn't fit
	{
		uShort w = mCaptionImage->Width() + 3 * 2;
		uShort h = mCaptionImage->Height() + 3 * 2;
		
		if (w > mPosition.w) 
			mPosition.w = w;
		if (h > mPosition.h)
			mPosition.h = h;
	}*/

	FlagRender();
}

void Button::Event(SDL_Event* event)
{
	if (event->type == SDL_MOUSEBUTTONUP && IsActive())
	{
		if (event->button.button == SDL_BUTTON_LEFT && onClickCallback)
			onClickCallback(this);
		else if (event->button.button == SDL_BUTTON_RIGHT && onRightClickCallback)
			onRightClickCallback(this);
		
		FlagRender();
	}
	else if (event->type == SDL_MOUSEMOTION)
	{
		// Our state is going to change, tell the renderer to update
		if (HadMouseFocus() || HasMouseFocus())
			FlagRender();
	}
		
	Widget::Event(event);	
}




