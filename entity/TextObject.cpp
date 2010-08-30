
#include "TextObject.h"

TextObject::TextObject()
	: StaticObject()
{
	mType = ENTITY_TEXT;
}

TextObject::~TextObject()
{

}

void TextObject::SetFont(string file, int size, int style)
{
	mFont = fonts->Get(file, size, style);
}

void TextObject::SetText(string text, int maxWidth)
{
	mText = text;
	
	if (!mFont || text.empty()) 
	{
		SetImage(NULL);
		return;
	}

	rect r = mFont->GetTextRect(text, true, maxWidth);

	Image* img = resman->NewImage(r.w, r.h, color(255,0,255,0), true);

	// Since we used NewImage, gotta do a little slower-rendering to get alpha right,
	// because SDL is a bitch about RGBA->RGBA
	mFont->UseAlphaBlending(true);
	mFont->Render(img, 0, 0, text, color(), maxWidth);
	mFont->UseAlphaBlending(false);
	
	// Done creating, let StaticObject do its stuff to the image
	SetImage(img);
}


