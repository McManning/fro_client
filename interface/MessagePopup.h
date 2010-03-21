
#ifndef _MESSAGEPOPUP_H_
#define _MESSAGEPOPUP_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

#define MESSAGEPOPUP_MAX_WIDTH 400
#define MESSAGEPOPUP_MIN_WIDTH 150

class MessagePopup : public Frame 
{
  public:
	MessagePopup(string id, string title, string msg);
	~MessagePopup();
	
	void Render(uLong ms);
	
	//void ResizeChildren();

	//Multiline* mMessage;
	Button* mOkay;
};

#endif //_MESSAGEPOPUP_H_
