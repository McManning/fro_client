
#include "HintBalloon.h"
#include "../Screen.h"
#include "../FontManager.h"

HintBalloon::HintBalloon(Widget* wParent)
{
	mType = WIDGET_HINTBALLOON;
	mMaxWidth = 0;
	
	mFont = fonts->Get();
	SetImage("assets/gui/gui.png");

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
		mImage->RenderBox(scr, rect(24, 0, 5, 5), pos);
	
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
    		text = stripCodes(text); //clean it for calculating size
    		mPosition.w = mFont->GetWidth(text) + 10;
    		if (mMaxWidth != 0 && mPosition.w > mMaxWidth)
    			mPosition.w = mMaxWidth + 10;
    		
    		mPosition.h = mFont->GetHeight(text, mMaxWidth) + 10;
    	}
    
    	FlagRender();
    }
}

