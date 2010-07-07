
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

	Actor* mLinkedActor;
	int mCurrentHealth;
	
	Font* mSmallFont;
	uLong mHealthSlideMs;
};

#endif //_ACTORSTATS_H_


