
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


#ifndef _RIGHTCLICKMENU_H_
#define _RIGHTCLICKMENU_H_

#include "Widget.h"

class RightClickMenu : public Widget
{
  public:
	
	struct sCallback 
	{
		void (*func)(RightClickMenu*, void*);
		void* userdata;
	};
		
	RightClickMenu();
	~RightClickMenu();

	void Render();

	void AddOption(string text, void (*callback)(RightClickMenu*, void*), void* userdata = NULL);
	
	std::vector<sCallback> mCallbacks;
	void (*onCloseCallback)(RightClickMenu*);
	
	bool mFirstEvent;
};

#endif //_RIGHTCLICKMENU_H_
