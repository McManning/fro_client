
#include "Label.h"
#include "../Screen.h"
#include "../GuiManager.h"
#include "../FontManager.h"
#include "Console.h"

Label::Label()
{
	mType = WIDGET_LABEL;
	mMaxWidth = 0;
}

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
	mCaption = text;

	if (!mFont) return;

	if (!text.empty())
	{
		text = stripCodes(text); //clean it for calculating size
		mPosition.w = mFont->GetWidth(text);
		if (mMaxWidth != 0 && mPosition.w > mMaxWidth)
			mPosition.w = mMaxWidth;
			
		mPosition.h = mFont->GetHeight(text, mMaxWidth);
	}

	FlagRender();
}

