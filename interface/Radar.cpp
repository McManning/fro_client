
#include "Radar.h"
#include "../core/widgets/Button.h"
#include "../game/GameManager.h"
#include "../map/Map.h"
#include "../entity/LocalActor.h"

/*
	Initial size:
	Radar itself: 200x200
	30 pixels at bottom for button bar.
	2  Zoom In/Out buttons.
	
	Radar at:
	rect(5, 30, 200, 200) (initially)
	buttons at:
	5, 235 & 35, 235
	Size:
	235+25 = 260 height
	200+10 = 210 width
*/


void callback_radarZoomIn(Button* b)
{
	Radar* r = (Radar*)b->GetParent();
	r->ZoomIn();
}

void callback_radarZoomOut(Button* b)
{
	Radar* r = (Radar*)b->GetParent();
	r->ZoomOut();
}

Radar::Radar() :
	Frame(gui, "radar", rect(), "Radar", true, true, true, true)
{
	mZoomIn = new Button(this, "in",rect(0,0,20,20), "", callback_radarZoomIn);
		mZoomIn->mHoverText = "Zoom In";
		makeImage(mZoomIn, "", "assets/radar.png", rect(0,0,20,20),
				rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
				
	mZoomOut = new Button(this, "out", rect(0,0,20,20), "", callback_radarZoomOut);
		mZoomOut->mHoverText = "Zoom Out";
		makeImage(mZoomOut, "", "assets/radar.png", rect(20,0,20,20),
				rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
		
	SetSize(210, 235);
	Center();
	ResizeChildren();
	
	mZoom = 3;
}

Radar::~Radar()
{
	
}

void Radar::ZoomIn()
{
	if (mZoom > 1)
		mZoom--;
}

void Radar::ZoomOut()
{
	if (mZoom < 20)
		mZoom++;
}

void Radar::Render(uLong ms)
{
	Frame::Render(ms);
	
	if (mResizing) 
		return;
	
	Image* scr = Screen::Instance();
	rect r = GetScreenPosition();
	
	r.x += 5;
	r.y += 30;
	r.w -= 10;
	r.h -= 60;

	scr->DrawRect(r, color());
	
	if (game->mMap)
	{	
		rect clip = scr->GetClip();
		scr->SetClip(r);
		
		Entity* e;
		for (sShort i = game->mMap->entityLevel[ENTITYLEVEL_USER].size() - 1; i >= 0; i--)
		{
			e = game->mMap->entityLevel[ENTITYLEVEL_USER].at(i);
			if ( e && (e->mType == ENTITY_REMOTEACTOR || e->mType == ENTITY_LOCALACTOR) )
			{
				_renderEntityOnRadar(e);
			}
		}
		
		scr->SetClip(clip);
	}
	else
	{
		mFont->Render(scr, r.x, r.y, "No Map", color(255,0,0));	
	}
	
	mFont->Render(scr, r.x, r.y + r.h - 20, its(100 / mZoom) + "% Zoom", color(255,0,0));	
}

void Radar::_renderEntityOnRadar(Entity* e)
{
	point2d p = e->GetPosition();
	point2d pp = game->mPlayer->GetPosition();
	
	p.x -= pp.x;
	p.y -= pp.y;
	
	p.x /= mZoom;
	p.y /= mZoom;
	
	rect r = GetScreenPosition();
	
	p.x += r.x + (r.w / 2);
	p.y += r.y + (r.h / 2); //wrong calculation, fix!
	
	sShort iconsize = 16 / mZoom;
	if (iconsize < 2)
		iconsize = 2;
		
	p.x -= iconsize / 2;
	p.y -= iconsize / 2;
		
	Image* scr = Screen::Instance();
	scr->DrawRect(rect(p.x, p.y, iconsize, iconsize), color(255, 0, 0), false);
}

void Radar::ResizeChildren()
{
	mZoomIn->SetPosition( rect(5, Height() - 25, 20, 20) );
	mZoomOut->SetPosition( rect(30, Height() - 25, 20, 20) );
	
	Frame::ResizeChildren();
}

void Radar::SetPosition(rect r)
{
	if (r.w >= 110 && r.h >= 135)
		Frame::SetPosition(r);
}


