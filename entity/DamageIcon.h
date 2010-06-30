
#ifndef _DAMAGEICON_H_
#define _DAMAGEICON_H_

#include "StaticObject.h"

class DamageIcon : public StaticObject
{
  public:
	DamageIcon();
	~DamageIcon();
	
	void Render();

	std::vector<int> mBorders;
	Image* mFontImage;
};

#endif //_DAMAGEICON_H_
