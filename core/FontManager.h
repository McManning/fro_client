
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
	
	void Render(Image* dst, sShort x, sShort y, string text, color c, uShort width = 0);
	
	//Wrap message based on individual characters and not spaces
	void CharacterWrapMessage(vString& v, string msg, uShort width);
		
	void GetTextSize(string text, uShort* w, uShort* h);
	
	uShort GetWidth(string text);
	uShort GetHeight(string text = "", uShort width = 0);
	uShort GetLineSkip();
	
	TTF_Font* data;
	uShort size;
	uShort style;
	string filename;
	byte mType;
  private:
	void _renderLine(Image* dst, sShort x, sShort y, string& text, color c); 
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
	Font* Get(string filename = "", uShort size = 0, uShort style = 0);
	
	//Returns true on success
	bool Unload(string filename, uShort size, uShort style);
	
	//Returns both the font and optionally the index position (Used internally)
	Font* Find(string filename, uShort size, uShort style = 0, uShort* index = NULL);
	
  private:
	std::vector<Font*> loadedFonts;
};

extern FontManager* fonts;

#endif //_FONTMANAGER_H_
