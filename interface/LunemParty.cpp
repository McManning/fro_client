
#include "LunemParty.h"

LunemParty::LunemParty() :
	Frame(gui, "party", rect(0,0,310,35+(65*5)), "My Party", true, false, true, true)
{
	rect r(6, 30, 200, 50);
	for (int i = 0; i < 5; ++i)
	{
		mInfoBar[i] = new ActorStats(this);
		mInfoBar[i]->SetLinked(NULL);
		mInfoBar[i]->SetPosition( r );
		
		if (r.x < 15)
			r.x = 15;
		else
			r.x = 10;
		
		r.y += 55;
	}
	
	Center();
}

