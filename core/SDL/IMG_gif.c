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

/* This is a GIF image file loading framework */

#include "SDL_Image.h"
#define LOAD_GIF 1
#ifdef LOAD_GIF

/* See if an image is contained in a data source */
int IMG_isGIF(SDL_RWops *src)
{
	int start;
	int is_GIF;
	char magic[6];

	if ( !src )
		return 0;
	start = SDL_RWtell(src);
	is_GIF = 0;
	if ( SDL_RWread(src, magic, sizeof(magic), 1) ) {
		if ( (strncmp(magic, "GIF", 3) == 0) &&
		     ((memcmp(magic + 3, "87a", 3) == 0) ||
		      (memcmp(magic + 3, "89a", 3) == 0)) ) {
			is_GIF = 1;
		}
	}
	SDL_RWseek(src, start, SEEK_SET);
	return(is_GIF);
}

/* Code from here to end of file has been adapted from XPaint:           */
/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, 1991, 1993 David Koblas.			       | */
/* | Copyright 1996 Torsten Martinsen.				       | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.	       | */
/* +-------------------------------------------------------------------+ */

/* Adapted for use in SDL by Sam Lantinga -- 7/20/98 */
/* Raped into outputting animated gif frames to a single SDL_Surface row by Chase McManning -- 11/14/08
	Note: Will return failure if the gif is larger than maximum SDL_Surface Width */
/* Chase 090627 - Further improved by outputting an array of SDL_Frames, allowing any size of gif
	and maintaining all animation properties */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Changes to work with SDL:

   Include SDL header file
   Use SDL_Surface rather than xpaint Image structure
   Define SDL versions of IMG_SetError(), ImageNewCmap() and ImageSetCmap()
*/

#define ImageNewCmap(w, h, s)	SDL_AllocSurface(SDL_SWSURFACE,w,h,8,0,0,0,0)
#define ImageSetCmap(s, i, R, G, B) do { \
				s->format->palette->colors[i].r = R; \
				s->format->palette->colors[i].g = G; \
				s->format->palette->colors[i].b = B; \
			} while (0)
/* * * * * */



#define	MAXCOLORMAPSIZE		256

#define	TRUE	1
#define	FALSE	0

#define CM_RED		0
#define CM_GREEN	1
#define CM_BLUE		2

#define	MAX_LWZ_BITS		12

#define INTERLACE		0x40
#define LOCALCOLORMAP	0x80
#define BitSet(byte, bit)	(((byte) & (bit)) == (bit))

#define	ReadOK(file,buffer,len)	SDL_RWread(file, buffer, len, 1)

#define LM_to_uint(a,b)			(((b)<<8)|(a))

static struct {
    unsigned int Width;
    unsigned int Height;
    unsigned char ColorMap[3][MAXCOLORMAPSIZE];
    unsigned int BitPixel;
    unsigned int ColorResolution;
    unsigned int Background;
    unsigned int AspectRatio;
    int GrayScale;
} GifScreen;

/* Replaced by GifFrame
static struct {
    int transparent;
    int delayTime;
    int inputFlag;
    int disposal;
} Gif89;*/

/* 
The Disposal Method subfield contains a value indicating how the graphic 
is to be disposed of once it has been displayed.
	These numbers are right
*/
#define DISPOSAL_UNSPECIFIED 0 /* 00h - Let the viewer decide */
#define DISPOSAL_DONOTDISPOSE 1 /* 01h - Leave graphic there */
#define DISPOSAL_BGCOLORWIPE 2 /* 02h - Overwrite graphic with background color */
#define DISPOSAL_PREVIOUSFRAME 4 /* 04h - Overwrite graphic with previous graphic */

typedef struct GifFrame {
	unsigned int x; /* Offset coordinates of this frame */
	unsigned int y;
	SDL_Surface* surf; /* w/h obtained from surf */
	int disposal; /* Detailed above */
	int delayMs; /* Display time, not really used */
	int transparent; /* index of the transparent pixel */
	int inputFlag; /* Used to click-through frames, but we don't use this */
} GifFrame;

static int ReadColorMap(SDL_RWops * src, int number,
			unsigned char buffer[3][MAXCOLORMAPSIZE], int *flag);
