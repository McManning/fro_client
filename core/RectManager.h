
/*
 * This class controls how everything is updated on screen
 * Initial design by Eduardo B. Fonseca <ebf@drmsolucoes.com.br>
 * With lots of code from Paul H. Liu <paul@theV.net>
 * SDL adaption by Chase McManning
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef _RECTMANAGER_H_
#define _RECTMANAGER_H_

#include <SDL/SDL.h>
#include "Common.h"

/****************************************************************************************/
// WARNING: This all has to be converted to template-using functions.
// THIS IS ALL TEMPORARY

//typedef struct Rect_Set Rect_Set;
typedef struct Rect_Clips {
    int length;
    SDL_Rect rects;

} Rect_Clips;

#define _NOT_FOUND_ERR	-1
#define _NO_ERR		0

#define RD(value, mask) ((value) & ~(mask))     // round down a value
#define RU(value, mask)	(((value) + (mask)) & ~(mask))  // round up a value

// range_type is basically a dynamic sorted (ascending order) array that
// grows exponentially. It is used to implement grid X/Y coordinates, and to
// represent marked cells.
typedef struct _range_type {
    int length, size;           // length refers to array, size to malloc
    int *points;
} range_type;

// a RectangleSet is just a grid of X/Y coordinates and marked cells.
struct Rect_Set {
    range_type xs, ys, ms;      // the grid and marked cells.

    range_type xsum, ysum;      // this is used as a hash value of row's x
    // coordinates, and column's y coordinates

    Uint16 bits_mask;           // how many bits to mask off the X/Y values.
    // This is to improve efficiency by compromising
    // accuracy over large scale grids.
};


class RectManager {
  public:
	RectManager();
    ~RectManager();

    void add_rect(SDL_Rect r);
    bool purge(void);
	
	bool generate_clips(SDL_Surface* surf);
    bool update_rects(SDL_Surface * surf);

    // TODO remove this all and convert this all to template-using functions
    Rect_Set *new_rects(int bits);
    void free_rects(Rect_Set * rects);
    void clear_rects(Rect_Set * rects);
    void rects_union(Rect_Set * rects, Sint16 x, Sint16 y, Uint16 w, Uint16 h);
    void rects_subtract(Rect_Set * rects, Sint16 x, Sint16 y, Uint16 w, Uint16 h);
    void rects_intersect(Rect_Set * rects, Sint16 x, Sint16 y, Uint16 w, Uint16 h);
    int rects_intersects(Rect_Set * rects, Sint16 x, Sint16 y, Uint16 w, Uint16 h);
    Rect_Clips *rects_clip(Rect_Set * rs, SDL_Rect * r);
    void rects_print(Rect_Set * rs);
    int range_search(range_type * range, int value);
    void range_inserti(range_type * range, int pos, int value);
    int range_insert(range_type * range, int value, int *index);
    void range_removei(range_type * range, int pos);
    int range_remove(range_type * range, int value);
    int v_line_insert(Rect_Set * rs, int v);
    void v_line_remove(Rect_Set * rs, int v);
    int h_line_insert(Rect_Set * rs, int v);
    void h_line_remove(Rect_Set * rs, int v);
    int v_concentrate(Rect_Set * rs);
    int h_concentrate(Rect_Set * rs);
    void concentrate(Rect_Set * rs);

    // Convert this all to native, template-using functions
    Rect_Clips *m_Clips;
    Rect_Set *m_RectSet;
};

#endif // _RECTMANAGER_H_
