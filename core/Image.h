
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


#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "SDL/SDL_Image.h"
#include "Common.h"

const int MAX_AVATAR_FRAMES = 50;

/* Frameset contains frames, and special properties that pertain to all frames of this set at once. */
typedef struct SDL_Frameset
{
	SDL_Frameset()
	{
		loop = true;
	};
	
	std::vector<SDL_Frame> frames;
	string key; //overall set key
	bool loop;
}
SDL_Frameset;


/*
	Stored in our database of loaded images and referenced 
		multiple times by multiple Image instances.
*/
class SDL_Image
{
  public:
	SDL_Image();
	~SDL_Image();

	bool Load(string file, string pass);

	SDL_Image* Clone(rect clip = rect()) const;
	void PrintInfo() const;
	int CountFrames();
	SDL_Frameset* GetFrameset(string key);
	
	std::vector<SDL_Frameset> framesets;
	
	//Memorized in case we need to reload
	string filename;
	string password;

	int refCount;
	int format; 
	
	enum { //states
		NOIMAGE = 0, //Waiting for something to call Load()
		LOADING, //reading file. 
		LOADED, //CountFrames() should return > 0
		BADIMAGE //invalid image format, or download failed
	};
	int state;
		
  private:
	void _copyFramesets(SDL_Image* dst, rect clip) const;
	void _unloadFramesets();
};

/*	Images are unique instances (Not maintained by the image manager) 
		These maintain their own animation properties.
*/
struct timer;
class Image
{
  public:
	Image();
	~Image();
	
	/*	Clones the properties of this Image into a new instance. 
		If fullClone == true, will create a clone of the SDL_Image also, 
		otherwise it will simply reference the shared SDL_Image.
		Clip defines a clip of the image to clone. If it is not default rect,
		it will create a new instance of SDL_Image
	*/
	Image* Clone(bool fullClone = false, rect clip = rect()) const;
	
	uShort Width() const;
	uShort Height() const;
	
	uShort MaxWidth() const;
	uShort MaxHeight() const;
	
	SDL_Frameset* Frameset() const;
	SDL_Frame* Frame() const;
	SDL_Surface* Surface() const;
	
	SDL_Frameset* GetFrameset(string key) const;
	bool SetFrameset(string key);

	void PushClip(rect clip);
	void PopClip();
	
	rect GetClip() const;
	
	/**
		@return true if the surface can be drawn to. Ie, GetClip() has a nonzero width/height
	*/
	bool IsDrawable() const;
	
	string Filename() const;

	/*	Returns color of the pixel at (x, y). If coordinates are invalid, or there is 
			no surface, will return default color() (Most likely black)
	*/
	color GetPixel(sShort x, sShort y) const;
	
	/* 	Set the pixel at (x, y) to the specified color. 
			Will return false if the coordinates are invalid or there is no surface to render to.
		Parameters:
			x, y: Coordinates of pixel to set.
			rgb: Color to set the pixel to. (NOTE: Behavior undefined for colortable surfaces)
			copy: If set to true, will copy the new color over the old, otherwise will attempt to SLOWLY alpha blend.
	*/
	bool SetPixel(sShort x, sShort y, color rgb, bool copy = false);

	/*	Return true if the pixel @ (x,y) has an alpha of 0, or is the colorkey */
	bool IsPixelTransparent(int x, int y);

//Routines to draw primitives to this Image.
	
	/*	Draws a rectangle of the specified dimensions, dimensions, and color */
	bool DrawRect(rect r, color c, bool filled = true);
	
	/*	Overloaded version of DrawRound, using a rect to define coordinates */
	bool DrawRound(rect r, uShort corner, color rgb);

	/* 	Draws a rectangle with rounded corners (based on a routine from SDL_draw library)
		Parameters:
			x0, y0: Topleft corner coordinates of the circle
			rgb: Fill color
			w, h: Dimensions of the resulting render
			corner: Value of how much the edges should be rounded off. 
				If w == h and corner = w / 2, will draw a circle.
	*/
	bool DrawRound(sShort x0, sShort y0, uShort w, uShort h, uShort corner, color rgb);
	
	/*	Draw a line starting at (x1, y1) and going to (x2, y2)  with the specified thickness in pixels */
	bool DrawLine(sShort x1, sShort y1, sShort x2, sShort y2, color rgb, byte thickness);

//Routines to render this Image to another using various techniques
	
	bool Render(Image* destination, sShort x, sShort y, rect clip = rect());
	bool _renderToSurface(SDL_Surface* dst, sShort x, sShort y, rect& clip);
	bool _renderToScreen(sShort x, sShort y, rect& clip);
	
	// Shorthand rendering routines for various behavior
	bool RenderHorizontalEdge(Image* dst, rect rSrc, rect rDst);
	bool RenderVerticalEdge(Image* dst, rect rSrc, rect rDst);
	bool RenderPattern(Image* dst, rect rSrc, rect rDst);
	bool RenderBox(Image* dst, rect rSrc, rect rDst);

//Routines to manipulate this Image (Warning: directly affects the SDL_Image this Image is referencing!)
	
	bool Rotate(double degree, double zoom, int aa = 0);
	bool Scale(double xzoom, double yzoom, int aa = 0);
	void ConvertToAlphaFormat();
		
	void ColorizeGreyscale(color modifier);
	void ReduceAlpha(byte amount);
	
	/*
		Converts a single frame type SDL_Image to a bunch of frames, moving horizontally (+x) starting @ x, y of clip and using w/h for each frame. 
			Constant delay is set between all frames.
		ie:
		|------|------|------|------|------|------|
		|        |        |        |        |        |        |
		|------|------|------|------|------|------|
	*/
	bool ConvertToHorizontalAnimation(rect clip, uShort delay);

	/*
		Converts single-frame SDL_Image to a multiframed & keyed avatar
		by clipping the image to multiples. If it's already multiframed, it'll set properties for each key
		and verify that the avatar is legal. (Doesn't surpass max dimensions, etc)
		
		Returns false if an error occured. 
	*/	
	bool ConvertToAvatarFormat(uShort w, uShort h, uShort delay, bool loopStand, bool loopSit);
	
//Routines for animation

	/*	Return delay to next change. ULONG_MAX if the current frame is null */
	uLong ForwardCurrentFrameset(bool forceLoop = false);
	
	/*	Return true if we're on the last frame of the current animation */
	bool IsOnLastFrame();

	void Reset();
		
//Misc Routines
	
	void SavePNG(string file) const;

	/*	Activates the (slower) alpha blend mode, that allows this image to 
		do real RGBA->RGBA blending (does not use SDL's quicker, lazier method
		of using destinations alpha). 
		
		Do NOT activate alpha blending on a surface that redraws itself every frame!
	*/
	bool IsAlphaBlending() const { return mUseBlitOverride; };
	bool UseAlphaBlending(bool b) { mUseBlitOverride = b; };

//private:
	
	/* 	mImage can be shared by multiple Image instances, so be wary of modifications.
	 	this->Clone() is suggested if there needs to be changes that would only affect 
	 	one instance of an Image's SDL_Image. (ex: Rotating one renderable map entity 
	 	while keeping the others, who look the same and used the same base SDL_Image, 
	 	as they normally were and not rotated)
	 */
	SDL_Image* mImage;
	
	uShort mFramesetIndex;
	uShort mFrameIndex;

	bool mUseBlitOverride;
	
	std::vector<rect> mClipStack;
	
  private:
	void _checkStatus();
};

#endif //_IMAGE_H_
