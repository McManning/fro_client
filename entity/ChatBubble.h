
#ifndef _CHATBUBBLE_H_
#define _CHATBUBBLE_H_

#include "../core/Core.h"
#include "Entity.h"

class ChatBubble : public Entity
{
  public:
	ChatBubble(Entity* owner, string& msg);
	ChatBubble(Entity* owner, int emote);
	
	~ChatBubble();
	
	void Render();
	rect GetBoundingRect();
	void UpdatePosition();
	
	void CreateText(string& msg);
	void CreateEmote(int id);
	
	Image* mImage;
	Entity* mOwner;
};

#endif //_CHATBUBBLE_H_
