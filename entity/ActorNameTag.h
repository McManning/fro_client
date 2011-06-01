
#ifndef _ACTORNAMETAG_H_
#define _ACTORNAMETAG_H_

#include "TextObject.h"

class ActorNameTag : public TextObject
{
  public:
	ActorNameTag(Actor* owner);
	~ActorNameTag();

	void Update();
	
	Actor* mOwner;
};

#endif //_ACTORNAMETAG_H_
