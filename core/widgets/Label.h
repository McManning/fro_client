
#ifndef _LABEL_H_
#define _LABEL_H_

#include "Widget.h"

class Font;
class Image;

/* Basic Label Class.
	TODO: Wrapping, Coloring, fancy crap, etc.
*/
class Label: public Widget 
{
  public:
	Label();
	Label(Widget* wParent, string sId, rect rPosition, string sCaption = "");
	~Label();

	void Render();
	
	string GetCaption() { return mCaption; };
	void SetCaption(string text);

	uShort mMaxWidth; //0 = unlimited

  protected:
	string mCaption; 
};

#endif //_LABEL_H_
