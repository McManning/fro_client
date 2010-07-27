
#ifndef _CHATBUBBLE_H_
#define _CHATBUBBLE_H_

#include "../core/Core.h"
#include "Entity.h"

class ChatBubble : public Entity
{
  public:
	ChatBubble(Entity* owner, string& msg);
	~ChatBubble();
	
	void Render();
	rect GetBoundingRect();
	
	void Create(string& msg);
	
	Image* mImage;
	Entity* mOwner;
};

#endif //_CHATBUBBLE_H_