static int DoExtension(GifFrame* frame, SDL_RWops * src, int label);
static int GetDataBlock(SDL_RWops * src, unsigned char *buf);
static int GetCode(SDL_RWops * src, int code_size, int flag);
static int LWZReadByte(SDL_RWops * src, int flag, int input_code_size);
static SDL_Surface*ReadImage(SDL_RWops * src, int len, int height, int,
			unsigned char cmap[3][MAXCOLORMAPSIZE],
			int gray, int interlace, int ignore);

SDL_Surface* convertTo32bit(SDL_Surface* img) {
	/* Convert the image to a 32 bit software surface because I HATE palettes */
	SDL_Surface* formatImage;
	formatImage = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 32, RMASK, GMASK, BMASK, AMASK);

	if (!formatImage)
		return NULL;

	img = SDL_ConvertSurface(img, formatImage->format, SDL_SWSURFACE);
	SDL_FreeSurface(formatImage); /* erase our old 32 bit software image */

	return img;
}
			
/* TODO: (Chase) dump frames into frameset instead of combining into one frame */
int IMG_LoadGIF_RW(SDL_RWops *src, IMG_File* dst)
{
    int start;
    unsigned char buf[16];
    unsigned char c;
    unsigned char localColorMap[3][MAXCOLORMAPSIZE];
    int grayScale;
    int useGlobalColormap;
    int bitPixel;
    int imageCount = 0;
    char version[4];
    int imageNumber = 1; /*<-- I believe this is what we want to control in order to change what frame to grab*/
    int successCount = 0;
	int w = 0;
	int i = 0;
	Uint32 color = 0; 
	SDL_Surface*image = NULL;
	SDL_Surface*nextImage = NULL;
	GifFrame frame;
	GifFrame* frames;
	
    if ( src == NULL ) {
		return 0;
    }
    start = SDL_RWtell(src);

    if (!ReadOK(src, buf, 6)) {
		IMG_SetError("error reading magic number");
        goto done;
    }
    if (strncmp((char *) buf, "GIF", 3) != 0) {
		IMG_SetError("not a GIF file");
        goto done;
    }
    strncpy(version, (char *) buf + 3, 3);
    version[3] = '\0';

    if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0)) {
		IMG_SetError("bad version number, not '87a' or '89a'");
        goto done;
    }
    frame.transparent = -1;
    frame.delayMs = -1;
    frame.inputFlag = -1;
    frame.disposal = 0;

    if (!ReadOK(src, buf, 7)) {
		IMG_SetError("failed to read screen descriptor");
        goto done;
    }
    GifScreen.Width = LM_to_uint(buf[0], buf[1]);
    GifScreen.Height = LM_to_uint(buf[2], buf[3]);
    GifScreen.BitPixel = 2 << (buf[4] & 0x07);
    GifScreen.ColorResolution = (((buf[4] & 0x70) >> 3) + 1);
    GifScreen.Background = buf[5];
    GifScreen.AspectRatio = buf[6];

    if (BitSet(buf[4], LOCALCOLORMAP)) {	/* Global Colormap */
		if (ReadColorMap(src, GifScreen.BitPixel, GifScreen.ColorMap,
			&GifScreen.GrayScale)) {
			IMG_SetError("error reading global colormap");
	        goto done;
		}
    }
	    
    dbgout("GLOBAL BG COLOR: Index:%i RGB:%i,%i,%i\n", GifScreen.Background,
			GifScreen.ColorMap[CM_RED][GifScreen.Background],
			GifScreen.ColorMap[CM_GREEN][GifScreen.Background], 
			GifScreen.ColorMap[CM_BLUE][GifScreen.Background]);
	
	
	frames = (GifFrame*)malloc(sizeof(GifFrame));
	if (!frames) {
		IMG_SetError("Malloc Failed");
		goto done;
	}
	
    do { /* This loop goes through and reads every image and sends its data to ReadImage, if its the one we want (index 1) it'll output SDL_Surface*/
		/*
			What this SHOULD do...
			METHOD 1 (Lazy):
				Read every image, output an SDL_Surface into a vector
				After loop, count vector images, make a new image which has a w equal to the vector.size() * image width
				Render each image from the vector onto the new image, each one with an x offset of image width * index
				Free all surfaces in the vector, return the new combined one.
			METHOD 2 (hardcore):
				Read every image, count the number of images we get.
				Create a surface with a w equal to count * width
				Go back and read every images data, writing their pixels, leftmost to images x * index
			GENERAL PROBLEM:
				We have no delay info. We'd have to manually set the frame delay... However, this isn't a problem. =3
			Looks like we can have local layer color maps.. of course being able to do this in imageready is a whole other issue.
				But as far as we know.. maybe?
		*/
		
		if (!ReadOK(src, &c, 1)) {
		    IMG_SetError("EOF / read error on image data");
	        goto done;
		} else
			dbgout("%c", c);
			
		if (c == ';') {		/* GIF terminator -0x3b */
		    /*if (imageCount < imageNumber) {
				IMG_SetError("only %d image%s found in file",
					imageCount, imageCount > 1 ? "s" : "");
	            goto done;
		    }*/
		    break;
		}
		if (c == '!') {		/* Extension 0x21 */
		    if (!ReadOK(src, &c, 1)) {
				IMG_SetError("EOF / read error on extention function code");
	            goto done;
		    }
		    DoExtension(&frame, src, c);
		    continue;
		}
		if (c != ',') {		/* Image seperator - 0x2c */
		    continue;
		}
		++imageCount;

		if (!ReadOK(src, buf, 9)) {
		    IMG_SetError("couldn't read left/top/width/height");
			goto done;
		}
/*
Offset   Length   Contents
  0      1 byte   Image Separator (0x2c)
  1      2 bytes  Image Left Position
  3      2 bytes  Image Top Position
  5      2 bytes  Image Width
  7      2 bytes  Image Height
  8      1 byte   bit 0:    Local Color Table Flag (LCTF)
                  bit 1:    Interlace Flag
                  bit 2:    Sort Flag
                  bit 2..3: Reserved
                  bit 4..7: Size of Local Color Table: 2^(1+n)
         ? bytes  Local Color Table(0..255 x 3 bytes) if LCTF is one
         1 byte   LZW Minimum Code Size
[ // Blocks
         1 byte   Block Size (s)
œSx    (s)bytes  Image Data
]*
         1 byte   Block Terminator(0x00)
*/

		useGlobalColormap = !BitSet(buf[8], LOCALCOLORMAP);

		frame.x = LM_to_uint(buf[0], buf[1]);
		frame.y = LM_to_uint(buf[2], buf[3]);
		
		dbgout("X:%i, Y:%i\n", frame.x, frame.y);

		bitPixel = 1 << ((buf[8] & 0x07) + 1);
		dbgout("Checkpoint\n");
		if (!useGlobalColormap) {
		    if (ReadColorMap(src, bitPixel, localColorMap, &grayScale)) {
				IMG_SetError("error reading local colormap");
	            goto done;
		    }
		    image = ReadImage(src, LM_to_uint(buf[4], buf[5]),
				      LM_to_uint(buf[6], buf[7]),
				      bitPixel, localColorMap, grayScale,
				      BitSet(buf[8], INTERLACE),
				      0);
			if (image) color = SDL_MapRGB(image->format,
										localColorMap[CM_RED][frame.transparent], 
										localColorMap[CM_GREEN][frame.transparent], 
										localColorMap[CM_BLUE][frame.transparent]);
		} else {
		    image = ReadImage(src, LM_to_uint(buf[4], buf[5]),
				      LM_to_uint(buf[6], buf[7]),
				      GifScreen.BitPixel, GifScreen.ColorMap,
				      GifScreen.GrayScale, BitSet(buf[8], INTERLACE),
				      0); 
			if (image) color = SDL_MapRGB(image->format,
										GifScreen.ColorMap[CM_RED][frame.transparent], 
										GifScreen.ColorMap[CM_GREEN][frame.transparent], 
										GifScreen.ColorMap[CM_BLUE][frame.transparent]);
		}
		dbgout("Image: %p\n", image);
		if (image) {
			++successCount;
			/*image = convertTo32bit(image); */ /* transform to a 32 bit surface because I HATE palettes*/
			dbgout("Converted Image: %ix%i [%p]\n", image->w, image->h, image);
			if (image) {
			    if ( frame.transparent >= 0 ) {
			        SDL_SetColorKey(image, SDL_SRCCOLORKEY, frame.transparent);
			    }
			    /*if ( frame.transparent >= 0 ) {
			        SDL_SetColorKey(image, SDL_SRCCOLORKEY, color);
			        Uint8 r, g, b;
					SDL_GetRGB(color, image->format, &r, &g, &b);
					dbgout("Local Transparency: %i,%i,%i\n", r, g, b);
			    }*/
				/*Reallocate room for another image in the list*/
				frames = (GifFrame*)realloc(frames, successCount * sizeof(GifFrame));
				if (!frames) { /*realloc failed*/
					IMG_SetError("Realloc Failed");
			        goto done;
				}
				frame.surf = image;
				/* Copy the current state of the GifFrame before the next read loop */
				frames[successCount-1] = frame;
				/*int i = 0;
				while (i < successCount) {
					dbgout("Frames[%i]: %p\n", i, frames[i]);	
					i++;
				}*/
				
				image = NULL;
			}
		}
		
    } while (1); /*let the above ... goto's... handle it*/
    
    SDL_Frame* framesArray;
	if (successCount > 0) 
	{
		dbgout("Mallocing Frames %i\n", successCount);fflush(stdout);
		
		framesArray = IMG_MallocFrames(successCount);

		i = 0;
		
		//first frame
		image = SDL_CreateRGBSurface(SDL_SWSURFACE, GifScreen.Width, 
										GifScreen.Height, 
										32, RMASK, GMASK, BMASK, 0);
				
		color = SDL_MapRGB(image->format,
							GifScreen.ColorMap[CM_RED][frames[0].transparent], 
							GifScreen.ColorMap[CM_GREEN][frames[0].transparent], 
							GifScreen.ColorMap[CM_BLUE][frames[0].transparent]);
		Uint8 r, g, b;
		SDL_GetRGB(color, image->format, &r, &g, &b);
		dbgout("Final Transparency: %i,%i,%i\n", r, g, b);fflush(stdout);
		if (color != 0) /* Ignore 0,0,0 as the transparent color */
		{ 
			SDL_FillRect(image, NULL, color);
			SDL_SetColorKey(image, SDL_SRCCOLORKEY, color); /* use the first frames trans color */
		}

		while (i < successCount) /* render our surfs and clear */
		{
			if (image)
			{
				dbgout("Dimensions: %ix%i: %p\n", image->w, image->h, image);fflush(stdout);

				w = 0;

				SDL_Rect r2, r3; 
				
				dbgout("Adding Frames[%i]: %p Disposal:%i TransIndex: %i Delay: %i Input:%i\n", i, 
						frames[i].surf, frames[i].disposal, frames[i].transparent, frames[i].delayMs,
						frames[i].inputFlag);	fflush(stdout);
						
				/* Print out the overlay at its offset coordinates */
				r2.x = frames[i].x;
				r2.y = frames[i].y; 
				r2.w = frames[i].surf->w;
				r2.h = frames[i].surf->h;
				
				dbgout("Drawing at: %i,%i\n", r2.x, r2.y);fflush(stdout);
				if (SDL_BlitSurface(frames[i].surf, NULL, image, &r2)) 
				{
					/*something bad happened but ignore it for now. */
					dbgout("Drawing Failed: %s\n", IMG_GetError());fflush(stdout);
				}
				dbgout("Setting Crap\n");fflush(stdout);
				//add image to our frames list
				framesArray[i].surf = image;
				framesArray[i].delay = frames[i].delayMs * 10; //HACK: All observed gifs have a delay of say.. 7, which means 70ms.
				framesArray[i].key = NULL;
				
				dbgout("i+1 crap\n");fflush(stdout);
				/*what to do in frame[i+1] before rendering*/
				if (i + 1 < successCount)
				{
					//create next frame so we can do something to it
					nextImage = SDL_CreateRGBSurface(SDL_SWSURFACE, GifScreen.Width, 
										GifScreen.Height, 
										32, RMASK, GMASK, BMASK, 0);
									
					color = SDL_MapRGB(nextImage->format,
										GifScreen.ColorMap[CM_RED][frames[0].transparent], 
										GifScreen.ColorMap[CM_GREEN][frames[0].transparent], 
										GifScreen.ColorMap[CM_BLUE][frames[0].transparent]);
					Uint8 r, g, b;
					SDL_GetRGB(color, nextImage->format, &r, &g, &b);
					dbgout("Final Transparency: %i,%i,%i\n", r, g, b);fflush(stdout);
					if (color != 0) /* Ignore 0,0,0 as the transparent color */
					{ 
						SDL_FillRect(nextImage, NULL, color);
						SDL_SetColorKey(nextImage, SDL_SRCCOLORKEY, color); /* use the first frames trans color */
					}
				
					dbgout("Disposal crap\n");fflush(stdout);
					switch (frames[i].disposal)
					{
						case DISPOSAL_PREVIOUSFRAME: /* 04h - Overwrite graphic with previous graphic */
							dbgout("Doing previous frame\n");fflush(stdout);
							r2.x = 0;
							r2.y = 0;
							r2.w = GifScreen.Width;
							r2.h = GifScreen.Height;
							SDL_BlitSurface(frames[0].surf, NULL, nextImage, &r2); //Since I'm lazy, and haven't seen many gifs use this, it'll just render the original frame
							break;
						case DISPOSAL_UNSPECIFIED: /* DISPOSAL_UNSPECIFIED 00h - Let the viewer decide */
							/*Drop down to donotdispose */
						case DISPOSAL_DONOTDISPOSE: /* 01h - Leave graphic there */
							dbgout("Doing nondispose\n");fflush(stdout);
							/*render a copy of the previous i-1 into i */
							
							r2.w = GifScreen.Width;
							r2.h = GifScreen.Height;
							r2.x = 0;
							r2.y = 0;
							/* r3 = src, r2 = dst */
							r3.w = GifScreen.Width;
							r3.h = GifScreen.Height;
							r3.x = 0;
							r3.y = 0;
							dbgout("r3:(%i,%i)%ix%i r2:(%i,%i)%ix%i\n", r3.x, r3.y, r3.w, r3.h, 
																	  r2.x, r2.y, r2.w, r2.h);fflush(stdout);

							if (SDL_BlitSurface(image, &r3, nextImage, &r2)) {
								dbgout("Drawing Failed: %s\n", IMG_GetError());fflush(stdout);
							}
							break;	
						default: /* This'll default to DISPOSAL_BGCOLORWIPE 
									02h - Overwrite graphic with background color 
									do nothing, next rect is empty already*/
							dbgout("Doing default %i\n", frames[i].disposal);fflush(stdout);
							break;
					}
					image = nextImage;
				}
			}
			i++;
		}
		
		i = 0;
		while (i < successCount) {
			SDL_FreeSurface(frames[i].surf);
			i++;
		}
		free(frames);
		frames = NULL;
	} else {
		image = NULL;
	}
	
