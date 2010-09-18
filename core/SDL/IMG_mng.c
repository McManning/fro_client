/*
    SDL_mng:  A library to load MNG files
    Copyright (C) 2003, Thomas Kircher

    PNG code based on SDL_png.c, Copyright (C) 1998 Philippe Lavoie

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>

#include <png.h>

#include <SDL/SDL.h>
#include <SDL/SDL_endian.h>
#include "SDL_Image.h"

/* Chunk structure */
typedef struct
{
    unsigned int  chunk_ID;
    unsigned int  chunk_size;
    char         *chunk_data;
    unsigned int  chunk_CRC;
}
chunk_t;

typedef struct MNG_TempFrame
{
    SDL_Frame* frame;
    struct MNG_TempFrame* next;
}
MNG_TempFrame;

typedef struct
{
    unsigned int Frame_width;
    unsigned int Frame_height;
    unsigned int Ticks_per_second;
    unsigned int Nominal_layer_count;
    unsigned int Nominal_frame_count;
    unsigned int Nominal_play_time;
    unsigned int Simplicity_profile;
}
MHDR_chunk;

/* http://www.libpng.org/pub/mng/spec/mng-lc.html#mng-FRAM */
typedef struct
{
	unsigned char Framing_mode;
	/* char* Subframe_name; exists, but I don't give a shit */
	unsigned char Change_interframe_delay;
	unsigned char Change_timeout_and_termination;
	unsigned char Change_layer_clipping_boundaries;
	unsigned char Change_sync_id_list;
	
	/* timeout crap (the only ones we really care about) */
	unsigned int Interframe_delay; /*	This field must be omitted if the change_interframe_delay 
										field is zero or is omitted. */
	unsigned int Timeout; /* Same rules as Interframe_delay */
	
	/* clipping boundary crap 
		This and the following four fields must be omitted if the
		change_layer_clipping_boundaries field is zero or is
		omitted.
	
	unsigned int Layer_clipping_boundary_delta_type;
	unsigned int Left_layer_cb;
	unsigned int Right_layer cb;
	unsigned int Top_layer_cb;
	unsigned int Bottom_layer_cb;*/
	/*
		Must be omitted if
        change_sync_id_list=0 and can be omitted if the new
        list is empty; repeat until all sync_ids have been
        listed. 
	
	unsigned int Sync_id;*/
}
FRAM_chunk;

/* MNG_Image type */
typedef struct
{
    MHDR_chunk mhdr;
    FRAM_chunk fram; //current frame properties, copied into each frame as they're created

    //global palette
    png_colorp globalPalette;
    int globalPaletteSize;
    
    //global transparency
	png_bytep trans;
	int num_trans;
	png_color_16p trans_values;

}
MNG_Image;

/* Some select chunk IDs, from libmng.h */
#define MNG_UINT_MHDR 0x4d484452L
#define MNG_UINT_BACK 0x4241434bL
#define MNG_UINT_PLTE 0x504c5445L
#define MNG_UINT_tRNS 0x74524e53L
#define MNG_UINT_IHDR 0x49484452L
#define MNG_UINT_IDAT 0x49444154L
#define MNG_UINT_IEND 0x49454e44L
#define MNG_UINT_MEND 0x4d454e44L
#define MNG_UINT_FRAM 0x4652414dL
#define MNG_UINT_LOOP 0x4c4f4f50L
#define MNG_UINT_ENDL 0x454e444cL
#define MNG_UINT_TERM 0x5445524dL
#define MNG_UINT_JHDR 0x4a484452L

/* Internal MNG functions */
unsigned char MNG_read_byte     (SDL_RWops *src);
unsigned int  MNG_read_uint32   (SDL_RWops *src);
chunk_t       MNG_read_chunk    (SDL_RWops *src);

/* Return 1 on success, fills surf and key (if applicable) of dst */
int MNG_read_frame(SDL_RWops *src, MNG_Image* image, SDL_Frame* dst);

/* Return whether or not the file claims to be an MNG */
int IMG_isMNG(SDL_RWops *src)
{
    unsigned char buf[8];

    if( SDL_RWread(src, buf, 1, 8) != 8 )
        return 0;

    return( !memcmp(buf, "\212MNG\r\n\032\n", 8) );
}

