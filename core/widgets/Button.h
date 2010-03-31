
#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "Widget.h"

class Font;
class Image;
//Basic button widget
class Button: public Widget 
{
  public:
	Button(Widget* wParent, string sId, rect rPosition, string sCaption,
			void (*cbOnClick)(Button*));
	~Button();

	void Render(uLong ms);
	
	void Event(SDL_Event* event);
	
	string GetCaption() { return mCaption; };
	void SetCaption(string text);

	/*	Set this buttons mImage to the specified file. Use this rather than accessing mImage directly 
		from outside the class in order to avoid memory leaks (+shorter to write)
	*/
	void SetImage(string file);
	
	void (*onClickCallback)(Button*);
	
  protected:
	string mCaption; 
	Image* mCaptionImage;
};

void callback_closeFrame(Button* b);

#endif //_BUTTON_H_