done:
	if (frames) { /* We didn't complete successfully, handle any leaks */
		i = 0;
		while (i < successCount) {
			SDL_FreeSurface(frames[i].surf);
			i++;
		}
		free(frames);
		/*if (image) {
			SDL_FreeSurface(image);
			image = NULL; 
		}*/
	}
	
	/*if ( image == NULL ) {
        SDL_RWseek(src, start, SEEK_SET);
    }   */
		
	if (successCount > 0)
	{
		dst->frames = framesArray;
		dst->count = successCount;
		dst->format = IMG_FORMAT_GIF;
		return 1;
	}
	return 0;
		
    //return IMG_SurfaceToFrameset(image, sdlFrames, sdlFrameCount);
}

static int
ReadColorMap(SDL_RWops *src, int number,
             unsigned char buffer[3][MAXCOLORMAPSIZE], int *gray)
{
    int i;
    unsigned char rgb[3];
    int flag;

    flag = TRUE;

    for (i = 0; i < number; ++i) {
		if (!ReadOK(src, rgb, sizeof(rgb))) {
		    IMG_SetError("bad colormap");
		    return 1;
		}
		buffer[CM_RED][i] = rgb[0];
		buffer[CM_GREEN][i] = rgb[1];
		buffer[CM_BLUE][i] = rgb[2];
		flag &= (rgb[0] == rgb[1] && rgb[1] == rgb[2]);
    }

#if 0
    if (flag)
	*gray = (number == 2) ? PBM_TYPE : PGM_TYPE;
    else
	*gray = PPM_TYPE;
#else
    *gray = 0;
#endif

    return FALSE;
}