/* Read a byte from the src stream */
unsigned char MNG_read_byte(SDL_RWops *src)
{
    unsigned char ch;

    SDL_RWread(src, &ch, 1, 1);

    return ch;
}

/* Read a 4-byte uint32 from the src stream */
unsigned int MNG_read_uint32(SDL_RWops *src)
{
    unsigned char buffer[4];
    unsigned int value;

    buffer[0] = MNG_read_byte(src); buffer[1] = MNG_read_byte(src);
    buffer[2] = MNG_read_byte(src); buffer[3] = MNG_read_byte(src);

    value  = buffer[0] << 24; value |= buffer[1] << 16;
    value |= buffer[2] << 8;  value |= buffer[3];

    return value;
}

/* Read an MNG chunk */
chunk_t MNG_read_chunk(SDL_RWops *src)
{
    chunk_t this_chunk;
    unsigned int i;

    this_chunk.chunk_size = MNG_read_uint32(src);
    this_chunk.chunk_ID   = MNG_read_uint32(src);

    this_chunk.chunk_data =
        (char *)calloc(sizeof(char), this_chunk.chunk_size);

    SDL_RWread(src, this_chunk.chunk_data, 1, this_chunk.chunk_size);

    this_chunk.chunk_CRC = MNG_read_uint32(src);

    return this_chunk;
}

/* Read MHDR chunk data */
MHDR_chunk read_MHDR(SDL_RWops *src)
{
    MHDR_chunk mng_header;

    mng_header.Frame_width         = MNG_read_uint32(src);
    mng_header.Frame_height        = MNG_read_uint32(src);
    mng_header.Ticks_per_second    = MNG_read_uint32(src);
    mng_header.Nominal_layer_count = MNG_read_uint32(src);
    mng_header.Nominal_frame_count = MNG_read_uint32(src);
    mng_header.Nominal_play_time   = MNG_read_uint32(src);
    mng_header.Simplicity_profile  = MNG_read_uint32(src);

    /* skip CRC bits */
    MNG_read_uint32(src);

    return mng_header;
}

FRAM_chunk read_FRAM(SDL_RWops *src)
{
	FRAM_chunk fram;
	int bytesRead = 0;
	
	fram.Framing_mode = MNG_read_byte(src);
	bytesRead++;
	
	dbgout("IMG_mng FRAM: Framing_mode:%i\n", fram.Framing_mode);fflush(stdout);
	
	char c;
	do //skip over Subframe_name (ends with a null character)
	{
		c = MNG_read_byte(src);
		bytesRead++;
	} while (c != 0);
	
	dbgout("IMG_mng FRAM: reading changes\n");fflush(stdout);
	
	fram.Change_interframe_delay 			= MNG_read_byte(src);
	fram.Change_timeout_and_termination 	= MNG_read_byte(src);
	fram.Change_layer_clipping_boundaries 	= MNG_read_byte(src);
	fram.Change_sync_id_list 				= MNG_read_byte(src);
	bytesRead += 4;

	if (fram.Change_interframe_delay != 0)
	{
		dbgout("IMG_mng FRAM: delay changed\n");fflush(stdout);
		fram.Interframe_delay 	= MNG_read_uint32(src);
		//fram.Timeout 			= MNG_read_uint32(src);
		bytesRead += sizeof(unsigned int);
		dbgout("IMG_mng FRAM: interframe_delay:%i\n", fram.Interframe_delay);fflush(stdout);
	}
	//rest of the crap: I don't care.
	
	//seek backward the amount of bytes read so we can skip the rest 
	SDL_RWseek(src, -(bytesRead), SEEK_CUR);
	
	dbgout("IMG_mng FRAM: Skipping useless crap\n");fflush(stdout);
	return fram;
}

/* png_read_data callback; return <size> bytes from wherever */
static void png_read_data(png_structp ctx, png_bytep area, png_size_t size)
{
	SDL_RWops *src;

	src = (SDL_RWops *)png_get_io_ptr(ctx);
	SDL_RWread(src, area, size, 1);
}

