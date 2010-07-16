
#include "DamageIcon.h"
#include "../map/Map.h"

const int LETTER_CROWDING = 5;
const int DAMAGE_ICON_TTL = 800;
const int DAMAGE_ICON_THINK = 30;

uShort timer_DeleteDamageIcon(timer* t, uLong ms)
{
	DamageIcon* d = (DamageIcon*)t->userData;
	if (d)
		d->mMap->RemoveEntity(d);

	return TIMER_DESTROY;
}

uShort timer_DamageIconThink(timer* t, uLong ms)
{
	DamageIcon* d = (DamageIcon*)t->userData;
	if (!d)
		return TIMER_DESTROY;	
		
	d->mOrigin.y++;
	
	return TIMER_CONTINUE;
}
	
DamageIcon::DamageIcon()
{
	mType = ENTITY_DAMAGEICON;
	//mId = its(amount);

	Image* img = resman->LoadImg("assets/dmg_font.png");
	mFontImage = img->Clone(true);
	resman->Unload(img); //dereference it, since we no longer touch the original

	// Background image will be a random one from the pool
	LoadImage("assets/dmg_bg" + its(rnd(0, 4)) + ".png");

	// Determine letter sizes based on pink pixels found. 
	// Create an array to store this info for quicker access later
	color c;
	for (int x = 0; x < mFontImage->Width(); ++x)
	{
		c = mFontImage->GetPixel(x, 0);
		if (c.r == 255 && c.g == 0 && c.b == 255) //pink!
			mBorders.push_back(x);
	}

	// Colorize our version to suit our needs
	//mFontImage->ColorizeGreyscale(fontRgb);
	//mImage->ColorizeGreyscale(bgRgb);
	
	// Add timer to destroy this entity after some constant time
	timers->Add("", DAMAGE_ICON_TTL, false, 
				timer_DeleteDamageIcon, NULL, this);
	timers->Add("", DAMAGE_ICON_THINK, false, 
				timer_DamageIconThink, NULL, this);
				
	mOrigin.y = mImage->Height();
	mOrigin.x = mImage->Width()/2;
}

DamageIcon::~DamageIcon()
{
	resman->Unload(mFontImage);
}

void DamageIcon::Render()
{
	Image* scr = Screen::Instance();
	rect r;
	int width, i, x, index, dx;

	/*if (mId.empty())
	{
		r = GetBoundingRect();
		if (!IsPositionRelativeToScreen())
			r = mMap->ToScreenPosition( r );

		mImage->Render(scr, r.x, r.y);
		return;
	}*/

	// Pre-Calculate total width of the numbers so we can center it
	width = 0;
	for (i = 0; i < mId.length(); ++i)
	{
		index = mId.at(i) - '0';

		if (index - 1 < 0)
			x = 0;
		else
			x = mBorders.at(index - 1) + 1;

		width += (mBorders.at(index) - x) - LETTER_CROWDING;
	}

	// Get screen position
	r = rect( mPosition.x - mOrigin.x, mPosition.y - mOrigin.y, 
				width, mImage->Height() );

	if (!IsPositionRelativeToScreen())
		r = mMap->ToScreenPosition( r );

	dx = r.x - (width - mImage->Width()) / 2 - LETTER_CROWDING; //Starting x position
	
	// Draw background, centered
	mImage->Render(scr, r.x, r.y);
	
	// Draw numbers, centered
	for (i = 0; i < mId.length(); ++i)
	{
		index = mId.at(i) - '0';

		if (index - 1 < 0)
			x = 0;
		else
			x = mBorders.at(index - 1) + 1;

		width = mBorders.at(index) - x;

		mFontImage->Render(scr, dx, r.y + (mImage->Height() - mFontImage->Height()) / 2, 
							rect(x, 0, width, mFontImage->Height()));
		dx += width - LETTER_CROWDING;
	}
}

/* This (better) method will only work if NewImage() works, which it does not. :(
void DamageIcon::_buildIcon(int amount, color& bgRgb, color& fontRgb)
{
	int x, i, dx, width, index;
	std::vector<int> borders;
	color c;
	string dmg = its(amount);
	
	Image* fontImg = resman->LoadImg("assets/dmg_font.png");
	
	// Background image will be a random one from the pool
	Image* bgImg = resman->LoadImg("assets/dmg_bg" + its(rnd(0, 0)) + ".png");

	mImage = bgImg;

	// Determine letter sizes based on pink pixels found. 
	// Create an array to store this info for quicker access later
	for (int x = 0; x < fontImg->Width(); ++x)
	{
		c = fontImg->GetPixel(x, 0);
		if (c.r == 255 && c.g == 0 && c.b == 255) //pink!
			borders.push_back(x);
	}

	// Colorize our version to suit our needs
	fontImg->ColorizeGreyscale(fontRgb);
	bgImg->ColorizeGreyscale(bgRgb);
	
	// Calculate total width of the numbers
	width = 0;
	for (i = 0; i < dmg.length(); ++i)
	{
		index = dmg.at(i) - '0';
		x = borders.at(index - 1) + 1;
		width += (borders.at(index) - x) - LETTER_CROWDING;
	}
	
	// Make sure it's not smaller than the background image
	if (width < bgImg->Width())
		width = bgImg->Width();
	
	//Now throw them all onto a single image
	mImage = resman->NewImage(width, bgImg->Height(), color(), false);
	
	// Draw background, centered
	bgImg->Render(mImage, mImage->Width() / 2 - bgImg->Width() / 2, 0);

	// Draw numbers, centered
	dx = mImage->Width() / 2 - width / 2; //Starting x position
	for (i = 0; i < dmg.length(); ++i)
	{
		index = dmg.at(i) - '0';
		x = borders.at(index - 1) + 1;
		width = borders.at(index) - x;
		
		fontImg->Render(mImage, dx, mImage->Height() / 2 - fontImg->Height() / 2, 
							rect(x, 0, width, fontImg->Height()));
		dx += width - LETTER_CROWDING;
	}
	
	//Cleanup
	resman->Unload(bgImg);
	resman->Unload(fontImg);
}
*/

