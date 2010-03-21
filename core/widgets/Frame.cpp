

#include "Frame.h"
#include "Label.h"
#include "Button.h"
#include "../Screen.h"
#include "../Image.h"
#include "../GuiManager.h"
#include "../FontManager.h"

void callback_frameCloseButton(Button* b)
{
	if (b->GetParent())
		b->GetParent()->Die();
}

Frame::Frame()
{
	mMoveable = false;
	mType = WIDGET_FRAME;
	//Mouse may move outside our frame while dragging. TODO: Find a way to fix that issue!
	gui->AddGlobalEventHandler(this);
	SetSizeable(false);
	mSizer = NULL;
	mCaption = NULL;
	mClose = NULL;
	mFont = fonts->Get();
}

Frame::Frame(Widget* wParent, string sId, rect rPosition, string sCaption,
			bool bMoveable, bool bSizeable, bool bCloseButton, bool bBackground)
{
	mFont = fonts->Get();
	mId = sId;
	mType = WIDGET_FRAME;

	//Mouse may move outside our frame while dragging. TODO: Find a way to fix that issue!
	gui->AddGlobalEventHandler(this);
	mMoveable = bMoveable;
	//mResizingBackgroundColor = color(0,64,128,100);
	mResizingBorderColor = color(0,64,200);

	if (bBackground)
		gui->WidgetImageFromXml(this, "framebg", "bg");

	SetSizeable(bSizeable);

	mSizer = NULL;
	if (bSizeable)
		mSizer = gui->WidgetImageFromXml(this, "framesizer", "sizer");

	mCaption = NULL;
	if (!sCaption.empty())
		mCaption = new Label(this, "", rect(10,4,0,0), sCaption);

	mClose = NULL;
	if (bCloseButton)
	{
		mClose = new Button(this, "", rect(0,0,20,20), "", callback_closeFrame);
		gui->WidgetImageFromXml(mClose, "frameclose");
	}

	SetPosition(rPosition);
	ResizeChildren(); //to do extra positioning crap for things like the close button

	if (wParent)
		wParent->Add(this);
}

Frame::~Frame()
{

}

void Frame::Render(uLong ms)
{
	rect r = GetScreenPosition();
	string s;
	Image* scr = Screen::Instance();
	
	if (mResizing) //draw custom stuff
	{

		scr->SetClip(r);
		//draw translucent rect..
		if (!isDefaultColor(mResizingBackgroundColor))
		{
			scr->DrawRect( r, mResizingBackgroundColor );
			
			Font* f = mFont;
			if (!f)
				f = fonts->Get();
			if (f) //render a caption indicating current size
			{
				s = its(r.w) + "x" + its(r.h);

				f->Render(scr, 
							r.x + (r.w / 2) - (f->GetWidth(s) / 2), 
							r.y + (r.h / 2) - (f->GetHeight() / 2), 
							s, color(255,255,255)
						);
			}
		}
		
		//draw dashed line at the top & bottom
		for (uShort x = r.x; x < r.x+r.w; x += 6)
		{
			scr->DrawLine( x, r.y, x+4, r.y, mResizingBorderColor, 3 );
			scr->DrawLine( x, r.y+r.h-3, x+4, r.y+r.h-3, mResizingBorderColor, 3 );
		}
		for (uShort y = r.y; y < r.y+r.h; y += 6)
		{
			scr->DrawLine( r.x, y, r.x, y+4, mResizingBorderColor, 3 );
			scr->DrawLine( r.x+r.w-3, y, r.x+r.w-3, y+4, mResizingBorderColor, 3 );
		}
	
		scr->SetClip();
	}
	else
	{
		scr->SetClip(r);
		RenderImages(ms); //just draw them all
		scr->SetClip();

		//draw children
		Widget::Render(ms);
	}
}

void Frame::Event(SDL_Event* event)
{
	switch (event->type)
	{
		case SDL_MOUSEBUTTONDOWN: {
			if (event->button.button == SDL_BUTTON_LEFT && HasKeyFocus())
			{
				mDrag.x = event->button.x - mPosition.x;
				mDrag.y = event->button.y - mPosition.y;
				if (mDrag.x > mPosition.w - 26
					&& mDrag.y > mPosition.h - 26 && mSizeable) //bottom right corner
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
					SetPosition( rect(mPosition.x, mPosition.y,
										mPosition.w + event->motion.xrel,
										mPosition.h + event->motion.yrel)
								);
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
	if (mSizer)
	{
		mSizer->mDst.x = mPosition.w - mSizer->mDst.w;
		mSizer->mDst.y = mPosition.h - mSizer->mDst.h;
	}
	if (mClose)
	{
		mClose->SetPosition( rect(Width() - mClose->Width() - 4, 2, mClose->Width(), mClose->Height()) );
	}
	
	for (int i = 0; i < mChildren.size(); i++)
	{
		if (mChildren.at(i) && mChildren.at(i)->mType == WIDGET_FRAME)
			((Frame*)mChildren.at(i))->ResizeChildren();
	}
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
		mCaption->SetCaption(caption);	
}


