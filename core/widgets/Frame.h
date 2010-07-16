
#ifndef _FRAME_H_
#define _FRAME_H_

#include "Widget.h"

class Button;
class Label;
class Frame : public Widget 
{
  public:
	Frame(Widget* wParent, string sId, rect rPosition, string sCaption = "", 
			bool bMoveable = false, bool bSizeable = false, bool bCloseButton = false, 
			bool bBackground = false);
	~Frame();

	virtual void Render(); 
	virtual void Event(SDL_Event* event); 

	virtual void SetPosition(rect r);

	virtual void ResizeChildren(); //called after resizing event is finished

	void SetSizeable(bool val);

	void SetCaption(string caption);

	bool mMoveable;
	bool mSizeable; 
	bool mBoxRender; //will mImage be rendered as a box, or a full image
	
	Image* mSizer;
	Button* mClose;
	Label* mCaption;

	color mResizingBorderColor;
  protected:
	point2d mDrag;
	bool mResizing;
};

#endif //_FRAME_H_
