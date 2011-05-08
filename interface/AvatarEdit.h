
#ifndef _AVATAREDIT_H_
#define _AVATAREDIT_H_

#include "../core/widgets/Frame.h"

class Label;
class AvatarProperties;
class AvatarEdit : public Frame 
{
  public:
	AvatarEdit(int workingIndex = 0, AvatarProperties* props = NULL);
	~AvatarEdit();
	
	Label* mAlertLabel;
	
	int mWorkingIndex;
};

#endif //_AVATAREDIT_H_
