
#ifndef _LUNEMINFOBAR_H_
#define _LUNEMINFOBAR_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

class Lunem //temp
{
  public:
	string mNickname;
	int mLevel;
};

class Label;
class LunemInfoBar : public Frame
{
  public:
	LunemInfoBar(Widget* pParent, rect rPosition, bool bTop);
	~LunemInfoBar() {};

	void Render() { Frame::Render(); };
	void Event(SDL_Event* event) { Frame::Event(event); }; 
	
	void SetLunem(Lunem* lu);
	
	Lunem* mLunem; //who's info we're displaying
	
	Label* mEmptyCaption;
	Label* mLunemName;
	Label* mLunemLevel;
};

#endif //_LUNEMINFOBAR_H_
