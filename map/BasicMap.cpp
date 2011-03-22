
#include "BasicMap.h"
#include "../game/GameManager.h"
#include "../core/widgets/Console.h"

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
	Image* scr = Screen::Instance();
	rect r = GetScreenPosition();
	
	scr->SetClip(r);
	
	//fill with background color
	scr->DrawRect(r, mBackground);

	_renderEntities();

	Map::Render();
		
	scr->SetClip();
}

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
			e->Render();
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
