
#include "TerrainTest.h"

TerrainTest::TerrainTest()
	: Frame(gui, "TerrainTest", gui->GetScreenPosition())
{
	grid = NULL;
	width = height = 0;
	m_iUpperHeight = 2;
	m_iLowerHeight = 0;
	m_bDisplayTypeMap = true;
	NewGrid(Width() / TILE_SIZE, Height() / TILE_SIZE);
}

TerrainTest::~TerrainTest()
{
	if (grid)
		delete grid;	
}

void TerrainTest::NewGrid(int w, int h)
{
	if (grid)
		delete grid;
	
	width = w;
	height = h;
	
	grid = new tile[width * height];
}

TerrainTest::tile* TerrainTest::GetTile(int x, int y)
{
	if (x >= width || y >= height || x < 0 || y < 0 || !grid)
		return NULL;
	
	return &grid[x + y * width];
}

void TerrainTest::PushUp(int x, int y)
{
	int o = 0;
	for (int i = m_iUpperHeight; i >= 0; i--)
	{
		PaintRange( x-o, y-o, x+4+o, y+4+(o*2), i, false );
		o++;
	}
	Repair();
}

void TerrainTest::PushDown(int x, int y)
{
	int o = 0;
	for (int i = m_iLowerHeight; i <= MAX_HEIGHT; i++)
	{
		PaintRange( x-o, y-2*o, x+4+o, y+4+o, i, true );
		o++;
	}

	Repair();
}

void TerrainTest::PaintRange(int x1, int y1, int x2, int y2, int height, bool paintDown)
{
	tile* t;
	for (int iy = y1; iy <= y2; iy++)
	{
		for (int ix = x1; ix <= x2; ix++)
		{
			t = GetTile(ix, iy);
			if (t && ( (t->height < height && !paintDown) || (t->height > height && paintDown) ))
				t->height = height;
		}
	}
}

/*	Generate a tile type for the specific tile based on surrounding tiles height. */
TerrainTest::tiletype TerrainTest::GetTileType(int x, int y)
{
	tile* t = GetTile(x, y);
	if (!t) return TILE_INVALID;

	//Get height values of surrounding tiles
	int n = -1, s = -1, e = -1, w = -1, se = -1, sw = -1, ne = -1, nw = -1;
	int c = t->height;
	
	tile* t2;
	t2 = GetTile(x, y-1); if (t2) n = t2->height;
	t2 = GetTile(x, y+1); if (t2) s = t2->height;
	t2 = GetTile(x-1, y); if (t2) w = t2->height;
	t2 = GetTile(x+1, y); if (t2) e = t2->height;
	t2 = GetTile(x-1, y-1); if (t2) nw = t2->height;
	t2 = GetTile(x+1, y-1); if (t2) ne = t2->height;
	t2 = GetTile(x-1, y+1); if (t2) sw = t2->height;
	t2 = GetTile(x+1, y+1); if (t2) se = t2->height;
	
	//Use surrounding heights to determine our own type
	if (e == c && w == c && n >= c && s < c && s != -1)
		return TILE_SOUTHEDGE;
	
	if (w >= c && e < c && e != -1 && n == c && s == c)
		return TILE_EASTEDGE;
		
	if (e >= c && w < c && w != -1 && n == c && s == c)
		return TILE_WESTEDGE;
	
	if (e == c && w == c && s >= c && n < c && n != -1)
		return TILE_NORTHEDGE;

	if (e < c && n < c && e != -1 && n != -1 && s >= c && w >= c)
		return TILE_NEEDGE;
	
	if (w < c && n < c && w != -1 && n != -1 && s >= c && e >= c)
		return TILE_NWEDGE;
	
	if (e < c && s < c && e != -1 && s != -1 && n >= c && w >= c)
		return TILE_SEEDGE;
	
	if (w < c && s < c && w != -1 && s != -1 && n >= c && e >= c)
		return TILE_SWEDGE;
	
	//Fix diagonals
	if (n == c && e == c && ne < c && ne != -1)
		return TILE_SWBEND;
		
	if (n == c && w == c && nw < c && nw != -1)
		return TILE_SEBEND;
	
	if (s == c && e == c && se < c && se != -1)
		return TILE_NWBEND;
		
	if (s == c && w == c && sw < c && sw != -1)
		return TILE_NEBEND;

	return TILE_INVALID;
}

