
#ifndef _MULTILINE_H_
#define _MULTILINE_H_

#include "Widget.h"

#define MULTILINE_MAX_LINES 300
#define MULTILINE_LEFT_BUFFER 3
#define MULTILINE_RIGHT_BUFFER 6

class Scrollbar;
class Font;
class Multiline: public Widget
{
  public:
	Multiline(Widget* wParent, string sId, rect rPosition);
	~Multiline();

	void Render();
	void Event(SDL_Event* event);

	void SetPosition(rect r);

	void ScrollDown();
	void ScrollUp();

	void Clear();

	void AddMessage(string msg);

	void SplitLines(string line, uShort w);

	string GetUrl(int line);
	sShort GetLineUnderXY(sShort x, sShort y);

	void RecalculateScrollbarMax();
	uShort GetNumberOfLinesVisible();

	void SetTopLine(uShort val);
	void _setTopLine(uShort val); //called by the scrollbar only

	string GetSelectedText()
	{
		if (mSelected < mLines.size() && mSelected > -1)
			return mLines.at(mSelected);
		else
			return "";
	};

	void EraseLine(uShort line);
	void SetLine(uShort line, string msg);

	Scrollbar* mScrollbar;

	std::vector<string> mLines;
	std::vector<string> mRawText;
	std::vector<string> mUrls; //blah.blah pulled from the text

	uShort mBottomLine; //bottom line number
	uShort mMaxTextWidth; //maximum width for text
	bool mWrap;
	bool mHighlightSelected;
	bool mSelectOnHover;
	bool mHideScrollbar;
	sShort mSelected;
	color mHighlightBackground;

	void ReflowLines(uShort w);

	void (*onLeftDoubleClickCallback)(Multiline*);
	void (*onLeftSingleClickCallback)(Multiline*);

	void (*onRightSingleClickCallback)(Multiline*);
  protected:
	void _addLine(string msg);

	void SetWidth(uShort w);
	void SetHeight(uShort h);

	bool mClickedOnce; //allows double clicking (for links 'n shit)
};

//Create a list-styled version of the multiline
Multiline* makeList(Widget* parent, string id, rect position);

void callback_multilineScrollbar(Scrollbar* caller);

#endif //_MULTILINE_H_
