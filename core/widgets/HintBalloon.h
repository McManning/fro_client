
#ifndef _HINTBALLOON_H_
#define _HINTBALLOON_H_

#include "Widget.h"

class HintBalloon: public Widget 
{
  public:
	HintBalloon(Widget* wParent);
	~HintBalloon();

	void Render();
	
	string GetCaption() { return mCaption; };
	void SetCaption(string text);

	uShort mMaxWidth; //0 = unlimited

  protected:
	string mCaption; 
};

#endif //_HINTBALLOON_H_
