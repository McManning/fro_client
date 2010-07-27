
#include "FontManager.h"
#include "ResourceManager.h"
#include "io/XmlFile.h"
#include "SDL/SDL_gfxBlitFunc.h"

FontManager* fonts;

Font::Font() 
{
	data = NULL;
	mType = FONTRENDER_BLENDED;
	mUseBlitOverride = false;
}

Font::~Font() 
{
	if (data)
		TTF_CloseFont(data);
}

/*	LOGIC

	for all words
		if word width > width
			for each character
				if width of characters all > max width
					split word @ char - 1
					go to next word (could be the split from this word)
	
	For each word, determine width of s + space + word
		If width > max width 
			add s to v, clear s, then put word into s
*/
void Font::CharacterWrapMessage(vString& v, string& msg, int maxWidth)
{
	string s, sw;
	int i, c;
	bool quit;
	vString words;

	// split into words
	explode(&words, &msg, " ");

	// Bunch of spaces.  We don't care.
	if (words.empty())
	{
		v.push_back(" ");
		return;
	}
	
	/*	Find words wider than the max width, and split them up into multiple words */
	i = 0;
	while (i < words.size())
	{	
		if (words.at(i) != "\\n")
		{
			// split at \n
			c = words.at(i).find("\\n", 0);
			if (c != string::npos)
			{
				// We found enough characters to fill one line, split and quit
				words.insert(words.begin() + i + 1, words.at(i).substr(c+2));
				words.insert(words.begin() + i + 1, "\\n");
				words.at(i) = words.at(i).substr(0, c);
			}
			
			if (GetWidth(words.at(i), true) > maxWidth)
			{
				// Word is too long, try to find a place to split it
				quit = false;
				for (c = 0; c < words.at(i).length() && !quit; ++c)
				{
					if (GetWidth(words.at(i).substr(0, c), true) > maxWidth)
					{
						// We found enough characters to fill one line, split and quit
						words.insert(words.begin() + i + 1, words.at(i).substr(c-1));
						words.at(i) = words.at(i).substr(0, c-1);
						quit = true;
					}
				}
			}
		}
		++i;
	}

	/*	Go through all words and try to fit them together on lines no longer than maxWidth */
	for (i = 0; i < words.size(); ++i)
	{
		if (words.at(i) == "\\n")
		{
			// If there's a newline marker, dump what we got and start a new line
			if (!s.empty())
			{
				v.push_back(s);
				s.clear();
			}
			
			v.push_back("");
		}
		else
		{
			if (s.empty())
				sw = words.at(i);
			else
				sw = ' ' + words.at(i);
	
			if (GetWidth(s + sw, true) > maxWidth)
			{
				// Too long, next word won't fit, split
				v.push_back(s);
				s = words.at(i);
			}
			else // add next word
			{
				s += sw;
			}
		}
	}
	
	// Add the remains of the string
	if (!s.empty())
		v.push_back(s);
}

rect Font::GetTextRect(string text, bool ignoreCodes, int maxWidth)
{
	rect r(0,0,maxWidth,0);
	vString v;

	if (ignoreCodes)
		text = stripCodes(text);
			
	if (maxWidth > 0)
	{
		CharacterWrapMessage(v, text, maxWidth);

	 	if (v.size() < 2)
	 	{
			r.w = GetWidth(text);
			r.h = GetHeight();
		}
		else
		{
			r.w = 0;
			int w;
			//find the longest line, and mark that as our width
			for (int i = 0; i < v.size(); ++i)
			{
				w = GetWidth(v.at(i), true);
				if (w > r.w)
					r.w = w;
			}
			
			r.h = v.size() * GetHeight();
		}
	}
	else
	{
		r.w = GetWidth(text);
		r.h = GetHeight();
	}
	
	return r;
}

SDL_Surface* Font::RenderToSDL(const char* text, color rgb)
{
	SDL_Color c = { rgb.r, rgb.g, rgb.b, rgb.a };
	SDL_Surface* s = NULL;

	if (mType == FONTRENDER_BLENDED)
		s = TTF_RenderText_Blended(data, text, c);
	else
		s = TTF_RenderText_Solid(data, text, c);
	
	return s;
}

