/*
    SDL_image:  An example image loading library for use with SDL
    Copyright (C) 1997-2006 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/* A simple library to load images of various formats as SDL surfaces */

#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   undef WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "SDL_Image.h"

#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))

/* Table of image detection and loading functions */
static struct {
	const char *type;
	int (SDLCALL *is)(SDL_RWops *src);
	int (SDLCALL *load)(SDL_RWops *src, IMG_File* dst);
} supported[] = {
	/* keep magicless formats first */
	{ "GIF", IMG_isGIF, IMG_LoadGIF_RW },
	{ "PNG", IMG_isPNG, IMG_LoadPNG_RW },
	{ "JPG", IMG_isJPG, IMG_LoadJPG_RW },
	{ "BMP", IMG_isBMP, IMG_LoadBMP_RW },
	{ "MNG", IMG_isMNG, IMG_LoadMNG_RW }
/*	{ "LBM", IMG_isLBM, IMG_LoadLBM_RW },
	{ "PCX", IMG_isPCX, IMG_LoadPCX_RW },

	{ "TGA", NULL,      IMG_LoadTGA_RW },
	{ "PNM", IMG_isPNM, IMG_LoadPNM_RW }, */ /* P[BGP]M share code */
/*	{ "TIF", IMG_isTIF, IMG_LoadTIF_RW },
	{ "XCF", IMG_isXCF, IMG_LoadXCF_RW },
	{ "XPM", IMG_isXPM, IMG_LoadXPM_RW },
	{ "XV",  IMG_isXV,  IMG_LoadXV_RW  } */
};

const SDL_version *IMG_Linked_Version(void)
{
	static SDL_version linked_version;
	SDL_IMAGE_VERSION(&linked_version);
	return(&linked_version);
}

/* Load an image from a file */
int IMG_Load(const char *file, IMG_File* dst)
{
    SDL_RWops *src = SDL_RWFromFile(file, "rb");

    char *ext = strrchr(file, '.');
    if(ext) {
        ext++;
    }
    if(!src) {
        /* The error message has been set in SDL_RWFromFile */
        return 0;
    }
    int result = IMG_LoadTyped_RW(src, 1, ext, dst);

	dbgout("IMG_Load Returning\n");

    return result;
}

int IMG_Load_Memory(const void* mem, int len, IMG_File* dst)
{
	SDL_RWops* rw = SDL_RWFromConstMem(mem, len);
	if (!rw)
	{
		IMG_SetError("Could Not Create RWops");
		return 0;
	}

	return IMG_LoadTyped_RW(rw, 1, NULL, dst);
}

/* Load an image from an SDL datasource (for compatibility) */
int IMG_Load_RW(SDL_RWops *src, int freesrc, IMG_File* dst)
{
    return IMG_LoadTyped_RW(src, freesrc, NULL, dst);
}

/* Portable case-insensitive string compare function */
static int IMG_string_equals(const char *str1, const char *str2)
{
	while ( *str1 && *str2 ) {
		if ( toupper((unsigned char)*str1) !=
		     toupper((unsigned char)*str2) )
			break;
		++str1;
		++str2;
	}
	return (!*str1 && !*str2);
}

/* Load an image from an SDL datasource, optionally specifying the type */
int IMG_LoadTyped_RW(SDL_RWops *src, int freesrc, char *type, IMG_File* dst)
{
	int i;
	int result;

	/* Make sure there is something to do.. */
	if ( src == NULL )
	{
		IMG_SetError("Passed a NULL data source");
		return 0;
	}

	/* See whether or not this data source can handle seeking */
	if ( SDL_RWseek(src, 0, SEEK_CUR) < 0 )
	{
		IMG_SetError("Can't seek in this data source");
		if(freesrc)
			SDL_RWclose(src);
		return 0;
	}

	/* Detect the type of image being loaded */
	for ( i=0; i < ARRAYSIZE(supported); ++i )
	{
		if(supported[i].is)
		{
			if(!supported[i].is(src))
				continue;
		}
		else
		{
			/* magicless format */
			if (!type || !IMG_string_equals(type, supported[i].type))
				continue;
		}

		dbgout("IMGLIB: Loading image as %s\n", supported[i].type);

		result = supported[i].load(src, dst);
		if (freesrc)
			SDL_RWclose(src);
		return result;
	}

	if ( freesrc )
	{
		SDL_RWclose(src);
	}

	IMG_SetError("Unsupported image format");
	return 0;
}

/* Invert the alpha of a surface for use with OpenGL
   This function is a no-op and only kept for backwards compatibility.
 */
int IMG_InvertAlpha(int on)
{
    return 1;
}

int IMG_SurfaceToFrameset(SDL_Surface* surface, IMG_File* dst)
{
	if (!surface || !dst)
		return 0;

	//build a container for this surface
	dst->frames = (SDL_Frame*)malloc(sizeof(SDL_Frame));
	if (!dst->frames)
		return 0;

	dst->frames[0].key = NULL;
	dst->frames[0].delay = 0;
	dst->frames[0].surf = surface;
	dst->count = 1;
	return 1;
}

SDL_Frame* IMG_MallocFrames(int count)
{
	SDL_Frame* frames = (SDL_Frame*)malloc(sizeof(SDL_Frame) * count);
	return frames;
}

/*
BOOL APIENTRY DllMain (HINSTANCE hInst, DWORD reason, LPVOID reserved) {
  // __hDllInstance_base = hInst;


  return TRUE;
}
*/
