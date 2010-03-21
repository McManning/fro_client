
#include "FontManager.h"
#include "ResourceManager.h"
#include "io/XmlFile.h"

FontManager* fonts;

Font::Font() 
{
	data = NULL;
	mType = FONTRENDER_BLENDED;
}

Font::~Font() 
{
	if (data)
		TTF_CloseFont(data);
}


void Font::CharacterWrapMessage(vString& v, string msg, uShort width)
{
	uShort i = 0, w = 0;

	while (true)
	{
		if (i >= msg.length() || msg.empty()) break;
		
		w = GetWidth( msg.substr(0, i).c_str() );
		if ( w + 5 >= width ) //if it overflows, add then go to next line
		{
			v.push_back( msg.substr(0, i) );
			msg.erase(0, i);
			i = 0;
		}
		else  //same line keep going
		{
			i++;
		}
	}
	//add rest (should be less than a max line length)
	v.push_back( msg.substr(0, i) );
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

void Font::_renderLine(Image* dst, sShort x, sShort y, string& text, color c)
{
	vString v;
	int i;
	SDL_Rect rDst;
	SDL_Surface* surf;
	SDL_Surface* dstsurf = dst->Surface();

	if (!dstsurf) //nowhere to render to, so no reason to do these calculations
		return;
	
	replace(&text, "\\\\c", "\x01"); //use some unwriteable character as a marker
	explode(&v, &text, "\\c"); //break up \c's

	/*	render each section. Using first 3 characters for a color code.
		Offset the X on each draw.
	*/
	rDst.x = x;
	rDst.y = y;
	for (i = 0; i < v.size(); i++)
	{
		replace(&v.at(i), "\x01", "\\c"); //replace marker with a renderable \c
	
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
			if ( SDL_BlitSurface(surf, &surf->clip_rect, dstsurf, &rDst) < 0 )
			{
				WARNING(SDL_GetError());
			}
			SDL_FreeSurface(surf);
			x += rDst.w; //offset the X for the next section
			
			//SDL_BlitSurface will fuck with our rect after clipping, fix it.
			rDst.x = x;
			rDst.y = y; 
		}
	}

}

void Font::Render(Image* dst, sShort x, sShort y, string text, color c, uShort width)
{
	if (width > 0) //wrap it 
	{
		vString v;
		CharacterWrapMessage(v, stripCodes(text), width);
		int index;
		
		for (int i = 0; i < v.size(); i++)
		{
			_renderLine(dst, x, y, v.at(i), c);
			index = v.at(i).find_last_of("\\c");
			
			//if there was a color change, and the change wasn't prefixed with \\c to escape, change resulting color
		/*	if (index != string::npos && (index < 1 || v.at(i).at(index-1) != '\\')
				&& index + 5 < v.at(i).length() ) 
			{
				c = slashCtoColor(v.at(i).substr(index, 5));
			}*/
			
			y += GetHeight();
		}
	} 
	else
	{
		_renderLine(dst, x, y, text, c);
	}
	
}

void Font::GetTextSize(string text, uShort* w, uShort* h) 
{
	int _w, _h;
	//if(getUTF8()) TTF_SizeUTF8(data, text, &_w, &_h);
	TTF_SizeText(data, text.c_str(), &_w, &_h);
	*w = _w;
	*h = _h;
}

uShort Font::GetWidth(string text)
{
	uShort w, h;
	GetTextSize(text, &w, &h);
	return w;
}

uShort Font::GetHeight(string text, uShort width)
{
	uShort h = TTF_FontHeight(data);
	
	if (width > 0)
	{
		vString v;
		CharacterWrapMessage(v, text, width);
		h *= v.size();
	}
	
	return h;
}

uShort Font::GetLineSkip()
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
	for (uShort i = 0; i < loadedFonts.size(); i++)
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

Font* FontManager::Get(string filename, uShort size, uShort style) 
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
bool FontManager::Unload(string filename, uShort size, uShort style) 
{
	uShort index;
	Font* f = Find(filename, size, style, &index);
	
	if (f)
	{
		SAFEDELETE(f);
		loadedFonts.erase(loadedFonts.begin() + index);
	}
	
	return (f != NULL);
}

//Returns both the font and optionally the index position (Used internally)
Font* FontManager::Find(string filename, uShort size, uShort style, uShort* index) {
	Font* f;
	
	for (uShort i = 0; i < loadedFonts.size(); i++)
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
