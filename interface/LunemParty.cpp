
#include "LunemParty.h"

LunemParty::LunemParty() :
	Frame(gui, "party", rect(0,0,310,35+(65*5)), "My Party", true, false, true, true)
{
	rect r(10, 30, 250, 60);
	for (int i = 0; i < 5; ++i)
	{
		mInfoBar[i] = new ActorStats(this);
		mInfoBar[i]->SetLinked(NULL);
		mInfoBar[i]->SetPosition( r );
		
		if (r.x < 50)
			r.x = 50;
		else
			r.x = 25;
		
		r.y += 65;
	}
	
	Center();
}

