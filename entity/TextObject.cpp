
#include "TextObject.h"

TextObject::TextObject()
	: StaticObject()
{
	mFontSize = 0;
	mWidth = 0;
}

TextObject::~TextObject()
{

}

void TextObject::SetText(string text, uShort size, double rotation)
{
	mName = text;
	mFontSize = size;

	Font* f = fonts->Get("", mFontSize);
	if (!f || text.empty()) 
		return;

	resman->Unload(mImage);
	resman->Unload(mOriginalImage);

//	mOriginalImage = resman->NewImage(f->GetWidth(text), f->GetHeight(), color(255,0,0), true);
//	f->Render(mOriginalImage, 0, 0, text, color());

	//TODO: The above code isn't working and I'm too lazy to fix it, so we'll do a ghetto technique. Fix it later
	color c;
	if (text.size() > 5 && text.find("\\c", 0) == 0)
	{
		c = slashCtoColor(text.substr(0, 5));
		text = text.substr(5);
	}

	SDL_Surface* surf = f->RenderToSDL(text.c_str(), c);
	mOriginalImage = resman->ImageFromSurface(surf);

	if (!mOriginalImage)
		mImage = NULL;
	else
		mImage = mOriginalImage->Clone();

	mRotation = rotation;
	mScale = 1.0;
	
	SetAA(true);
	if (rotation != 0.0)
		Rotozoom(mRotation, mScale);
		
	//Set our origin point to the dead center
	if (mImage)
	{
		mOrigin.y = mImage->Height();
		mOrigin.x = mImage->Width() / 2;
	}
	else
	{
		mOrigin.x = mOrigin.y = 0;
	}
}


