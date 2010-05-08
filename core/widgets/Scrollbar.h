
#ifndef _W_SCROLLBAR_H_
#define _W_SCROLLBAR_H_

#include "Widget.h"

enum {
	VERTICAL = 0,
	HORIZONTAL
};

class Image;
class Button;
class Scrollbar: public Widget 
{
  public:
	Scrollbar(Widget* wParent, string sId, rect rPosition, 
					byte bOrientation, uShort uMax, uShort uBigScroll, uShort uValue, 
					void (*onValueChange)(Scrollbar*));
	~Scrollbar();

	void Event(SDL_Event* event);
	void Render(); //ran on render loops or .. w/e

	void SetPosition(rect r);

  	//Scrollbar-specific functions
    
	sShort GetValue() { return mScrollerPos; };
	void SetValue(int val);

	sShort GetMax() { return mMax; };
	void SetMax(int val)
	{
		if (val < 1) val = 1; 
		mMax = val; 
		if (mScrollerPos > mMax) mScrollerPos = mMax;
	};

	uShort GetBigScroll() { return mBigScroll; };
	void SetBigScroll(int val) 
	{
		if (val > mMax) val = mMax;	
		if (val < 1) val = 1;
		mBigScroll = val;
	};

	void ScrollUp(uShort amount = 0);
	void ScrollDown(uShort amount = 0);
	
	void SetImageBase(string base);
	
	void (*onValueChangeCallback)(Scrollbar*);

	bool mShowValue; //draw the value or not~
	byte mOrientation;

	Image* mTabImage;
	uShort ScrollerSize();
	
	Button* mValueUp;
	Button* mValueDown;
	
  protected:

	uShort mMax; //max value 
	
	uShort mScrollerPos; //current value of the scroll
	sShort mScrollPress; //where on the scroller is pressed, -1 = it's not
	uShort mBigScroll; //how much to scroll while clicked on other area
	
	
	uShort ScrollArea();
	
	uShort ScrollerTop();
};

//helper function
uShort getScrollbarValue(Widget* parent, string id);

void callback_scrollbarButtonDown(Button* b);
void callback_scrollbarButtonUp(Button* b);

#endif //_W_SCROLLBAR_H_
