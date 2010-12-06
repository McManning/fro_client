
#include "ActorStats.h"
#include "../core/widgets/RightClickMenu.h"
#include "../entity/Actor.h"
#include "../map/Map.h"
#include "../game/GameManager.h"
//#include "../interface/LunemParty.h"

const int HEALTH_SLIDE_MS = 10;

void callback_ActorStatsRCM_Info(RightClickMenu* m, void* data)
{
	ActorStats* as = (ActorStats*)data;
}

void callback_ActorStatsRCM_DuelSwap(RightClickMenu* m, void* data)
{
	ActorStats* as = (ActorStats*)data;

	ASSERT(game && game->mParty && as);
	game->mParty->SendDuelSwapEvent(as);
}

void callback_ActorStatsRCM_UseItem(RightClickMenu* m, void* data)
{
	ActorStats* as = (ActorStats*)data;

	ASSERT(game && game->mParty && as);
	game->mParty->SendUseItemEvent(as);
}

uShort timer_ActorStatsThink(timer* t, uLong ms)
{
	ActorStats* s = (ActorStats*)t->userData;
	
	if (!s)
		return TIMER_DESTROY;
		
	// if our health is going down, slide down
	if (s->mCurrentHealth > s->mLinkedActor->m_iCurrentHealth)
	{
		s->mCurrentHealth--;
	}
	
	return TIMER_CONTINUE;
}

ActorStats::ActorStats(Widget* parent)
{
	mImage = resman->LoadImg("assets/stats_bar.png");
	mFont = fonts->Get();
	mSmallFont = fonts->Get("", 10);
	mMenuMode = NO_MENU;

	SetLinked(NULL);
	
	SetSize(200, 50);
	
	if (parent)
		parent->Add(this);
}

ActorStats::~ActorStats()
{
	timers->RemoveMatchingUserData(this);
}

void ActorStats::SetLinked(Actor* a)
{
	mLinkedActor = a;

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

	// Adjust immediately if we're not slowly sliding
	if (mMenuMode != DUEL_SCREEN_MENU)
	{
		mCurrentHealth = mLinkedActor->m_iCurrentHealth;
	}

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
	
	msg = "Lv." + its(mLinkedActor->m_bLevel);
	
	// Draw level (same position, right aligned)
	mSmallFont->Render(scr, r.x + 5, 
						healthBarPos.y + healthBarPos.h + 3, 
						msg, 
						color());

}

void ActorStats::SetMenuMode(int mode)
{
	mMenuMode = mode;
	
	if (mode == DUEL_SCREEN_MENU)
	{
		// add a timer for animating the health bar
		timers->Add("", 50, true, timer_ActorStatsThink, NULL, this);	
	}
}

void ActorStats::Event(SDL_Event* event)
{
	if (event->type == SDL_MOUSEBUTTONDOWN)
	{
		if (event->button.button == SDL_BUTTON_RIGHT)
			CreateMenu();
	}
		
	Widget::Event(event);	
}

void ActorStats::CreateMenu()
{
	if (mMenuMode == NO_MENU || !mLinkedActor)
		return;
		
	RightClickMenu* m = new RightClickMenu();
	
		// We can always look up info
		m->AddOption("Info", callback_ActorStatsRCM_Info, this);
	
	switch (mMenuMode)
	{
		case DUEL_SCREEN_MENU:
			// What to add besides Info?
			break;
		case PARTY_VIEW_MENU:
			// What to add besides Info?
			break;
		case DUEL_SWAP_MENU:
			if (mLinkedActor->m_iCurrentHealth > 0)
			{
				m->AddOption("Summon", callback_ActorStatsRCM_DuelSwap, this);
			}
			break;
		case USE_ITEM_MENU:
			m->AddOption("Use Item", callback_ActorStatsRCM_UseItem, this);
			break;
		default: break; 
	}
}





