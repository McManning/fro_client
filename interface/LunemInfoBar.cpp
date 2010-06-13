
#include "LunemInfoBar.h"
#include "../core/widgets/Label.h"

LunemInfoBar::LunemInfoBar(Widget* pParent, rect rPosition, bool bTop)
	: Frame(pParent, "", rPosition, "", false, false, false, true)
{
	string imageFile = "assets/gui/lunem_info_pane";
	if (bTop)
		imageFile += "_top";
		
	imageFile += ".png";

	SetImage(imageFile);
	
	mEmptyCaption = new Label(this, "", rect(), "EMPTY");
	mEmptyCaption->SetPosition( rect(rPosition.w / 2 - mEmptyCaption->mFont->GetWidth("EMPTY") / 2,
										rPosition.h / 2 - mEmptyCaption->mFont->GetHeight() / 2,
										0, 0)
								);
	
	mLunemName = new Label(this, "", rect(10, 10), "Nickname");
	mLunemLevel = new Label(this, "", rect(rPosition.w - 35, 10), "999");
}

void LunemInfoBar::SetLunem(Lunem* lu)
{
	mLunem = lu;
	
	if (!lu)
	{
		mEmptyCaption->SetVisible(true);
		mLunemName->SetVisible(false);
		mLunemLevel->SetVisible(false);
	}
	else
	{
		mEmptyCaption->SetVisible(false);
		mLunemName->SetVisible(true);
		mLunemName->SetCaption(lu->mNickname);
		mLunemLevel->SetVisible(true);
		mLunemLevel->SetCaption(its(lu->mLevel));
	}
}