/*	Mistakes include:
		-Tile where 3 adjacent tiles have lesser height. -1 its own height.
		-Tile where all 4 adjacents have lesser height. -1 height.
*/
void TerrainTest::_repairTile(int x, int y)
{
	tile* t = GetTile(x, y);
	if (!t) return;

	//Get height values of surrounding tiles
	int n = -1, s = -1, e = -1, w = -1, se = -1, sw = -1, ne = -1, nw = -1;
	int c = t->height;
	
	tile* t2;
	t2 = GetTile(x, y-1); if (t2) n = t2->height;
	t2 = GetTile(x, y+1); if (t2) s = t2->height;
	t2 = GetTile(x-1, y); if (t2) w = t2->height;
	t2 = GetTile(x+1, y); if (t2) e = t2->height;
	t2 = GetTile(x-1, y-1); if (t2) nw = t2->height;
	t2 = GetTile(x+1, y-1); if (t2) ne = t2->height;
	t2 = GetTile(x-1, y+1); if (t2) sw = t2->height;
	t2 = GetTile(x+1, y+1); if (t2) se = t2->height;
	
	int lowcount = 0;
	if (n < c && n != -1) lowcount++;
	if (e < c && e != -1) lowcount++;
	if (s < c && s != -1) lowcount++;
	if (w < c && w != -1) lowcount++;
	
	if (lowcount > 2) //repair island/peninsula error
	{
		if (t->height > 0)
			t->height--;
	}
	
	//check for bridges
	if (n >= c && s >= c && e < c && w < c)
	{
		if (t->height > 0)
			t->height--;
	}
	
	if (e >= c && w >= c && n < c && s < c)
	{
		if (t->height > 0)
			t->height--;
	}
}
/*
void TerrainTest::_recalculateType(int x, int y)
{
	tile* t = GetTile(x, y);
	tile* t2;
	if (t)
		t->type = type;
	
	if (t->type == TILE_NORTHEDGE)
	{
		
	}
}
*/
void TerrainTest::Repair()
{
	tiletype type;
	tile* t;
	
	//fix invalid types
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{	
			_repairTile(x, y);	
		}	
	}
	
	//set types
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{	
			t = GetTile(x, y);
			type = GetTileType(x, y);
			if (t)
				t->type = type;
		}		
	}
	
	//after all types are set, go again and reload
/*	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{	
			_recalculateType(x, y);
		}		
	}*/
}