static int
DoExtension(GifFrame* frame, SDL_RWops *src, int label)
{
    static unsigned char buf[256];
    char *str;

    switch (label) {
	    case 0x01:			/* Plain Text Extension */
			dbgout("Plain Text Extension\n");
			break;
		case 0xff:			/* Application Extension */
			dbgout("Application Extension\n");
			break;
	    case 0xfe:			/* Comment Extension */
			dbgout("Comment Extension\n");
			while (GetDataBlock(src, (unsigned char *) buf) != 0)
			    ;
			return FALSE;
	    case 0xf9:			/* Graphic Control Extension */
			dbgout("Graphic Control Extension\n");
			(void) GetDataBlock(src, (unsigned char *) buf);
			frame->disposal = (buf[0] >> 2) & 0x7;
			frame->inputFlag = (buf[0] >> 1) & 0x1;
			frame->delayMs = LM_to_uint(buf[1], buf[2]);
			if ((buf[0] & 0x1) != 0)
			    frame->transparent = buf[3];

			while (GetDataBlock(src, (unsigned char *) buf) != 0)
			    ;
			return FALSE;
	    default:
			dbgout("Unknown Extension: %s (0x%02x)\n", (char*)buf, label);
			break;
    }

    while (GetDataBlock(src, (unsigned char *) buf) != 0)
	;

    return FALSE;
}

