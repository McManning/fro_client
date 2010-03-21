

#ifndef _AVATARVIEWER_H_
#define _AVATARVIEWER_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

class SmallSelect;
class Input;
class Avatar;
class AvatarViewer : public Frame 
{
  public:
	AvatarViewer();
	~AvatarViewer();
	
	void Render(uLong ms);
	
	void Load();
	void Reload();
	void ChangePosition();
	
	void LoadFile(string file);

  private:
	Input* mAvyWidth;
	Input* mAvyHeight;
	Input* mAvyDelay;
	SmallSelect* mAvyPositions;
	Avatar* mAvatar;
};

#endif //_AVATARVIEWER_H_
