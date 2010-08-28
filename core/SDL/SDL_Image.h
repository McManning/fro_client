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

/* A simple library to load images of various formats as SDL surfaces 
	Also PNG saving herp derp
*/

#ifndef _SDL_IMAGE_H
#define _SDL_IMAGE_H

#include <SDL/SDL.h>
#include <SDL/SDL_version.h>
#include <SDL/begin_code.h>

//Byte order table
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define RMASK 0xff000000
    #define GMASK 0x00ff0000
    #define BMASK 0x0000ff00
    #define AMASK 0x000000ff
#else
    #define RMASK 0x000000ff
    #define GMASK 0x0000ff00
    #define BMASK 0x00ff0000
    #define AMASK 0xff000000
#endif

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* Printable format: "%d.%d.%d", MAJOR, MINOR, PATCHLEVEL
*/
#define SDL_IMAGE_MAJOR_VERSION	1
#define SDL_IMAGE_MINOR_VERSION	3
#define SDL_IMAGE_PATCHLEVEL	6

#define IMG_FORMAT_UNKNOWN 0
#define IMG_FORMAT_GIF 1
#define IMG_FORMAT_PNG 2
#define IMG_FORMAT_JPG 3
#define IMG_FORMAT_BMP 4
#define IMG_FORMAT_MNG 5

//Custom container for SDL_Surfaces which allows animated properties
typedef struct
{
	SDL_Surface* surf;
	unsigned int delay;
	char* key;
}
SDL_Frame;

typedef struct
{
	SDL_Frame* frames;
	int count;
	int format;
}
IMG_File;

#ifdef __DEBUG__
#	define SDL_IMAGE_DEBUG
#endif

#ifdef SDL_IMAGE_DEBUG
#	define dbgout printf
#else
#	define dbgout 
#endif

/* This macro can be used to fill a version structure with the compile-time
 * version of the SDL_image library.
 */
#define SDL_IMAGE_VERSION(X)						\
{									\
	(X)->major = SDL_IMAGE_MAJOR_VERSION;				\
	(X)->minor = SDL_IMAGE_MINOR_VERSION;				\
	(X)->patch = SDL_IMAGE_PATCHLEVEL;				\
}

/* This function gets the version of the dynamically linked SDL_image library.
   it should NOT be used to fill a version structure, instead you should
   use the SDL_IMAGE_VERSION() macro.
 */
extern DECLSPEC const SDL_version * SDLCALL IMG_Linked_Version(void);

/* Load an image from an SDL data source.
   The 'type' may be one of: "BMP", "GIF", "PNG", etc.

   If the image format supports a transparent pixel, SDL will set the
   colorkey for the surface.  You can enable RLE acceleration on the
   surface afterwards by calling:
	SDL_SetColorKey(image, SDL_RLEACCEL, image->format->colorkey);
 */
extern DECLSPEC int SDLCALL IMG_LoadTyped_RW(SDL_RWops *src, int freesrc, char *type, IMG_File* dst);



/* Convenience functions */
extern DECLSPEC int SDLCALL IMG_Load(const char *file, IMG_File* dst);
extern DECLSPEC int SDLCALL IMG_Load_RW(SDL_RWops *src, int freesrc, IMG_File* dst);
extern DECLSPEC int SDLCALL IMG_Load_Memory(const void* mem, int len, IMG_File* dst);

/* Invert the alpha of a surface for use with OpenGL
   This function is now a no-op, and only provided for backwards compatibility.
*/
extern DECLSPEC int SDLCALL IMG_InvertAlpha(int on);

/* Functions to detect a file type, given a seekable source */
extern DECLSPEC int SDLCALL IMG_isBMP(SDL_RWops *src);
extern DECLSPEC int SDLCALL IMG_isJPG(SDL_RWops *src);
extern DECLSPEC int SDLCALL IMG_isGIF(SDL_RWops *src);
extern DECLSPEC int SDLCALL IMG_isPNG(SDL_RWops *src);
extern DECLSPEC int SDLCALL IMG_isMNG(SDL_RWops *src);

/* Individual loading functions */
extern DECLSPEC int SDLCALL IMG_LoadBMP_RW(SDL_RWops *src, IMG_File* dst);
extern DECLSPEC int SDLCALL IMG_LoadJPG_RW(SDL_RWops *src, IMG_File* dst);
extern DECLSPEC int SDLCALL IMG_LoadGIF_RW(SDL_RWops *src, IMG_File* dst);
extern DECLSPEC int SDLCALL IMG_LoadPNG_RW(SDL_RWops *src, IMG_File* dst);
extern DECLSPEC int SDLCALL IMG_LoadMNG_RW(SDL_RWops *src, IMG_File* dst);

/* Writes the surf to a png file designated by filename
	Does NOT support alpha channels yet. Will convert the surface to a 
	24 bit RGB before writing. Returns 0 on success, -1 otherwise. 
*/
extern DECLSPEC int SDLCALL IMG_SavePNG(const char* filename, SDL_Surface* surf);

extern DECLSPEC int SDLCALL IMG_SurfaceToFrameset(SDL_Surface* surface, IMG_File* dst);
SDL_Frame* IMG_MallocFrames(int count);



/* We'll use SDL for reporting errors */
#define IMG_SetError	SDL_SetError
#define IMG_GetError	SDL_GetError

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include <SDL/close_code.h>

#endif /* _SDL_IMAGE_H */