static int ZeroDataBlock = FALSE;

static int
GetDataBlock(SDL_RWops *src, unsigned char *buf)
{
    unsigned char count;

    if (!ReadOK(src, &count, 1)) {
		dbgout("error in getting DataBlock size\n");
		return -1;
    }
    ZeroDataBlock = count == 0;

    if ((count != 0) && (!ReadOK(src, buf, count))) {
		dbgout("error in reading DataBlock\n");
		return -1;
    }
    return count;
}

static int
GetCode(SDL_RWops *src, int code_size, int flag)
{
    static unsigned char buf[280];
    static int curbit, lastbit, done, last_byte;
    int i, j, ret;
    unsigned char count;

    if (flag) {
		curbit = 0;
		lastbit = 0;
		done = FALSE;
		return 0;
    }
    if ((curbit + code_size) >= lastbit) {
		if (done) {
		    if (curbit >= lastbit)
			IMG_SetError("ran off the end of my bits");
		    return -1;
		}
		buf[0] = buf[last_byte - 2];
		buf[1] = buf[last_byte - 1];

		if ((count = GetDataBlock(src, &buf[2])) == 0)
		    done = TRUE;

		last_byte = 2 + count;
		curbit = (curbit - lastbit) + 16;
		lastbit = (2 + count) * 8;
    }
    ret = 0;
    for (i = curbit, j = 0; j < code_size; ++i, ++j)
		ret |= ((buf[i / 8] & (1 << (i % 8))) != 0) << j;

    curbit += code_size;

    return ret;
}

