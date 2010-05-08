
#ifndef _MESSAGEPOPUP_H_
#define _MESSAGEPOPUP_H_

#include "Frame.h"

#define MESSAGEPOPUP_MAX_WIDTH 400
#define MESSAGEPOPUP_MIN_WIDTH 150

class MessagePopup : public Frame 
{
  public:
	MessagePopup(string id, string title, string msg, bool useMultiline = false);
	~MessagePopup();
	
	void Render();
	
	//void ResizeChildren();

	//Multiline* mMessage;
	Button* mOkay;

	void (*onCloseCallback)(MessagePopup*);
	void* userdata;
};

#endif //_MESSAGEPOPUP_H_