void Font::_renderLine(Image* dst, int x, int y, string& text, color c)
{
	vString v;
	int i;
	SDL_Rect rDst;
	SDL_Surface* surf;
	SDL_Surface* dstsurf = dst->Surface();

	if (!dstsurf) //nowhere to render to, so no reason to do these calculations
		return;

	explode(&v, &text, "\\c"); //break up \c's

	/*	render each section. Using first 3 characters for a color code.
		Offset the X on each draw.
	*/
	rDst.x = x;
	rDst.y = y;
	for (i = 0; i < v.size(); i++)
	{
		//if we don't have a \c prefix for the message, just render default color without skip.
		if (i == 0 && text.find("\\c", 0) != 0)
			surf = RenderToSDL(v.at(i).c_str(), c);
		else if (v.at(i).length() > 3) //must have at least RGB starting it
			surf = RenderToSDL(v.at(i).substr(3).c_str(), slashCtoColor(v.at(i).substr(0, 3)));	
		else
			surf = NULL;
		
		if (surf)
		{
			rDst.w = surf->w;
			rDst.h = surf->h;
			
			if (mUseBlitOverride)
			{
				if ( SDL_gfxBlitRGBA(surf, &surf->clip_rect, dstsurf, &rDst) < 0 )
				{
			        WARNING(SDL_GetError());
				}
			}
			else
			{
				if ( SDL_BlitSurface(surf, &surf->clip_rect, dstsurf, &rDst) < 0 )
				{
					WARNING(SDL_GetError());
				}
			}
			SDL_FreeSurface(surf);
			x += rDst.w; //offset the X for the next section
			
			//SDL_BlitSurface will fuck with our rect after clipping, fix it.
			rDst.x = x;
			rDst.y = y; 
		}
	}

}

void Font::Render(Image* dst, int x, int y, string text, color c, int width)
{
	if (width > 0) //wrap it 
	{
		vString v;
		CharacterWrapMessage(v, text, width);
		int index;
		
		for (int i = 0; i < v.size(); ++i)
		{
			_renderLine(dst, x, y, v.at(i), c);

			index = v.at(i).rfind("\\c");
			if (index != string::npos && v.at(i).length() > index + 5)
				c = slashCtoColor(v.at(i).substr(index, 5));

			y += GetHeight();
		}
	} 
	else
	{
		_renderLine(dst, x, y, text, c);
	}
	
}

void Font::GetTextSize(string text, int* w, int* h) 
{
	int _w, _h;
	//if(getUTF8()) TTF_SizeUTF8(data, text, &_w, &_h);
	TTF_SizeText(data, text.c_str(), &_w, &_h);
	*w = _w;
	*h = _h;
}

int Font::GetWidth(string text, bool ignoreCodes)
{
	int w, h;
	if (ignoreCodes)
		GetTextSize(stripCodes(text), &w, &h);
	else
		GetTextSize(text, &w, &h);
		
	return w;
}

int Font::GetHeight(string text, int width)
{
	int h = TTF_FontHeight(data);
	
	if (width > 0)
	{
		vString v;
		CharacterWrapMessage(v, text, width);
		h *= v.size();
	}
	
	return h;
}

int Font::GetLineSkip()
{
	return TTF_FontLineSkip(data);
}

FontManager::FontManager() 
{

}

FontManager::~FontManager() 
{
	//Unload all loaded fonts
	Font* f;
	for (int i = 0; i < loadedFonts.size(); i++)
	{
		f = loadedFonts.at(i);
		PRINT("Erasing Font: " + f->filename + " " + pts(f));
		SAFEDELETE(f);
	}
	
	PRINT("TTF_Quit");
	TTF_Quit();
}

//Load up default standard fonts, intialize ttf and shit, etc
bool FontManager::Initialize() 
{
	if(TTF_Init() < 0) {
        WARNING("SDL_TTF: " + string(SDL_GetError()));
    	return false;
	}

	return true;
}

Font* FontManager::Get(string filename, int size, int style) 
{
	if (filename.empty())
		filename = config.GetParamString("font", "face");	
	if (size == 0)
		size = config.GetParamInt("font", "size");

	Font* f = Find(filename, size, style);
	if (f) return f; //Already loaded, Hooray the works done!

	//Try TTF
	TTF_Font* ttf = TTF_OpenFont(filename.c_str(), size);
	if (ttf)
	{
		f = new Font();
		f->data = ttf;
		f->size = size;
		f->filename = filename;
		f->style = style;
		
		//#define TTF_STYLE_NORMAL	0x00
		//#define TTF_STYLE_BOLD		0x01
		//#define TTF_STYLE_ITALIC	0x02
		//#define TTF_STYLE_UNDERLINE	0x04
		if (style != 0)
			TTF_SetFontStyle(ttf, style);
		
		loadedFonts.push_back(f);
		return f;
	}
	else WARNING(filename + ": " + SDL_GetError());

	return NULL;
}

//Returns true on success
bool FontManager::Unload(string filename, int size, int style) 
{
	int index;
	Font* f = Find(filename, size, style, &index);
	
	if (f)
	{
		SAFEDELETE(f);
		loadedFonts.erase(loadedFonts.begin() + index);
	}
	
	return (f != NULL);
}

//Returns both the font and optionally the index position (Used internally)
Font* FontManager::Find(string filename, int size, int style, int* index) {
	Font* f;
	
	for (int i = 0; i < loadedFonts.size(); i++)
	{
		f = loadedFonts.at(i);
		if (f->filename == filename && f->size == size && f->style == style)
		{
			if (index)
				*index = i;
				
			return f;
		}
	}
	
	return NULL;
}
