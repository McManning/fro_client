
#include "LunemParty.h"
#include "LunemInfoBar.h"

LunemParty::LunemParty() :
	Frame(gui, "party", rect(0,0,260,35+(55*5)), "My Party", true, false, true, true)
{
	int x = 10, y = 30;
	for (int i = 0; i < 5; ++i)
	{
		mInfoBar[i] = new LunemInfoBar(this, rect(x, y, 200, 50), (i == 0));
		mInfoBar[i]->SetLunem(NULL);
		
		if (x < 50)
			x = 50;
		else
			x = 25;
		
		y += 55;
	}
	
	Center();
}

