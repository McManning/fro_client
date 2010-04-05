
#ifndef _YESNOPOPUP_H_
#define _YESNOPOPUP_H_

#include "Frame.h"

#define YESNOPOPUP_MAX_WIDTH 400
#define YESNOPOPUP_MIN_WIDTH 150

class YesNoPopup : public Frame 
{
  public:
	YesNoPopup(string id, string title, string msg, bool useMultiline = false);
	~YesNoPopup();
	
	void Render(uLong ms);

	Button* mYes;
	Button* mNo;

	void (*onYesCallback)(YesNoPopup*);
	void (*onNoCallback)(YesNoPopup*);
	void* userdata;
};

#endif //_YESNOPOPUP_H_
