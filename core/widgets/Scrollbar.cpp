
#include "Scrollbar.h"
#include "Button.h"
#include "../GuiManager.h"
#include "../FontManager.h"
#include "../Screen.h"
#include "../ResourceManager.h"

uShort getScrollbarValue(Widget* parent, string id)
{
	Scrollbar* s = (Scrollbar*)parent->Get(id, false, WIDGET_SCROLLBAR);
	if (s)
		return s->GetValue();
	else
		return 0;
}

void callback_scrollbarButtonDown(Button* b)
{
	Scrollbar* s = (Scrollbar*)b->GetParent();
	s->SetValue(s->GetValue() + 1);
}

void callback_scrollbarButtonUp(Button* b)
{
	Scrollbar* s = (Scrollbar*)b->GetParent();
	s->SetValue(s->GetValue() - 1);
}

Scrollbar::Scrollbar(Widget* wParent, string sId, rect rPosition,
					byte bOrientation, uShort uMax, uShort uBigScroll, uShort uValue,
					void (*onValueChange)(Scrollbar*))
{
	mScrollPress = -1;
	mShowValue = false;
	onValueChangeCallback = NULL;
	mType = WIDGET_SCROLLBAR;
	mFont = fonts->Get();
	mId = sId;
	mImage = NULL;
	mTabImage = NULL;
	
	SetMax(uMax);
	SetBigScroll(uBigScroll);
	SetValue(uValue);
	
	onValueChangeCallback = onValueChange;
	mOrientation = bOrientation;

	mValueDown = new Button(this, "down", rect(0,0,15,15), "", callback_scrollbarButtonDown);
	mValueUp = new Button(this, "up", rect(0,0,15,15), "", callback_scrollbarButtonUp);

	string base;
	if (mOrientation == VERTICAL)
		base = "assets/gui/vscroller";
	else
		base = "assets/gui/hscroller";

	SetImageBase(base);
	
	if (mOrientation == VERTICAL)
	{
		rPosition.w = 15;
		if (rPosition.h < mValueUp->Height() + mValueDown->Height())
			rPosition.h = mValueUp->Height() + mValueDown->Height();
	}
	else
	{
		rPosition.h = 15;
		if (rPosition.w < mValueUp->Width() + mValueDown->Width())
			rPosition.w = mValueUp->Width() + mValueDown->Width();
	}

	SetPosition(rPosition);
	if (wParent)
		wParent->Add(this);

}

Scrollbar::~Scrollbar()
{
	resman->Unload(mTabImage);
}

void Scrollbar::Render()
{
	rect r = GetScreenPosition();
	uShort ss;
	Image* scr = Screen::Instance();

    // draw the elevator icon
    if (mOrientation == VERTICAL)
	{
		if (mImage)
			mImage->RenderVerticalEdge(scr, rect(0, 0, r.w, r.w), r);
		
		if (GetMax() > 0)
		{
			ss = r.y + ScrollerTop();
			if (ss < r.y) ss = r.y;
			if (ss > r.y + r.h - ScrollerSize())
				ss = r.y + r.h - ScrollerSize();
	
			if (mTabImage)
				mTabImage->Render(scr, r.x, ss);
		}
	}
	else
	{
		if (mImage)
			mImage->RenderHorizontalEdge(scr, rect(0, 0, r.h, r.h), r);
			
		if (GetMax() > 0)
		{
			ss = r.x + ScrollerTop();
			if (ss < r.x) ss = r.x;
			if (ss > r.x + r.w - ScrollerSize())
				ss = r.x + r.w - ScrollerSize();
	
			if (mTabImage)
				mTabImage->Render(scr, ss, r.y);
		}
	}

	Widget::Render(); //draws buttons
}

