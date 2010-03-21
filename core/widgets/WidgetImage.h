
/*
	WidgetImage internally defines how it's rendered, and can be created from XML. 
		Used by Widgets usually.
*/

#ifndef _WIDGETIMAGE_H_
#define _WIDGETIMAGE_H_

#include "../Common.h"

enum //WidgetImage .. type
{
	WIDGETIMAGE_FULL = 0,
	WIDGETIMAGE_BOX,
	WIDGETIMAGE_VEDGE,
	WIDGETIMAGE_HEDGE
};

class Image;
class Widget;
class WidgetImage
{
  public:
	WidgetImage();
	~WidgetImage();

	void Render(Widget* parent, Image* dst, rect rDst = rect(), 
				sShort xOffset = 0, sShort yOffset = 0);

	byte mType;
	rect mSrc;
	rect mDst;
	bool mRepeat;
	bool mOffsetOnWidgetState;
	string mId;
	bool mVisible;
	Image* mImage;
};

//TODO: Turn this into an overloaded constructor
WidgetImage* makeImage(Widget* parent, string id, string file, rect src, rect dst, 
						byte type, bool offsets, bool repeat);

#endif //_WIDGETIMAGE_H_