static int
LWZReadByte(SDL_RWops *src, int flag, int input_code_size)
{
    static int fresh = FALSE;
    int code, incode;
    static int code_size, set_code_size;
    static int max_code, max_code_size;
    static int firstcode, oldcode;
    static int clear_code, end_code;
    static int table[2][(1 << MAX_LWZ_BITS)];
    static int stack[(1 << (MAX_LWZ_BITS)) * 2], *sp;
    register int i;

    if (flag) {
		set_code_size = input_code_size;
		code_size = set_code_size + 1;
		clear_code = 1 << set_code_size;
		end_code = clear_code + 1;
		max_code_size = 2 * clear_code;
		max_code = clear_code + 2;

		GetCode(src, 0, TRUE);

		fresh = TRUE;

		for (i = 0; i < clear_code; ++i) {
		    table[0][i] = 0;
		    table[1][i] = i;
		}
		for (; i < (1 << MAX_LWZ_BITS); ++i)
		    table[0][i] = table[1][0] = 0;

		sp = stack;

		return 0;
    } else if (fresh) {
		fresh = FALSE;
		do {
		    firstcode = oldcode = GetCode(src, code_size, FALSE);
		} while (firstcode == clear_code);
		
		return firstcode;
    }
    if (sp > stack)
		return *--sp;

    while ((code = GetCode(src, code_size, FALSE)) >= 0) {
		if (code == clear_code) {
		    for (i = 0; i < clear_code; ++i) {
			table[0][i] = 0;
			table[1][i] = i;
		    }
		    for (; i < (1 << MAX_LWZ_BITS); ++i)
			table[0][i] = table[1][i] = 0;
		    code_size = set_code_size + 1;
		    max_code_size = 2 * clear_code;
		    max_code = clear_code + 2;
		    sp = stack;
		    firstcode = oldcode = GetCode(src, code_size, FALSE);
		    return firstcode;
		} else if (code == end_code) {
		    int count;
		    unsigned char buf[260];

		    if (ZeroDataBlock)
				return -2;

		    while ((count = GetDataBlock(src, buf)) > 0)
			;

		    if (count != 0) {
			/*
			 * pm_message("missing EOD in data stream (common occurence)");
			 */
				dbgout("missing EOD in data stream (common occurence)\n");
		    }
		    return -2;
		}
		incode = code;

		if (code >= max_code) {
		    *sp++ = firstcode;
		    code = oldcode;
		}
		while (code >= clear_code) {
		    *sp++ = table[1][code];
		    if (code == table[0][code])
				IMG_SetError("circular table entry BIG ERROR");
		    code = table[0][code];
		}

		*sp++ = firstcode = table[1][code];

		if ((code = max_code) < (1 << MAX_LWZ_BITS)) {
		    table[0][code] = oldcode;
		    table[1][code] = firstcode;
		    ++max_code;
		    if ((max_code >= max_code_size) &&
				(max_code_size < (1 << MAX_LWZ_BITS))) {
				max_code_size *= 2;
				++code_size;
		    }
		}
		oldcode = incode;

		if (sp > stack)
		    return *--sp;
    }
    return code;
}

