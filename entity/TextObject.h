

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

	void SetText(string text, color c, int maxWidth = 0);
	void SetFont(string file = "", int size = 0, int style = 0);
	
	void SetCentered(bool centered);
	bool IsCentered() const { return mCentered; };

	void UpdateCollisionAndOrigin();

	int LuaSetProp(lua_State* ls, string& prop, int index);
	int LuaGetProp(lua_State* ls, string& prop);
	
	int mFontSize;
	int mMaxWidth;
	bool mCentered;
	color mColor;
	Font* mFont;
	string mText;
};

#endif //_TEXTOBJECT_H_
