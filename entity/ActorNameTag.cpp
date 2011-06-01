
#include "ActorNameTag.h"

uShort timer_updateActorNameTag(timer* t, uLong ms)
{
	ActorNameTag* ant = (ActorNameTag*)t->userData;
	
	if (ant)
	{
		ant->Update();
		return TIMER_CONTINUE;
	}
	
	return TIMER_DESTROY;
}


ActorNameTag::ActorNameTag(Actor* owner)
	: TextObject()
{
	timers->Add("", 10, true, timer_updateActorNameTag, NULL, this);
}

ActorNameTag::~ActorNameTag()
{

}

void ActorNameTag::Render()
{
	if (mOwner && !mOwner->mActiveChatBubble)
	{
		TextObject::Render();
	}
}


void ActorNameTag::Update()
{
	rect r;
	rect rr;
	
	// reposition self relative to owner
	if (mOwner)
	{
		if (mText != mOwner->mName)
		{	
			mText = mOwner->mName;
		}
	
		r = mOwner->GetBoundingRect();
		rr = GetBoundingRect();
		
		r.y -= rr.h + 2;
		r.x += r.w / 2 - (rr.w / 2);

		SetPosition(point2d(r.x, r.y));
	}
}


