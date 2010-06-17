
#ifndef _PLAYERACTIONMENU_H_
#define _PLAYERACTIONMENU_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

class Actor;
class PlayerActionMenu : public Frame
{
  public:
	PlayerActionMenu(int timeout, Actor* controlled);
	~PlayerActionMenu();
	
	Actor* m_pControlled;
	int m_iCountdown;
	
	Frame* m_pSkillsFrame;
	Frame* m_pChoicesFrame;
};

#endif //_PLAYERACTIONMENU_H_
