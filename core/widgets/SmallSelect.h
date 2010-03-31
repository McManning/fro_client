
/*
	idk what's a good name for this.. Basically, two arrow buttons.
	Has a list of items (strings). ++ the visible one when the 
	right button is pressed, -- when the left is pressed. Loop. 
	Very simple but it'll be a compressed method of checkbox groups
	and all that jazz. 
*/
#ifndef _SMALLSELECT_H_
#define _SMALLSELECT_H_

#include "Widget.h"

#define SMALLSELECT_BUFFER 3

class Button;
class SmallSelect: public Widget 
{
  public:
	SmallSelect();
	SmallSelect(Widget* wParent, string sId, rect rPosition,
			void (*cbOnChange)(SmallSelect*));
	~SmallSelect();

	void Render(uLong ms);
	
	void Event(SDL_Event* event);
	
	void SetPosition(rect r);

	void AddItem(string s) { mItems.push_back(s); };
	
	string GetSelectedItem() const;
	
	void (*onChangeCallback)(SmallSelect*);
	
	vString mItems; //options and whatever
	uShort mSelectedIndex; 
	
	Button* mLeft;
	Button* mRight;
	
	Image* mBackgroundImage;
};


#endif //_SMALLSELECT_H_
