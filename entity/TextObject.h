

#ifndef _TEXTOBJECT_H_
#define _TEXTOBJECT_H_

#include "StaticObject.h"

/* Text rendered directly to the map

<text layer="1" size="12" position="x,y" color="r,g,b" rotation="0.0">Text Here</text>

*/
class TextObject : public StaticObject
{
  public:
	TextObject();
	~TextObject();

	void SetText(string text, uShort size, double rotation);

	uShort mFontSize;
	uShort mWidth; //TODO: Use this and add wrapping!
};

#endif //_TEXTOBJECT_H_
