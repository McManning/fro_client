
#include "ActorStats.h"
#include "../entity/Actor.h"
#include "../map/Map.h"

const int HEALTH_SLIDE_MS = 10;

ActorStats::ActorStats(Widget* parent)
{
	mImage = resman->LoadImg("assets/stats_bar.png");
	mFont = fonts->Get();
	mSmallFont = fonts->Get("", 10);

	SetLinked(NULL);
	
	SetSize(200, 50);
	
	if (parent)
		parent->Add(this);
}

ActorStats::~ActorStats()
{

}

void ActorStats::SetLinked(Actor* a)
{
	mLinkedActor = a;
	mHealthSlideMs = 0;
	if (a)
		mCurrentHealth = a->m_iCurrentHealth;
	else
		mCurrentHealth = 0;
}

void ActorStats::Render()
{
	Image* scr = Screen::Instance();
	rect r;

	r = GetScreenPosition();
	
	rect oldclip = scr->GetClip();
	scr->SetClip(r);

	if (mLinkedActor)
	{
		//draw background
		mImage->Render(scr, r.x, r.y, rect(0,0,r.w,r.h));
	
		//draw health stats
		RenderHealth(scr, r);
		
		// draw name
		mFont->Render(scr, r.x + 5, r.y + 4, mLinkedActor->mName, color());
		
		//draw icons for type
	}
	else
	{
		//draw background
		mImage->Render(scr, r.x, r.y, rect(0,r.h,r.w,r.h));
	}
	
	scr->SetClip(oldclip);
}

void ActorStats::RenderHealth(Image* scr, rect& r)
{
	rect healthBarPos(r.x + 6, r.y + 22, r.w - 12, 10);
	rect sr(200, 0, 2, 10);

	// draw health bar fg: width = healthBarPos.w * (cur hp / max hp)
	double d = (double)mCurrentHealth / (double)mLinkedActor->m_iMaxHealth;
	healthBarPos.w = (int)((double)healthBarPos.w * d);

	//change source rect to green, yellow, or red bar based on how low current  health is
	if (d < 0.25) //red
		sr.y = 20;
	else if (d < 0.5) //yellow
		sr.y = 10;
	else //green
		sr.y = 0;

	if (healthBarPos.w > 0)
		mImage->RenderHorizontalEdge(scr, sr, healthBarPos);
	
	string msg = its(mCurrentHealth) + " / " + its(mLinkedActor->m_iMaxHealth);
	
	// draw health text (cur hp / max hp) left aligned
	mSmallFont->Render(scr, r.x + r.w - mSmallFont->GetWidth(msg) - 5, 
						healthBarPos.y + healthBarPos.h + 3, 
						msg, 
						color());
	
	msg = "Lv." + its(mLinkedActor->m_iLevel);
	
	// Draw level (same position, right aligned)
	mSmallFont->Render(scr, r.x + 5, 
						healthBarPos.y + healthBarPos.h + 3, 
						msg, 
						color());
	
	//if our health is going down, slide down
	if (mCurrentHealth > mLinkedActor->m_iCurrentHealth)
	{
		if (mHealthSlideMs < SDL_GetTicks())
		{
			--mCurrentHealth;
			mHealthSlideMs = SDL_GetTicks() + HEALTH_SLIDE_MS;
		}
	}
}


