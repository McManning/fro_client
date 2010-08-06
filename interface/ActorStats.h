
#ifndef _ACTORSTATS_H_
#define _ACTORSTATS_H_

#include "../core/widgets/Widget.h"

class Actor;
class ActorStats : public Widget
{
  public:
	ActorStats(Widget* parent);
	~ActorStats();
	
	void SetLinked(Actor* a);
	
	void Render();
	void RenderHealth(Image* scr, rect& r);
	
	void Event(SDL_Event* event);
	
	enum // menu modes
	{
		NO_MENU = 0,
		DUEL_SCREEN_MENU, 
		PARTY_VIEW_MENU,
		DUEL_SWAP_MENU,
		USE_ITEM_MENU,
	};
	
	void SetMenuMode(int mode);
	
	/* Called by Event() when right clicked */
	void CreateMenu(); 
	
	int mMenuMode;

	Actor* mLinkedActor;
	int mCurrentHealth;
	
	Font* mSmallFont;

};

#endif //_ACTORSTATS_H_


