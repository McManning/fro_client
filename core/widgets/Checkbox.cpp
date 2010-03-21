
#include "Checkbox.h"
#include "../GuiManager.h"
#include "../Screen.h"
#include "../FontManager.h"

#define CHECKBOX_LABEL_BUFFER 3 /* Space between the checkbox and the label */

uShort getCheckboxState(Widget* parent, string id)
{
	Checkbox* c = (Checkbox*)parent->Get(id, false, WIDGET_CHECKBOX);
	if (c)
		return c->GetState();
	else
		return 0;
}

Checkbox::Checkbox()
{
	//TODO: load font name (along with other things) from XML and all that fancy crap
	mState = 0;
	mLastState = 0;
	mStateCount = 1;
	mType = WIDGET_CHECKBOX;
	mGroup = 0;
	mFont = fonts->Get();
}

Checkbox::Checkbox(Widget* wParent, string sId, rect rPosition, string sCaption, byte bGroup)
{
	mState = 0;
	mLastState = 0;
	mStateCount = 1;
	mType = WIDGET_CHECKBOX;
	mGroup = bGroup;
	mFont = fonts->Get();
	mId = sId;
	
	SetCaption(sCaption);
	mImage = gui->WidgetImageFromXml(this, "checkbox", "");

	SetPosition(rPosition);
	if (wParent)
		wParent->Add(this);
}

Checkbox::~Checkbox()
{

}

//TODO: Fix this code. There isn't supposed to be ANY position changing during render phase.
void Checkbox::Render(uLong ms)
{
	if (!mImage) return;
	
	Image* scr = Screen::Instance();

	//change the size value to match whatever is being displayed
	mPosition.w = CHECKBOX_LABEL_BUFFER + mImage->mSrc.w;
	if (!mCaption.empty())
		mPosition.w += mFont->GetWidth(stripCodes(mCaption));
	
	//height is whichever is larger, caption or src
	if (!mCaption.empty() && mFont->GetHeight() < mImage->mSrc.h) 
		mPosition.h = mFont->GetHeight();
	else 
		mPosition.h = mImage->mSrc.h;
		
	rect pos = GetScreenPosition();

	mImage->Render(this, scr, pos, mImage->mSrc.w * mState, 0);

	//Render text (centered on Y, just to the right of the image on X)
	if (mFont && !mCaption.empty())
		mFont->Render(scr, 
						pos.x + mImage->mSrc.w + CHECKBOX_LABEL_BUFFER, 
						pos.y + (mImage->mSrc.h / 2) - (mFont->GetHeight() / 2),
						mCaption, mFontColor
					);
	
	Widget::Render(ms); //TODO: do I need this?
}

void Checkbox::Event(SDL_Event* event)
{
	switch (event->type)
	{
		case SDL_MOUSEBUTTONDOWN: {
			if (event->button.button == MOUSE_BUTTON_LEFT)
			{
				//Cycle through states
				mState++;
				if (mState > mStateCount)
				{
					SetState(0);
					if (mGroup > 0) 
						SetState(mStateCount); //can't use 0 when cycling with a group. 
				}
				if (mGroup > 0 && mParent) //alert all other checkboxes of this group of the change
				{ 
					for (uShort i = 0; i < mParent->mChildren.size(); i++)
					{
						if (mParent->mChildren.at(i)->mType == WIDGET_CHECKBOX)
						{
							Checkbox* c = (Checkbox*)(mParent->mChildren.at(i));
							if (c != this && c->mGroup == mGroup)
								c->SetState(0);
						}	
					}
				}
				FlagRender(); 	
			}
		} break;
		default: break;
	}
	Widget::Event(event);
}

void Checkbox::SetCaption(string text)
{
	if (!mFont) return;
	mCaption = text;

	FlagRender();
}

void Checkbox::SetState(byte state)
{
	if (state <= mStateCount)
	{
		mLastState = mState;
		mState = state;
	}
}

void Checkbox::SetStateCount(byte count)
{
	mStateCount = count;
}

