
#include <lua.hpp>
#include "TextObject.h"

TextObject::TextObject()
	: StaticObject()
{
	mType = ENTITY_TEXT;
	mCentered = false;
}

TextObject::~TextObject()
{

}

void TextObject::SetFont(string file, int size, int style)
{
	mFont = fonts->Get(file, size, style);
	mFontSize = size;
}

void TextObject::SetText(string text, color c, int maxWidth)
{
	mText = text;
	mMaxWidth = maxWidth;
	mColor = c;
	
	if (!mFont || text.empty())
	{
		SetImage(NULL);
		return;
	}
	else
	{
		rect r = mFont->GetTextRect(text, true, maxWidth);

		Image* img = resman->NewImage(r.w, r.h, color(255,0,255,0), true);

		// Since we used NewImage, gotta do a little slower-rendering to get alpha right,
		// because SDL is a bitch about RGBA->RGBA
		mFont->UseAlphaBlending(true);
		mFont->Render(img, 0, 0, text, mColor, maxWidth);
		mFont->UseAlphaBlending(false);
		
		// Done creating, let StaticObject do its stuff to the image
		SetImage(img);
	}
	
	UpdateCollisionAndOrigin();
}

int TextObject::LuaSetProp(lua_State* ls, string& prop, int index)
{
	if (prop == "center") SetCentered(lua_toboolean(ls, index));
	else return StaticObject::LuaSetProp(ls, prop, index);

	return 1;
}

int TextObject::LuaGetProp(lua_State* ls, string& prop)
{
	if (prop == "center") lua_pushboolean( ls, IsCentered() );
	else return StaticObject::LuaGetProp(ls, prop);

	return 1;
}

void TextObject::SetCentered(bool centered)
{
	mCentered = centered;
	UpdateCollisionAndOrigin();
}

void TextObject::UpdateCollisionAndOrigin()
{
	if (GetImage() && mCentered)
	{
		mOrigin.x =	GetImage()->Width() / 2;
		mOrigin.y = GetImage()->Height();
	}
	else
	{
		mOrigin.x = 0;
		mOrigin.y = 0;	
	}

	AddPositionRectForUpdate();
}

