
/*
 * Copyright (c) 2011 Chase McManning
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */



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
