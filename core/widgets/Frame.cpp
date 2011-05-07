
#include "Frame.h"
#include "Label.h"
#include "Button.h"
#include "../MessageManager.h"
#include "../Screen.h"
#include "../Image.h"
#include "../GuiManager.h"
#include "../FontManager.h"
#include "../ResourceManager.h"

#include "Console.h"

void callback_frameCloseButton(Button* b)
{
	if (b->GetParent())
		b->GetParent()->Die();
}

Frame::Frame(Widget* wParent, string sId, rect rPosition, string sCaption,
			bool bMoveable, bool bSizeable, bool bCloseButton, bool bBackground)
{
	mFont = fonts->Get();
	mId = sId;
	mType = WIDGET_FRAME;
	mBoxRender = true;
	mSavedGuiBaseColor = gui->mBaseColor;
	
	//Mouse may move outside our frame while dragging. TODO: Find a way to fix that issue!
	gui->AddGlobalEventHandler(this);
	mMoveable = bMoveable;

	if (bBackground)
		SetImage("assets/gui/frame_bg.png");
		
	SetSizeable(bSizeable);

	mSizer = NULL;
	if (bSizeable)
		mSizer = resman->LoadImg("assets/gui/frame_sizer.png");

	mCaption = NULL;
	if (!sCaption.empty())
	{
		mCaption = new Label(this, "", rect(), "");
		//mCaption->mFont = fonts->Get("", 0, TTF_STYLE_BOLD);
		SetCaption(sCaption);
	}

	mClose = NULL;
	if (bCloseButton)
	{
		mClose = new Button(this, "", rect(0,0,20,20), "", callback_closeFrame);
		mClose->SetImage("assets/gui/frame_close.png");
	}

	SetPosition(rPosition);
	ResizeChildren(); //to do extra positioning crap for things like the close button

	if (wParent)
		wParent->Add(this);
		
	MessageData md("CREATE_FRAME");
	md.WriteString("id", mId);
	messenger.Dispatch(md);
}

Frame::~Frame()
{
	resman->Unload(mSizer);
	
	MessageData md("DESTROY_FRAME");
	md.WriteString("id", mId);
	messenger.Dispatch(md);
}

void Frame::Render()
{
	rect r = GetScreenPosition();
	Image* scr = Screen::Instance();

	if (mResizing) //draw custom stuff
	{
		scr->PushClip(r);

		//draw dashed line at the top & bottom
		for (uShort x = r.x; x < r.x+r.w; x += 6)
		{
			scr->DrawLine( x, r.y, x+4, r.y, gui->mBaseColor, 3 );
			scr->DrawLine( x, r.y+r.h-3, x+4, r.y+r.h-3, gui->mBaseColor, 3 );
		}
		for (uShort y = r.y; y < r.y+r.h; y += 6)
		{
			scr->DrawLine( r.x, y, r.x, y+4, gui->mBaseColor, 3 );
			scr->DrawLine( r.x+r.w-3, y, r.x+r.w-3, y+4, gui->mBaseColor, 3 );
		}
	
		scr->PopClip();
	}
	else
	{
		scr->PushClip(r);
		
		if (mImage)
		{
			if (mBoxRender)
				mImage->RenderBox(scr, rect(0, 0, 24, 24), r);
			else
				mImage->Render(scr, r.x, r.y, rect(0, 0, r.w, r.h));
		}
		
		if (mSizer)
			mSizer->Render(scr, r.x + r.w - 24, r.y + r.h - 24);
		
		//draw children
		Widget::Render();
		
		scr->PopClip();
	}
}

void Frame::_readjustBaseColor()
{
	mSavedGuiBaseColor = gui->mBaseColor;

	if (mCaption)
		SetCaption(mCaption->GetCaption());
}

void Frame::Event(SDL_Event* event)
{
	if (mSavedGuiBaseColor.r != gui->mBaseColor.r 
		|| mSavedGuiBaseColor.g != gui->mBaseColor.g
		|| mSavedGuiBaseColor.b != gui->mBaseColor.b)
	{
		_readjustBaseColor();	
	}
	
	switch (event->type)
	{
		case SDL_MOUSEBUTTONDOWN: {
			if (event->button.button == SDL_BUTTON_LEFT && HasKeyFocus())
			{
				mDrag.x = event->button.x - mPosition.x;
				mDrag.y = event->button.y - mPosition.y;
				if (mSizeable && mDrag.x > mPosition.w - 26
					&& mDrag.y > mPosition.h - 26) //bottom right corner
				{
					mResizing = true;
					SetActive(false); //make children ignore events during resize
				}
			}
		} break;
		case SDL_MOUSEBUTTONUP: {
			mDrag.x = mDrag.y = 0;
			if (mResizing)
			{
				mResizing = false;
				ResizeChildren(); //have children move themselves to adjust and whatnot
				SetActive(true);
			}
		} break;
		case SDL_MOUSEMOTION: {
			if ((mDrag.x != 0 || mDrag.y != 0) && HasKeyFocus())
			{
				if (mResizing)
				{
                    SetSize(mPosition.w + event->motion.xrel, mPosition.h + event->motion.yrel);
				}
				else if (mMoveable)
				{
					SetPosition( rect(event->motion.x - mDrag.x,
										event->motion.y - mDrag.y,
										mPosition.w, mPosition.h) );
				}
			}
		} break;
		default: break;
	}

	Widget::Event(event);
}

void Frame::ResizeChildren() //so that children don't size themselves while resizing (would lag like hell)
{
	if (mClose)
		mClose->SetPosition( rect(Width() - mClose->Width() - 4, 2, mClose->Width(), mClose->Height()) );
	
	if (mCaption)
		mCaption->SetPosition( rect(10,6,0,0) );
	
	for (int i = 0; i < mChildren.size(); i++)
	{
		if (mChildren.at(i) && mChildren.at(i)->mType == WIDGET_FRAME)
			((Frame*)mChildren.at(i))->ResizeChildren();
	}
	
	FlagRender();
}

void Frame::SetPosition(rect r)
{
	Widget::SetPosition(r);
}

void Frame::SetSizeable(bool val)
{
	mSizeable = val;
	mResizing = false;
}

void Frame::SetCaption(string caption)
{
	if (mCaption)
	{
		// if it's too dark, invert the color of our caption
		if (isDark(mSavedGuiBaseColor))
			mCaption->SetCaption("\\c999" + stripCodes(caption));
		else
			mCaption->SetCaption(stripCodes(caption));
	}
}


