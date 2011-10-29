
/*
 * Copyright (c) 2011 Chase McManning
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */


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

	color mSavedGuiBaseColor;
  protected:
	point2d mDrag;
	bool mResizing;
	
	void _readjustBaseColor();
};

#endif //_FRAME_H_
