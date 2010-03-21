
#include "Widget.h"
#include "WidgetImage.h"
#include "../GuiManager.h"
#include "../ResourceManager.h"

WidgetImage* makeImage(Widget* parent, string id, string file, rect src, rect dst,
						byte type, bool offsets, bool repeat)
{
	WidgetImage* wi = new WidgetImage();
	wi->mType = type;
	wi->mOffsetOnWidgetState = offsets;
	wi->mRepeat = repeat;
	wi->mDst = dst;
	wi->mSrc = src;
	wi->mId = id;
	wi->mImage = resman->LoadImg(file);
	parent->AddImage(wi);
	PRINT("makeImage " + pts(wi->mImage) + " " + file);
	return wi;
}

WidgetImage::WidgetImage()
{
	mRepeat = false;
	mOffsetOnWidgetState = false;
	mImage = NULL;
	mType = WIDGETIMAGE_FULL;
	mVisible = true;
}

WidgetImage::~WidgetImage()
{
	resman->Unload(mImage);
}

void WidgetImage::Render(Widget* parent, Image* dst, rect rDst,
						sShort xOffset, sShort yOffset)
{
	//PRINT("WidgetImage::Render");
	if (!dst || !mImage || !parent || !mVisible)
		return;

	rect rSrc = mSrc;
	rSrc.y += yOffset;
	rSrc.x += xOffset;

	rect r = parent->GetScreenPosition();

	//If we didn't supply a target, use default
	if (rDst.w == 0 || rDst.h == 0)
	{
		rDst = mDst;
		rDst.x += r.x; //change to match parents offset
		rDst.y += r.y;
	}

	//resize rects if default w/h
	if (rSrc.w == 0) rSrc.w = mImage->Width() - rSrc.x;
	if (rSrc.h == 0) rSrc.h = mImage->Height() - rSrc.y;
	if (rDst.w == 0) rDst.w = r.w - mDst.x;
	if (rDst.h == 0) rDst.h = r.h - mDst.y;

	uShort ry = rSrc.y; //memorize position in case offsetting goes off the file

	if (mOffsetOnWidgetState)
	{
		uShort adjustment;
		if (mType == WIDGETIMAGE_BOX || mType == WIDGETIMAGE_VEDGE)
			adjustment = 3;
		else
			adjustment = 1;

		if (!parent->IsActive())
		{
			rSrc.y += rSrc.h * adjustment * 3; //disabled
		}
		else if (parent->HasMouseFocus())
		{
			if (gui->IsMouseButtonDown(MOUSE_BUTTON_LEFT))
				rSrc.y += rSrc.h * adjustment * 2; //mouse down and hover
			else
				rSrc.y += rSrc.h * adjustment; //mouse hover
		}
	}

	//if we went off the file, set to default.
	if (rSrc.y + rSrc.h > mImage->Height())
		rSrc.y = ry;

	switch (mType)
	{
		case WIDGETIMAGE_BOX: {

			mImage->RenderBox(dst, rSrc, rDst);
		} break;
		case WIDGETIMAGE_VEDGE: {
			if (!mRepeat)
				rDst.w = rSrc.w;

			if (rDst.h > rSrc.h * 2) //center
				mImage->RenderPattern(dst,
							rect( 0 + rSrc.x, rSrc.h + rSrc.y, rSrc.w, rSrc.h ),
							rect( rDst.x, rDst.y + rSrc.h, rDst.w, rDst.h - rSrc.h * 2)
						);

			//top
			mImage->RenderPattern(dst,
						rSrc,
						rect( rDst.x, rDst.y, rDst.w, rSrc.h )
					);

			//bottom
			mImage->RenderPattern(dst,
						rect( rSrc.x, rSrc.h * 2 + rSrc.y, rSrc.w, rSrc.h ),
						rect( rDst.x, rDst.y + rDst.h - rSrc.h, rDst.w, rSrc.h )
					);

		} break;
		case WIDGETIMAGE_HEDGE: {
			if (!mRepeat)
				rDst.h = rSrc.h;

			if (rDst.w > rSrc.w * 2) //center
				mImage->RenderPattern(dst,
							rect( rSrc.w + rSrc.x, 0 + rSrc.y, rSrc.w, rSrc.h ),
							rect( rDst.x + rSrc.w, rDst.y, rDst.w - rSrc.w * 2, rDst.h)
						);

			//left
			mImage->RenderPattern(dst,
						rSrc,
						rect( rDst.x, rDst.y, rSrc.w, rDst.h )
					);

			//right
			mImage->RenderPattern(dst,
						rect( rSrc.w * 2 + rSrc.x, rSrc.y, rSrc.w, rSrc.h ),
						rect( rDst.x + rDst.w - rSrc.w, rDst.y, rSrc.w, rDst.h )
					);

		} break;
		default: { //WIDGETIMAGE_FULL
			if (!mRepeat)
			{
				rDst.w = rSrc.w;
				rDst.h = rSrc.h;
			}
			mImage->RenderPattern(dst, rSrc, rDst);
		} break;
	}

}

