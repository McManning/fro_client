
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
	
	SetCaption(sCaption);
	SetPosition(rPosition);

	if (wParent)
		wParent->Add(this);
}

Label::~Label()
{

}

void Label::Render(uLong ms)
{
	rect pos = GetScreenPosition();

	if (mFont && !mCaption.empty())
		mFont->Render(Screen::Instance(), pos.x, pos.y, mCaption, mFontColor, mMaxWidth);

	Widget::Render(ms);
}

void Label::SetCaption(string text)
{
	mCaption = text;

	if (!mFont) return;

	if (!text.empty())
	{
		text = stripCodes(text);
		mPosition.w = mFont->GetWidth(text);
		mPosition.h = mFont->GetHeight();
		
		if (mPosition.w > mMaxWidth && mMaxWidth != 0)
		{
			mPosition.h = mFont->GetHeight(text, mMaxWidth);
			mPosition.w = mMaxWidth;
		}
	}

	FlagRender();
}

