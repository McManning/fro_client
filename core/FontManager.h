
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


#ifndef _FONTMANAGER_H_
#define _FONTMANAGER_H_

#include "Common.h"
#include "SDL/SDL_ttf.h"

enum
{
	FONTRENDER_SOLID = 0,
	FONTRENDER_BLENDED = 1,
};

class Image;
class Font 
{
  public:
	Font();
	~Font();

	SDL_Surface* RenderToSDL(const char* text, color rgb);
	
	void Render(Image* dst, int x, int y, string text, color c, int width = 0);
	
	void CharacterWrapMessage(vString& v, string& msg, int maxWidth);	
	void GetTextSize(string text, int* w, int* h);
	
	int GetWidth(string text, bool ignoreCodes = false);
	int GetHeight(string text = "", int width = 0);
	int GetLineSkip();
	
	/*	Returns a rect representation of the text size. If maxWidth > 0, wrapping will occur */
	rect GetTextRect(string text, bool ignoreCodes = false, int maxWidth = 0);
	
	
	/*	Activates the (slower) alpha blend mode, that allows this image to 
		do real RGBA->RGBA blending (does not use SDL's quicker, lazier method
		of using destinations alpha). 
		
		Do NOT activate alpha blending on a surface that redraws itself every frame!
	*/
	bool IsAlphaBlending() const { return mUseBlitOverride; };
	bool UseAlphaBlending(bool b) { mUseBlitOverride = b; };
	
	TTF_Font* data;
	int size;
	int style;
	string filename;
	byte mType;
	bool mUseBlitOverride;
  private:
	void _renderLine(Image* dst, int x, int y, string& text, color c); 
};

//Single font manager that loads and maintains all font instances
class FontManager 
{
  public:
	FontManager();
	~FontManager();
	
	bool Initialize();
	
	/* 	Get a loaded font. If it isn't loaded, it'll load it from the directory. If the
		file doesn't exist, it'll return NULL.
	*/
	Font* Get(string filename = "", int size = 0, int style = 0);
	
	//Returns true on success
	bool Unload(string filename, int size, int style);
	
	//Returns both the font and optionally the index position (Used internally)
	Font* Find(string filename, int size, int style = 0, int* index = NULL);
	
  private:
	std::vector<Font*> loadedFonts;
};

extern FontManager* fonts;

#endif //_FONTMANAGER_H_
