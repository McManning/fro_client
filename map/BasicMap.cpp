
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

			/*if (mShowInfo)
			{
				//Render collision rects
				for (uShort u = 0; u < e->mCollisionRects.size(); u++)
				{
					//Render red for solid entities, pink for passable
					if (e->IsSolid())
						c = color(255,0,0);
					else
						c = color(255,0,255);
						
					r.x = e->mCollisionRects.at(u).x + e->mPosition.x - e->mOrigin.x;
					r.y = e->mCollisionRects.at(u).y + e->mPosition.y - e->mOrigin.y;
					r.w = e->mCollisionRects.at(u).w;
					r.h = e->mCollisionRects.at(u).h;
					scr->DrawRect( ToScreenPosition(r), c, false );
				}
				
				//Render bounding rect
				r = ToScreenPosition( e->GetBoundingRect() );	
				scr->DrawRect(r, color(0,255,0), false);
					
				//Render origin marker
				r.x = r.x + e->mOrigin.x - 2;
				r.y = r.y + e->mOrigin.y - 2;
				r.w = 5;
				r.h = 5;
				scr->DrawRound(r, 2, color(0,0,255));
							
				//Render position marker
				r.x = e->mPosition.x - 2;
				r.y = e->mPosition.y - 2;
				r.w = 5;
				r.h = 5;
				r = ToScreenPosition(r);
				scr->DrawRound(r, 2, color(255,255,0));
				
				//Render destination marker & line, if actor, and we're moving
				if (e->mType >= ENTITY_ACTOR && e->mType < ENTITY_END_ACTORS)
				{
					r.x = ( (Actor*)e )->GetDestination().x;
					r.y = ( (Actor*)e )->GetDestination().y;
					rr.x = e->mPosition.x;
					rr.y = e->mPosition.y;

					if (r.x != rr.x || r.y != rr.y)
					{
						r.x -= 2;
						r.y -= 2;
						r.w = 5;
						r.h = 5;
						r = ToScreenPosition(r);
						rr = ToScreenPosition(rr);
						
						scr->DrawLine(rr.x, rr.y, r.x + 2, r.y + 2, color(0,255,255), 1);
						scr->DrawRound(r, 2, color(0,255,255));
					}
				}
			} // if show info */
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