void TerrainTest::Render(uLong ms)
{
	Image* scr = Screen::Instance();
	if (!grid)
		return;
	
	color c;
	tile* t;
	tile* tt;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			t = GetTile(x, y);
			c.r = c.g = c.b = t->height * 25;
			
			scr->DrawRect( rect(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE), c, true);

			if (m_bDisplayTypeMap)
			{
				//Mark tile types
				c = color();
				switch (GetTileType(x, y))
				{
					case TILE_SOUTHEDGE:
						c.r = 255; 
						break;
					case TILE_WESTEDGE:
						c.g = 255;
						break;
					case TILE_EASTEDGE:
						c.b = 255;
						break;
					case TILE_NORTHEDGE:
						c.r = c.g = 255;
						break;
					case TILE_SWEDGE:
						c.r = 128;
						break;
					case TILE_SEEDGE:
						c.g = 128;
						break;
					case TILE_NWEDGE:
						c.b = 128;
						break;
					case TILE_NEEDGE:
						c.r = c.g = 128;
						break;
					case TILE_SWBEND:
						c.g = c.b = 255;
						break;
					case TILE_SEBEND:
						c.g = c.b = 128;
						break;
					case TILE_NEBEND:
						c.r = c.b = 255;
						break;
					case TILE_NWBEND:
						c.r = c.b = 128;
						break;
					default: break;	
				}
			
				if (!isDefaultColor(c))
					scr->DrawRound( x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE/2, c);	
			}
			
			//Render edge lines for all tiles that have too drastic a height difference between them
			tt = GetTile(x, y-1);
			if (tt)
			{
				if (tt->height > t->height+1)
					scr->DrawLine(x*TILE_SIZE, y*TILE_SIZE, x*TILE_SIZE+16, y*TILE_SIZE, color(255), 2);
				else if (tt->height+1 < t->height)
					scr->DrawLine(x*TILE_SIZE, y*TILE_SIZE, x*TILE_SIZE+16, y*TILE_SIZE, color(0,255), 2);
			}
			
			tt = GetTile(x, y+1);
			if (tt)
			{
				if (tt->height > t->height+1)
					scr->DrawLine(x*TILE_SIZE, y*TILE_SIZE+14, x*TILE_SIZE+16, y*TILE_SIZE+14, color(255), 2);
				else if (tt->height+1 < t->height)
					scr->DrawLine(x*TILE_SIZE, y*TILE_SIZE+14, x*TILE_SIZE+16, y*TILE_SIZE+14, color(0,255), 2);
			}
			
			tt = GetTile(x+1, y);
			if (tt)
			{
				if (tt->height > t->height+1)
					scr->DrawLine(x*TILE_SIZE+14, y*TILE_SIZE, x*TILE_SIZE+14, y*TILE_SIZE+14, color(255), 2);
				else if (tt->height+1 < t->height)
					scr->DrawLine(x*TILE_SIZE+14, y*TILE_SIZE, x*TILE_SIZE+14, y*TILE_SIZE+14, color(0,255), 2);
			}
			
			tt = GetTile(x-1, y);
			if (tt)
			{
				if (tt->height > t->height+1)
					scr->DrawLine(x*TILE_SIZE, y*TILE_SIZE, x*TILE_SIZE, y*TILE_SIZE+14, color(255), 2);
				else if (tt->height+1 < t->height)
					scr->DrawLine(x*TILE_SIZE, y*TILE_SIZE, x*TILE_SIZE, y*TILE_SIZE+14, color(0,255), 2);	
			}
			
			
		}
	}
	
	string msg = "m_bDisplayTypeMap: " + its(m_bDisplayTypeMap) + " (T) \\c900 m_iUpperHeight: " + its(m_iUpperHeight) + " (+Q -A). \\c090 m_iLowerHeight: " 
				+  its(m_iLowerHeight) + " (+W -S). \\c009 N to clear all";
	gui->mFont->Render(scr, 5, 5, msg, color(255,255,255));
	
	msg = "\\c090Green \\c999and \\c900red \\c999lines mark gradients too steep (Algorithm failure).";
	gui->mFont->Render(scr, 5, scr->Height() - 20, msg, color());
}

void TerrainTest::Event(SDL_Event* event)
{
	switch (event->type)
	{
		case SDL_KEYUP:
		{
			switch (event->key.keysym.sym)
			{
				case SDLK_n:
					NewGrid(Width() / TILE_SIZE, Height() / TILE_SIZE);
					break;	
				case SDLK_q:
					if (m_iUpperHeight < MAX_HEIGHT)
						m_iUpperHeight++;
					break;
				case SDLK_a:
					if (m_iUpperHeight > 0)
						m_iUpperHeight--;
					break;
				case SDLK_w:
					if (m_iLowerHeight < MAX_HEIGHT)
						m_iLowerHeight++;
					break;
				case SDLK_s:
					if (m_iLowerHeight > 0)
						m_iLowerHeight--;
					break;
				case SDLK_t:
					m_bDisplayTypeMap = !m_bDisplayTypeMap;
					break;
				default: break;
			}
		} break;
		case SDL_MOUSEMOTION:
		{
			int x, y;
			if (gui->IsMouseButtonDown(MOUSE_BUTTON_LEFT, &x, &y))
			{
				PushUp(x / TILE_SIZE, y / TILE_SIZE);
			}
			else if (gui->IsMouseButtonDown(MOUSE_BUTTON_RIGHT, &x, &y))
			{
				PushDown(x / TILE_SIZE, y / TILE_SIZE);
			}
		} break;
		default: break;
	}
}

