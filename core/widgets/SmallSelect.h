
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


/*
	idk what's a good name for this.. Basically, two arrow buttons.
	Has a list of items (strings). ++ the visible one when the 
	right button is pressed, -- when the left is pressed. Loop. 
	Very simple but it'll be a compressed method of checkbox groups
	and all that jazz. 
*/
#ifndef _SMALLSELECT_H_
#define _SMALLSELECT_H_

#include "Widget.h"

#define SMALLSELECT_BUFFER 3

class Button;
class SmallSelect: public Widget 
{
  public:
	SmallSelect();
	SmallSelect(Widget* wParent, string sId, rect rPosition,
			void (*cbOnChange)(SmallSelect*));
	~SmallSelect();

	void Render();
	
	void Event(SDL_Event* event);
	
	void SetPosition(rect r);

	void AddItem(string s) { mItems.push_back(s); };
	
	string GetSelectedItem() const;
	
	void (*onChangeCallback)(SmallSelect*);
	
	vString mItems; //options and whatever
	uShort mSelectedIndex; 
	
	Button* mLeft;
	Button* mRight;
	
	Image* mBackgroundImage;
};


#endif //_SMALLSELECT_H_