//int MNG_iterate_chunks(SDL_RWops *src, SDL_Frame** frames, int* count)
int IMG_LoadMNG_RW(SDL_RWops* src, IMG_File* dst)
{
    chunk_t current_chunk;

    unsigned int byte_count  = 0;
    unsigned int frame_count = 0;
    unsigned int i;

	int count = 0;
	SDL_Frame* frames = NULL;
	
    MNG_Image image;
	image.globalPalette = NULL;
    image.globalPaletteSize = 0;
	image.trans = NULL;
	image.num_trans = 0;
	image.trans_values = NULL;
	
	int doneWithHeader = 0; /* set to 1 once we encounter a png */
	int IHDR_position = 0;

	dbgout("IMG_mng Looping MNG Headers\n");fflush(stdout);

    do {
        current_chunk = MNG_read_chunk(src);
        byte_count += current_chunk.chunk_size + 12;

        switch(current_chunk.chunk_ID)
        {
            /* Read MHDR chunk, and store in image struct */
            case MNG_UINT_MHDR:
				//seek back to start to read
                SDL_RWseek(src, -(current_chunk.chunk_size + 4), SEEK_CUR);
                image.mhdr = read_MHDR(src);
                
                /* I'm not writing support for JNG, so check and break out if invalid
					TODO: this
					bit 4: JNG
					   0: JNG and JDAA are absent.
					   1: JNG or JDAA may be present.
					   (must be 0 in MNG-LC datastreams)
				*/
                /*if (image->mhdr.Simplicity_profile & (1 << 4))
				{
					dbgout("Has JNG\n");	
				}
				else
					dbgout("No JNG\n");
					
				fflush(stdout);*/
                
                break;
				
			/* 
				set frame properties. Only appears if there's a differences between two frames.
				All frames that appear after this FRAM is declared would use its settings
			*/
			case MNG_UINT_FRAM:
				dbgout("IMG_mng Reading FRAM\n");fflush(stdout);
				//seek back to start to read
				SDL_RWseek(src, -(current_chunk.chunk_size + 4), SEEK_CUR);
				image.fram = read_FRAM(src);
				//Seek forward again, skipping the crap we didn't want to read from FRAM
				SDL_RWseek(src, current_chunk.chunk_size + 4, SEEK_CUR);
				dbgout("IMG_mng Read Done\n");fflush(stdout);
				
				break;

            /* Set global color palette. Only on the FIRST read of this. */ 
            case MNG_UINT_PLTE:
				/*TODO: set up global palette, if png doesn't have a palette, use
					this.
					
					The number of entries is determined from the chunk length. 
					A chunk length not divisible by 3 is an error.
				*/
				if (doneWithHeader)
				{
					dbgout("IMG_mng Encountered Plte that will be ignored\n");fflush(stdout);
				}
				else
				{
					if (current_chunk.chunk_size % 3 != 0) 
					{
						dbgout("IMG_mng MNG_UINT_PLTE Not divisible by 3\n");fflush(stdout);
						goto done;
					}
					/* store in our main image */
					image.globalPalette = (png_colorp)current_chunk.chunk_data;
					
					/* size is the number of colors (3 bytes each) */
					image.globalPaletteSize = current_chunk.chunk_size / 3;
					
					dbgout("IMG_mng Palette Size: %i\n", image.globalPaletteSize);fflush(stdout);
				}
				break;

			/* set global transparency. Only on the FIRST read of this */
			case MNG_UINT_tRNS:
				if (doneWithHeader)
				{
					dbgout("IMG_mng Encountered Plte that will be ignored\n");fflush(stdout);
				}
				else
				{
					/*
						http://www.libpng.org/pub/png/spec/iso/index-object.html#11tRNS
						Depending on color type, there's various formats of this crap.
						To ease useage, let's just guess and check them to see what 
						giam will produce. :| 
						I BELIEVE giam will produce the palette version all the time, since
						it's lazy. (color_type == 3) So let's that one
						
						color_type 3:
							the tRNS chunk contains a series of one-byte alpha values, 
							corresponding to entries in the PLTE chunk. 
					*/
					image.trans_values = NULL; //for non-palette images
					image.trans = (png_bytep)current_chunk.chunk_data;
					image.num_trans = current_chunk.chunk_size;
					dbgout("IMG_mng Created global tRNS. num_trans:%i trans:%p, trans_values:%p\n",
								image.num_trans, image.trans, image.trans_values);fflush(stdout);
				}
				
				break;

            /* Reset our byte count */
            case MNG_UINT_IHDR:
				doneWithHeader = 1;
                byte_count = current_chunk.chunk_size + 12;
                dbgout("IMG_mng Found IHDR\n");fflush(stdout);
                IHDR_position = byte_count;
                break;
                
            /* We've reached the end of a PNG - seek to IHDR (if it exists) and read */
            case MNG_UINT_IEND:
				if (IHDR_position == 0)
				{
					dbgout("IMG_mng Zero IHDR_position\n");	fflush(stdout);
					count = 0;
					goto done;
				}
				else
				{
					dbgout("IMG_mng IEND, reading from IHDR at byte_count: %i\n", byte_count);fflush(stdout);
					
	                SDL_RWseek(src, -byte_count, SEEK_CUR);
	
					if (frames == NULL)
	                {
						frames = (SDL_Frame*)malloc(sizeof(SDL_Frame));
						count = 1;
	                }
	                else
	                {
						frames = (SDL_Frame*)realloc(frames, (count + 1) * sizeof(SDL_Frame));
						if (!frames) { /*realloc failed*/
							dbgout("IMG_mng Realloc Failed\n");
							count = 0;
							goto done;
						}
						count++;
	                }
					
					if ( MNG_read_frame(src, &image, &frames[count-1]) )
					{
						/* Since mngs can have a different TPS instead of 1000, factor it in */
						frames[count-1].delay = 
							1000 / image.mhdr.Ticks_per_second * image.fram.Interframe_delay;
					}
					else
					{
						dbgout("IMG_mng Couldn't read frame\n");fflush(stdout);
						count = 0;
						goto done;
					}
					
	                IHDR_position = 0;
				}
                break;

            default:
                break;
        }
    }
    while(current_chunk.chunk_ID != MNG_UINT_MEND);

	dbgout("IMG_mng \nMEND HIT. Beginning copy\n");fflush(stdout);

done:
	if (count < 1 || !frames)
	{
		dbgout("IMG_mng No frames\n");fflush(stdout);
		if (frames)
			free(frames);
		dst->frames = NULL;
		dst->count = 0;
	}
	else
	{
	    dbgout("IMG_mng Finalizing Frames!\n");fflush(stdout);
	    dst->frames = frames;
	    dst->count = count;
	    dst->format = IMG_FORMAT_MNG;
	    dbgout("IMG_mng Finalized!\n");fflush(stdout);
	}
	
	dbgout("IMG_mng Returning!\n");fflush(stdout);
	return (dst->count > 0);
}
	
