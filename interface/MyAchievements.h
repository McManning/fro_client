
#ifndef _MYACHIEVEMENTS_H_
#define _MYACHIEVEMENTS_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "../core/widgets/Scrollbar.h"

class MyAchievements : public Frame
{
  public:
	MyAchievements();
	~MyAchievements();
	
	void Render();
	void SetPosition(rect r);
	void ResizeChildren();
	
	struct achievement
	{
		string title;
		string description;
		string file;
		int total;
		int max;
		Image* icon; //if this isn't local, it'll be downloaded on the fly
	};

	std::vector<achievement> mAchievements;

	Frame* mAchFrame;
	Scrollbar* mScroller;
	Image* mDefaultIcon;

  private:
	void _load();
	uShort _renderSingle(rect r, uShort index);
};

#endif //_MYACHIEVEMENTS_H_
