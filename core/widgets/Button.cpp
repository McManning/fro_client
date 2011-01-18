
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
}

void Button::SetCaption(string text)
{
	mCaption = text;
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
	if (event->type == SDL_MOUSEBUTTONUP)
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




