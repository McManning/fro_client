
#ifndef _COLORPICKER_H_
#define _COLORPICKER_H_

#include "Frame.h"

class Input;
class Scrollbar;
class ColorPicker : public Frame 
{
  public:
	ColorPicker(Input* creator = NULL);
	~ColorPicker();
	
	void Render();
	void Update();
	
	Scrollbar* mRed;
	Scrollbar* mGreen;
	Scrollbar* mBlue;
	Input* mResult;
	
	//Will be non null if this widget was created by an input menu
	Input* mCreator;
};

#endif //_COLORPICKER_H_
