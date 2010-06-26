
#include "BasicMap.h"
#include "../game/GameManager.h"
#include "../core/widgets/Console.h"

TimeProfiler mapRenderProfiler("BasicMap::Render");
TimeProfiler mapProcessProfiler("BasicMap::Process");

TimeProfiler mapEntityRenderProfiler("BasicMap::_renderEntities");
TimeProfiler mapBaseRenderProfiler("Map::Render");

BasicMap::BasicMap()
	: Map()
{
	mType = BASIC;
}

BasicMap::~BasicMap()
{
	PRINT("BasicMap::~BasicMap");
}

void BasicMap::Render()
{
	mapProcessProfiler.Start();
	Process(); //Here until we have a timer for it
	mapProcessProfiler.Stop();
	
	mapRenderProfiler.Start();
	
	Image* scr = Screen::Instance();
	rect r = GetScreenPosition();
	
	scr->SetClip(r);
	
	//fill with background color
	scr->DrawRect(r, mBackground);
	
	mapEntityRenderProfiler.Start();
	_renderEntities();
	mapEntityRenderProfiler.Stop();

	mapBaseRenderProfiler.Start();
	Map::Render();
	mapBaseRenderProfiler.Stop();
		
	scr->SetClip();
	
	mapRenderProfiler.Stop();
}

TimeProfiler mapEntitesProfiler1("::_renderEntities::Render");

void BasicMap::_renderEntities()
{
	Image* scr = Screen::Instance();

	rect r, rr;
	Entity* e;
	color c;
	
	int i;
	for (i = 0; i < mEntities.size(); ++i)
	{
		e = mEntities.at(i);
		
		if (e && e->IsVisibleInCamera())
		{
			mapEntitesProfiler1.Start();
			e->Render(); //draw entity
			mapEntitesProfiler1.Stop();
			
		} //if visible & in camera
	} //for all entities
}

bool BasicMap::IsRectBlocked(rect r)
{
	Entity* e;
	for (int i = 0; i < mEntities.size(); ++i)
	{
		e = mEntities.at(i);
		if ( e && e->IsSolid() && e->CollidesWith(r) && e->mType != ENTITY_LOCALACTOR )
			return true;
	} 
	return false;
}