void Scrollbar::Event(SDL_Event* event)
{
	switch (event->type)
	{
		case SDL_MOUSEMOTION: {
			if (mScrollPress >= 0 && GetMax() > 0 && HasMouseFocus())
			{
				int p;
				if (mOrientation == VERTICAL)
			    	p = event->motion.y - mScrollPress - mPosition.w;
			    else
					p = event->motion.x - mScrollPress - mPosition.h;

				double tmp = ( (double)p / (double)(ScrollArea() - ScrollerSize()) ) * (double)mMax;
			    if (tmp < 0) tmp = 0;
				SetValue( (uShort)tmp );
			}
		} break;
		case SDL_MOUSEBUTTONDOWN: {
			rect r = GetScreenPosition();
			rect r2;
			if (GetMax() > 0)
			{
				switch (event->button.button)
				{
					case MOUSE_BUTTON_LEFT:
						if (mOrientation == VERTICAL)
						{
							r2.x = r.x;
							r2.y = r.y + ScrollerTop();
							r2.w = r.w;
							r2.h = ScrollerSize();
	
							if (event->button.y < r2.y)
								ScrollUp();
							else if (event->button.y > r2.y + r2.h)
								ScrollDown();
							else
							{
								mScrollPress = event->button.y - ScrollerTop() - r.w;
								if (mValueUp && mValueDown)
									mScrollPress += mValueDown->Height();
							}
						}
						else
						{
							r2.x = r.x + ScrollerTop();
							r2.y = r.y;
							r2.w = ScrollerSize();
							r2.h = r.h;
							if(event->button.x < r2.x)
								ScrollUp();
							else if(event->button.x > r2.x + r2.w)
								ScrollDown();
							else
							{
								mScrollPress = event->button.x - ScrollerTop() - r.h;
								if (mValueUp && mValueDown)
									mScrollPress += mValueDown->Width();
							}
						}
						break;
					case MOUSE_BUTTON_WHEELUP:
						ScrollUp();
						break;
					case MOUSE_BUTTON_WHEELDOWN:
						ScrollDown();
						break;
					default: break;
				}
			} // if GetMax() > 0
		} break;
		case SDL_MOUSEBUTTONUP: {
			mScrollPress = -1;
		} break;
		default: break;
	}
	Widget::Event(event);
}

void Scrollbar::ScrollUp(uShort amount)
{
	if (amount == 0) amount = mBigScroll;
	if (mScrollerPos - amount < 0)
		SetValue(0);
	else
  		SetValue( mScrollerPos - amount );
}

void Scrollbar::ScrollDown(uShort amount)
{
	if (amount == 0) amount = mBigScroll;

  	SetValue( mScrollerPos + amount );
}

void Scrollbar::SetValue(int val)
{
	if (val > mMax)
	  	val = mMax;
	if (val < 0)
		val = 0;

	mScrollerPos = val;

	if (onValueChangeCallback)
		onValueChangeCallback(this);

	if (mShowValue)
		mHoverText = its(val);
		
	FlagRender();
}

uShort Scrollbar::ScrollArea()
{
	uShort s;
  	if (mOrientation == VERTICAL)
  	{
  		s = mPosition.h;
  		if (mValueUp && mValueDown)
  			s -= (mValueUp->Height() + mValueDown->Height());
	}
	else
	{
		s = mPosition.w;
		if (mValueUp && mValueDown)
  			s -= (mValueUp->Width() + mValueDown->Width());
	}
	return s;
}

uShort Scrollbar::ScrollerSize() //static size for now~ (I don't like the resizable method, too complex)
{
 // int s = (int)((double)scrollArea() / ( ((double)mMax + 1) / (double)m_pagesize ));
  //int s = (int)((double)getRealPos().getHeight() / ((double)mMax + 1));

 // if (s >= mScrollMinH) return s;

  	if (mTabImage)
  	{
		if (mOrientation == VERTICAL)
		{
			return mTabImage->Width();
		}
		else
		{
			return mTabImage->Height();
		}
	}

	return 0;
}

uShort Scrollbar::ScrollerTop()
{
	double tmp = (double)mScrollerPos * (((double)ScrollArea() - (double)ScrollerSize()) / (double)mMax );

	uShort u = (uShort)tmp;
	if (mOrientation == VERTICAL)
	{
		if (mValueDown)
			u += mValueDown->Height();
	}
	else
	{
		if (mValueDown)
			u += mValueDown->Width();
	}

	return u;
}

void Scrollbar::SetPosition(rect r)
{
	Widget::SetPosition(r);

	if (mValueUp && mValueDown)
	{
		if (mOrientation == VERTICAL)
		{
			mValueUp->SetPosition( rect(0, 0, Width(), mValueUp->Height()) );
			mValueDown->SetPosition( rect(0, Height() - mValueDown->Height(), Width(), mValueDown->Height()) );
		}
		else
		{
			mValueUp->SetPosition( rect(0, 0, mValueUp->Width(), Height()) );
			mValueDown->SetPosition( rect(Width() - mValueDown->Width(), 0, mValueDown->Width(), Height()) );
		}
	}
}

void Scrollbar::SetImageBase(string base)
{
	resman->Unload(mTabImage);
	
	SetImage(base + "_bg.png");
	
	mTabImage = resman->LoadImg(base + "_tab.png");
	mValueDown->SetImage(base + "_down.png");
	mValueUp->SetImage(base + "_up.png");
}

