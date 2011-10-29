
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


#ifndef _YESNOPOPUP_H_
#define _YESNOPOPUP_H_

#include "Frame.h"

#define YESNOPOPUP_MAX_WIDTH 400
#define YESNOPOPUP_MIN_WIDTH 150

class YesNoPopup : public Frame 
{
  public:
	YesNoPopup(string id, string title, string msg, bool useMultiline = false);
	~YesNoPopup();
	
	void Render();

	Button* mYes;
	Button* mNo;

	void (*onYesCallback)(YesNoPopup*);
	void (*onNoCallback)(YesNoPopup*);
	void* userdata;
};

#endif //_YESNOPOPUP_H_
