
#ifndef _CHECKBOX_H_
#define _CHECKBOX_H_

#include "Widget.h"

//For simple on/off checkboxes
#define UNCHECKED 0
#define CHECKED 1

const int CHECKBOX_SIZE = 16;

class Font;
class Image;
/*
	Has a simple on/off state, along with disabled version and mouseover version. 
	Basically like a button, but ... state button with caption to the right. 
	TODO: Checkbox grouping
*/
class Checkbox: public Widget 
{
  public:
	Checkbox(Widget* wParent, string sId, rect rPosition, string sCaption, byte bGroup);
	~Checkbox();

	void Render();
	void Event(SDL_Event* event);
	
	/* TODO: Do I need to FlagRender on EventMouseMotion since it 
		changes the image? (Same question for Button)  */
	
	string GetCaption() { return mCaption; };
	
	void SetCaption(string text);
	
	void SetState(byte state);
	void SetStateCount(byte count);
	
	byte GetStateCount() { return mStateCount; };
	byte GetState() { return mState; };
	
	byte mGroup; //group number
	
  protected:
	string mCaption; 
	
	byte mState; //Which state we're currently in
	byte mLastState;
	byte mStateCount; //how many states the checkbox has. Default of 2
};

//Helper function
uShort getCheckboxState(Widget* parent, string id);

#endif //_CHECKBOX_H_
