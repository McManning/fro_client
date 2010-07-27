

#ifndef _TEXTOBJECT_H_
#define _TEXTOBJECT_H_

#include "StaticObject.h"

/* Text rendered directly to the map as an entity
*/
class TextObject : public StaticObject
{
  public:
	TextObject();
	~TextObject();

	void SetText(string text, int maxWidth = 0);
	void SetFont(string file = "", int size = 0, int style = 0);

	int mFontSize;
	int mMaxWidth;
	Font* mFont;
	string mText;
};

#endif //_TEXTOBJECT_H_
