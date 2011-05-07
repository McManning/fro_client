
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
    		text = stripCodes(text); //clean it for calculating size
    		r.w = mFont->GetWidth(text);
    		if (mMaxWidth != 0 && r.w > mMaxWidth)
    			r.w = mMaxWidth;
    			
    		r.h = mFont->GetHeight(text, mMaxWidth);
    	}
    	
    	SetPosition(r);
    	FlagRender(); //force a redraw, even if it doesn't change in size (content changed)
    	
    }
}

