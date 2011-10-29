
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


#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "Common.h"
#include "Image.h"
#include "RectManager.h"

#define SCREEN_WIDTH 800 //640
#define SCREEN_HEIGHT 600 //480

/**	
	Main application screen, where most of the rendering is done. 
*/
class Screen : public Image
{
  public:
	void Resize(uShort w, uShort h);
	
	void PreRender();
	void PostRender();
	
	/**	Adds a rect clip to the list of rects that can be drawn to during 
		the next render phase
	*/
	void AddRect(rect r);
	
	void Update() { mNeedUpdate = true; };
	bool NeedsUpdate() const { return mNeedUpdate; };
	
	/**
		Checks if the input rect intersects any of the drawable clips in RectManager.
		Callable only during the Render phase (ie: after PreRender() & before PostRender())
		
		@return true if the specific rect intersects any of the clips in RectManager
	*/
	bool IsRectDrawable(rect r);
	
	static Screen* Instance();
	static void Destroy();

	bool mNoDraw;
	bool mDrawOptimizedRects;

	enum optimizationMethod
	{
		NO_OPTIMIZATION = 1,
		LAZY_OPTIMIZATION,
		FULL_OPTIMIZATION	
	};
	
	optimizationMethod mOptimizationMethod;

  private:
  	Screen();
	~Screen();

	bool mNeedUpdate;
	SDL_Rect mLazyRect;
};

extern Screen* g_screen;
extern RectManager g_RectMan;

//Define SDL Flags for our screen. Will require Resize() in order take effect.	
void SetScreenFlags(Uint32 flags);

#endif // _SCREEN_H_