/* Read a PNG frame from the MNG file */
int MNG_read_frame(SDL_RWops *src, MNG_Image* image, SDL_Frame* dst)
{
	SDL_Surface *volatile surface;
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	Uint32 Rmask;
	Uint32 Gmask;
	Uint32 Bmask;
	Uint32 Amask;
	SDL_Palette *palette;
	png_bytep *volatile row_pointers;
	int row, i;
	volatile int ckey = -1;
	png_color_16 *transv;

	/* Initialize the data we will clean up when we're done */
	png_ptr = NULL; info_ptr = NULL; row_pointers = NULL; surface = NULL;

	/* Check to make sure we have something to do */
	if ( ! src ) 
	{
		goto done;
	}

	/* Create the PNG loading context structure */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
					  NULL,NULL,NULL);
	if (png_ptr == NULL)
	{
		SDL_SetError("Couldn't allocate memory for PNG file");fflush(stdout);
		goto done;
	}

	 /* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) 
	{
		SDL_SetError("Couldn't create image information for PNG file");fflush(stdout);
		goto done;
	}

	/* Set error handling if you are using setjmp/longjmp method (this is
	 * the normal method of doing things with libpng).  REQUIRED unless you
	 * set up your own error handlers in png_create_read_struct() earlier.
	 */
	if ( setjmp(png_ptr->jmpbuf) ) 
	{
		SDL_SetError("Error reading the PNG file.");fflush(stdout);
		goto done;
	}

	/* Set up the input control */
	png_set_read_fn(png_ptr, src, png_read_data);

        /* tell PNG not to read the signature */
        png_set_sig_bytes(png_ptr, 8);

	/* Read PNG header info */
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, NULL, NULL);

	/* tell libpng to strip 16 bit/color files down to 8 bits/color */
	png_set_strip_16(png_ptr);

	/* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
	 * byte into separate bytes (useful for paletted and grayscale images).
	 */
	png_set_packing(png_ptr);

	/* scale greyscale values to the range 0..255 */
	if(color_type == PNG_COLOR_TYPE_GRAY)
		png_set_expand(png_ptr);

	/* For images with a single "transparent colour", set colour key;
	   if more than one index has transparency, or if partially transparent
	   entries exist, use full alpha channel */


	/*
		If we have an empty palette, use global
	*/
	if (color_type == PNG_COLOR_TYPE_PALETTE && info_ptr->num_palette < 1) 
	{
		png_set_PLTE(png_ptr, info_ptr, image->globalPalette, image->globalPaletteSize);
		dbgout("Changing palette. New Size: %i\n", info_ptr->num_palette);fflush(stdout);
	}
	else
	{
		dbgout("Not modifying palette\n");fflush(stdout);
	}

	/*
		If we don't have transparency information, use global
	*/
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) == 0)
	{
		dbgout("Using global transparency %i 0x%p 0x%p\n", image->num_trans, image->trans, image->trans_values);fflush(stdout);
		png_set_tRNS(png_ptr, info_ptr, image->trans, image->num_trans, image->trans_values);
		dbgout("Trans set\n");fflush(stdout);
	}
	else
	{
		dbgout("Not modifying transparency\n");fflush(stdout);
	}
	

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) 
	{
		dbgout("Using internal transparency\n");
	    int num_trans;
		Uint8 *trans;
		png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &transv);
		if(color_type == PNG_COLOR_TYPE_PALETTE) 
		{
		    /* Check if all tRNS entries are opaque except one */
		    int i, t = -1;
		    for(i = 0; i < num_trans; i++)
		    {
				if(trans[i] == 0) 
				{
				    if(t >= 0)
					break;
				    t = i;
				} 
				else if(trans[i] != 255)
				{
				    break;
				}
			}
			
		    if(i == num_trans) 
			{
				/* exactly one transparent index */
				ckey = t;
		    } 
			else 
			{
				/* more than one transparent index, or translucency */
				png_set_expand(png_ptr);
		    }
		} else
		    ckey = 0; /* actual value will be set later */
	}
	
	if ( color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
		png_set_gray_to_rgb(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, NULL, NULL);


///////////////////////////////////////////////////////////

	dbgout("\n\nPNG INFO\n");
	dbgout("width:%i height:%i\n", info_ptr->width, info_ptr->height);
	dbgout("valid:%i rowbytes:%i num_palette:%i\n", 
			info_ptr->valid, info_ptr->rowbytes, info_ptr->num_palette);
	dbgout("num_trans:%i bit_depth:%i color_type:%i\n", 
			info_ptr->num_trans, info_ptr->bit_depth, info_ptr->color_type);
	dbgout("compression_type:%i filter_type:%i interlace_type:%i\n", 
			info_ptr->compression_type, info_ptr->filter_type, info_ptr->interlace_type);
	dbgout("channels:%i pixel_depth:%i spare_byte:%i\n", 
			info_ptr->channels, info_ptr->pixel_depth, info_ptr->spare_byte);
	if (info_ptr->text)
	{
		dbgout("text:%s key:%s\n", info_ptr->text->text, info_ptr->text->key);	
	}
	//output RAW CRAP
	if (info_ptr->valid & PNG_INFO_IDAT)
	{
		dbgout("OUTPUTTING RAW DATA\n");
		for (int row = 0; row < info_ptr->height; row++) 
		{
			for (int col = 0; col < info_ptr->width; col++)
			{
				dbgout("%i", info_ptr->row_pointers[row][col]);
			}
		}
		dbgout("DONE!\n");
	}
	else
	{
		dbgout("info_ptr->valid & PNG_INFO_IDAT zero\n");
	}
	
	dbgout("FLAG CHECKING:\n");
	
#ifdef SDL_IMAGE_DEBUG
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_gAMA) != 0) dbgout("PNG_INFO_gAMA\n");
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_sBIT) != 0) dbgout("PNG_INFO_sBIT\n");
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_cHRM) != 0) dbgout("PNG_INFO_cHRM\n");
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_IDAT) != 0) dbgout("PNG_INFO_IDAT\n");
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_iCCP) != 0) dbgout("PNG_INFO_iCCP\n");
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_sRGB) != 0) dbgout("PNG_INFO_sRGB\n");
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_oFFs) != 0) dbgout("PNG_INFO_oFFs\n");
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_pHYs) != 0) dbgout("PNG_INFO_pHYs\n");
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) != 0) dbgout("PNG_INFO_tRNS\n");
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE) != 0) dbgout("PNG_INFO_PLTE\n");
#endif
	dbgout("\n\n");
	
	fflush(stdout);
