
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
	
	TTF_Font* data;
	int size;
	int style;
	string filename;
	byte mType;
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
