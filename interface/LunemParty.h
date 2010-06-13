
#ifndef _LUNEMPARTY_H_
#define _LUNEMPARTY_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

class LunemInfoBar;
class LunemParty : public Frame
{
  public:
	LunemParty();
	
	LunemInfoBar* mInfoBar[5];
};

#endif //_LUNEMPARTY_H_