///////////////////////////////////////////////////////////

	/* Allocate the SDL surface to hold the image */
	Rmask = Gmask = Bmask = Amask = 0 ; 
	if ( color_type != PNG_COLOR_TYPE_PALETTE ) {
		if ( SDL_BYTEORDER == SDL_LIL_ENDIAN ) {
			Rmask = 0x000000FF;
			Gmask = 0x0000FF00;
			Bmask = 0x00FF0000;
			Amask = (info_ptr->channels == 4) ? 0xFF000000 : 0;
		} else {
		    int s = (info_ptr->channels == 4) ? 0 : 8;
			Rmask = 0xFF000000 >> s;
			Gmask = 0x00FF0000 >> s;
			Bmask = 0x0000FF00 >> s;
			Amask = 0x000000FF >> s;
		}
	}
	surface = SDL_AllocSurface(SDL_SWSURFACE, width, height,
			bit_depth*info_ptr->channels, Rmask,Gmask,Bmask,Amask);
	if ( surface == NULL ) 
	{
		SDL_SetError("Out of memory");
		goto done;
	}

	if (ckey != -1) 
	{
	        if(color_type != PNG_COLOR_TYPE_PALETTE)
	        {
			/* FIXME: Should these be truncated or shifted down? */
		        ckey = SDL_MapRGB(surface->format,
			                 (Uint8)transv->red,
			                 (Uint8)transv->green,
			                 (Uint8)transv->blue);
			}
	        SDL_SetColorKey(surface, SDL_SRCCOLORKEY, ckey);
	}

	/* Create the array of pointers to image data */
	row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*height);
	if ( (row_pointers == NULL) ) 
	{
		SDL_SetError("Out of memory");
		SDL_FreeSurface(surface);
		surface = NULL;
		goto done;
	}
	
	for (row = 0; row < (int)height; row++) 
	{
		row_pointers[row] = (png_bytep)
				(Uint32 *)surface->pixels + row*surface->pitch;
	}

	/* Read the entire image in one go */
	png_read_image(png_ptr, row_pointers);

	/* read rest of file, get additional chunks in info_ptr - REQUIRED */
	png_read_end(png_ptr, info_ptr);

	/* Load the palette, if any */
	palette = surface->format->palette;
	if ( palette ) 
	{
	    if(color_type == PNG_COLOR_TYPE_GRAY) 
		{
			palette->ncolors = 256;
			for(i = 0; i < 256; i++) 
			{
			    palette->colors[i].r = i;
			    palette->colors[i].g = i;
			    palette->colors[i].b = i;
			}
	    } else if (info_ptr->num_palette > 0 ) 
		{
			palette->ncolors = info_ptr->num_palette; 
			for( i=0; i<info_ptr->num_palette; ++i ) 
			{
			    palette->colors[i].b = info_ptr->palette[i].blue;
			    palette->colors[i].g = info_ptr->palette[i].green;
			    palette->colors[i].r = info_ptr->palette[i].red;
			}
	    }
	}

done:	/* Clean up and return */
	
	if ( row_pointers ) 
	{
		free(row_pointers);
	}

	int result = 0;
	
	if (surface && dst)
	{
		dst->surf = surface;
		dbgout("MNG_read_frame 0x%p Surf: 0x%p\n", dst, dst->surf);fflush(stdout);

		//copy key over
		if (info_ptr->text && info_ptr->text->key)
		{
			dst->key = (char*)malloc( strlen(info_ptr->text->key) + 1 );
			dst->key = strcpy( dst->key, info_ptr->text->key );
		}
		else
		{
			dst->key = NULL;
		}
		result = 1;
	}
	else if (dst)
	{
		dst->key = NULL;
		dst->surf = NULL;
		result = 0;
	}
	
	png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : (png_infopp)0,
								(png_infopp)0);

	return result; 
}
