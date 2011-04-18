
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

	void SplitLines(string line, int w);

	string GetUrl(int line);
	int GetLineUnderXY(int x, int y);

	void RecalculateScrollbarMax();
	int GetNumberOfLinesVisible();

	void SetTopLine(int val);
	void _setTopLine(int val); //called by the scrollbar only

	string GetSelectedText()
	{
		if (mSelected < mLines.size() && mSelected > -1)
			return mLines.at(mSelected);
		else
			return "";
	};

	void EraseLine(int line);
	void SetLine(int line, string msg);
	
	void SetSelected(int line);

	Scrollbar* mScrollbar;

	std::vector<string> mLines;
	std::vector<string> mRawText;
	std::vector<string> mUrls; //blah.blah pulled from the text

	int mBottomLine; //bottom line number
	int mMaxTextWidth; //maximum width for text
	bool mWrap;
	bool mHighlightSelected;
	bool mSelectOnHover;
	bool mHideScrollbar;
	int mSelected;

	void ReflowLines(int w);

	void (*onLeftDoubleClickCallback)(Multiline*);
	void (*onLeftSingleClickCallback)(Multiline*);

	void (*onRightSingleClickCallback)(Multiline*);
  protected:
	void _addLine(string msg);

	void _setWidth(int w);
	void _setHeight(int h);

	bool mClickedOnce; //allows double clicking (for links 'n shit)
};

//Create a list-styled version of the multiline
Multiline* makeList(Widget* parent, string id, rect position);

void callback_multilineScrollbar(Scrollbar* caller);

#endif //_MULTILINE_H_