static SDL_Surface*
ReadImage(SDL_RWops * src, int len, int height, int cmapSize,
	  unsigned char cmap[3][MAXCOLORMAPSIZE],
	  int gray, int interlace, int ignore)
{
    SDL_Surface*image;
    unsigned char c;
    int i, v;
    int xpos = 0, ypos = 0, pass = 0;

    /*
    **	Initialize the compression routines
     */
    if (!ReadOK(src, &c, 1)) {
		IMG_SetError("EOF / read error on image data");
		return NULL;
    }
    if (LWZReadByte(src, TRUE, c) < 0) {
		IMG_SetError("error reading image");
		return NULL;
    }
    /*
    **	If this is an "uninteresting picture" ignore it.
     */
    if (ignore) {
		while (LWZReadByte(src, FALSE, c) >= 0)
		    ;
		return NULL;
    }
    image = ImageNewCmap(len, height, cmapSize);

    for (i = 0; i < cmapSize; i++)
		ImageSetCmap(image, i, cmap[CM_RED][i],
				cmap[CM_GREEN][i], cmap[CM_BLUE][i]);

    while ((v = LWZReadByte(src, FALSE, c)) >= 0) {

	((Uint8 *)image->pixels)[xpos + ypos * image->pitch] = v;

	++xpos;
	if (xpos == len) {
	    xpos = 0;
	    if (interlace) {
			switch (pass) {
				case 0:
				case 1:
				    ypos += 8;
				    break;
				case 2:
				    ypos += 4;
				    break;
				case 3:
				    ypos += 2;
				    break;
			}

			if (ypos >= height) {
			    ++pass;
			    switch (pass) {
				    case 1:
						ypos = 4;
						break;
				    case 2:
						ypos = 2;
						break;
				    case 3:
						ypos = 1;
						break;
				    default:
						goto fini;
			    }
			}
	    } else {
			++ypos;
	    }
	}
	if (ypos >= height)
	    break;
    }

  fini:

    return image;
}

#endif /* LOAD_GIF */
