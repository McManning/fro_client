
#ifndef _BUBBLEMANAGER_
#define _BUBBLEMANAGER_

#include "../core/Core.h"

class Entity;
class Map;
struct bubble
{
	Entity* owner;
	Image* img;
	uLong killTick;
};

/* Controls map chat bubbles.
	Creates, attaches, deletes, animates, etc.
*/
class BubbleManager
{
  public:
	BubbleManager();
	~BubbleManager();
	
	void Render();
	void Process();
	
	bool CreateBubble(Entity* owner, string msg);
	bool PopBubble(Entity* owner);
	bool HasBubble(Entity* owner);
	
	std::vector<bubble*> mBubbles;
	
	Font* mFont;
	Image* mImage;
	uShort mMaxWidth;
	Map* mMap;
};

#endif //_BUBBLEMANAGER_
