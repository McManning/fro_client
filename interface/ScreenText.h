
#ifndef _SCREENTEXT_H_
#define _SCREENTEXT_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

class ScreenText : public Widget
{
  public:
  	
	enum animationType
	{
		LEFT_TO_RIGHT = 0, //wipes from off the left of the screen to the right
		RIGHT_TO_LEFT,
		TOP_TO_BOTTOM, //falls from the top
		BOTTOM_TO_TOP, //rises from the bottom
		ZOOM_IN, // Starts @ near 0% size in the center of the screen and grows to 100 or 200%
		BOUNCE, //bounces in the center for a bit
		QUICK_ZOOM, //zooms from 100% to 150%, then dies
		SMASH_IN, //adds letters one at a time
	};
	
	/**
		Set caption, load and colorize font image, detect lettering borders
		@param y Y coordinate the widget will be CENTERED on.
	*/
	ScreenText(string text, color rgb, animationType type, int y);
	~ScreenText();
	
	/**
		Draw each letter of our caption
	*/
	void Render();
	
	void Animate();

	/**
		@return source rect of the specified character
	*/
	rect GetCharacterRect(char c);
	
	animationType mAnimType;
	
	std::vector<int> mBorders;
	
	int mStopX;
	int mCurrentLetterIndex;
	int mCurrentLetterX;
	
};

#endif // _SCREENTEXT_H_

