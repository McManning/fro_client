
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


#ifndef _CHECKBOX_H_
#define _CHECKBOX_H_

#include "Widget.h"

//For simple on/off checkboxes
#define UNCHECKED 0
#define CHECKED 1

const int CHECKBOX_SIZE = 16;

class Font;
class Image;
/*
	Has a simple on/off state, along with disabled version and mouseover version. 
	Basically like a button, but ... state button with caption to the right. 
	TODO: Checkbox grouping
*/
class Checkbox: public Widget 
{
  public:
	Checkbox(Widget* wParent, string sId, rect rPosition, string sCaption, byte bGroup);
	~Checkbox();

	void Render();
	void Event(SDL_Event* event);
	
	/* TODO: Do I need to FlagRender on EventMouseMotion since it 
		changes the image? (Same question for Button)  */
	
	string GetCaption() { return mCaption; };
	
	void SetCaption(string text);
	
	void SetState(byte state);
	void SetStateCount(byte count);
	
	byte GetStateCount() { return mStateCount; };
	byte GetState() { return mState; };
	
	byte mGroup; //group number
	
  protected:
	string mCaption; 
	
	byte mState; //Which state we're currently in
	byte mLastState;
	byte mStateCount; //how many states the checkbox has. Default of 2
};

//Helper function
uShort getCheckboxState(Widget* parent, string id);

#endif //_CHECKBOX_H_
