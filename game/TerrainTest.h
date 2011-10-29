
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


#ifndef _TERRAINTEST_H_
#define _TERRAINTEST_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

const int TILE_SIZE = 16;
const int MAX_HEIGHT = 10;

class TerrainTest : public Frame 
{
  public:
	TerrainTest();
	~TerrainTest();
	
	enum tiletype
	{
		TILE_INVALID = 0,
		TILE_CHILD,
		TILE_NORTHEDGERENDERPOS,
		TILE_SOUTHEDGE,
		TILE_EASTEDGE,
		TILE_WESTEDGE,
		TILE_NORTHEDGE,
		TILE_NWEDGE,
		TILE_NEEDGE,
		TILE_SWEDGE,
		TILE_SEEDGE,
		TILE_NWBEND,
		TILE_NEBEND,
		TILE_SWBEND,
		TILE_SEBEND,
	};
	
	struct tile
	{
		tile()
		{
			height = 1;	
			type = TILE_INVALID;
		};
		int height;
		tiletype type; 
	};

	void Render(); 
	void Event(SDL_Event* event); 

	void NewGrid(int w, int h);
	tile* GetTile(int x, int y);
	
	void PushUp(int x, int y);
	void PushDown(int x, int y);
	
	void PaintRange(int x1, int y1, int x2, int y2, int height, bool paintDown);
	
	tiletype GetTileType(int x, int y);
	
	void _repairTile(int x, int y);
	void Repair();
	
	int width;
	int height;
	
	int m_iUpperHeight;
	int m_iLowerHeight;
	bool m_bDisplayTypeMap;
  private:
	tile* grid;
};

#endif //_TERRAINTEST_H_
