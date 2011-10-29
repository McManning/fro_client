
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


#include "Multiline.h"
#include "Scrollbar.h"
#include "OpenUrl.h"
#include "../GuiManager.h"
#include "../Screen.h"
#include "../FontManager.h"

//supported clickable urls
const char* g_sSupportedUrls[] =
{
	"http://",
	"ftp://",
	"https://",
	//"steam://",
	NULL
};

Multiline* makeList(Widget* parent, string id, rect position)
{
	Multiline* m = new Multiline(parent, id, position);
	m->mHighlightSelected = true;
	m->mWrap = false;
	return m;
}

void callback_multilineScrollbar(Scrollbar* caller)
{
	Multiline* parent = (Multiline*)(caller->mParent);
	if (parent)
		parent->_setTopLine(caller->GetValue());
}

Multiline::Multiline(Widget* wParent, string sId, rect rPosition)
{
	mClickedOnce = false;
	mHideScrollbar = false;
	mType = WIDGET_MULTILINE;
	mBottomLine = 0; //at the start
	mWrap = true;
	mHighlightSelected = false;
	mSelectOnHover = false;
	mSelected = -1;
	onLeftDoubleClickCallback = onLeftSingleClickCallback = NULL;
	onRightSingleClickCallback = NULL;
	mScrollbar = NULL;
	mFont = fonts->Get();

	mId = sId;

	SetImage("assets/gui/multi_bg.png");

	mScrollbar = new Scrollbar(this, "", rect(0,0,15,rPosition.h), VERTICAL, 1, 1, 0,
								callback_multilineScrollbar);

	SetPosition(rPosition);

	if (wParent)
		wParent->Add(this);
}

Multiline::~Multiline()
{
	mLines.clear();
	mUrls.clear();
}

void Multiline::Render()
{
    rect r = GetScreenPosition();
	Screen* scr = Screen::Instance();

	// Don't even bother rendering the lines if nothing is in the clips
	if (!scr->IsRectDrawable(r))
		return;

	if (mImage)
		mImage->RenderBox(scr, rect(0, 0, 5, 5), r);

	//set target clipping here so it doesn't overflow lines
	r.w -= mScrollbar->Width();
	r.h -= 4;
	r.y += 2;

	scr->PushClip(r);

	if (!mLines.empty() && mFont)
	{
		int z = 0;
		int height = GetNumberOfLinesVisible();
		height *= (mFont->GetHeight());
		height += mFont->GetHeight();

		int topLine = mBottomLine - GetNumberOfLinesVisible(); //-1 to get an extra line up top for overflow
		if (topLine < 0) topLine = 0;

		int renderTop;
		if (topLine == 0)
			renderTop = r.y - mFont->GetHeight();
		else
			renderTop = (r.y + r.h) - height;

		z = 1;

		//draw the line above topline if it's > 0 for a hint of overflow
		if (topLine > 0)
		{
			if (mSelected == topLine - 1 && mHighlightSelected)
				scr->DrawRect( rect(r.x + MULTILINE_LEFT_BUFFER, renderTop,
										r.w, mFont->GetHeight()),
								gui->mBaseColor );

			mFont->Render( scr, r.x + MULTILINE_LEFT_BUFFER,
							renderTop,
							mLines.at(topLine - 1),
							mFontColor );

		}

    	for (int i = topLine; i < mBottomLine; i++)
		{
			if (mLines.size() - 1 < i) break; //invalid line

			if (mSelected == i && mHighlightSelected)
			{
				scr->DrawRect( rect(r.x + MULTILINE_LEFT_BUFFER, renderTop + (mFont->GetHeight() * z),
									r.w, mFont->GetHeight()),
								gui->mBaseColor );

				mFont->Render( scr, r.x + MULTILINE_LEFT_BUFFER,
								renderTop + (mFont->GetHeight() * z),
								mLines.at(i),
								(isDark(gui->mBaseColor) ? invertColor(mFontColor) : mFontColor) );
			}
			else
			{
				mFont->Render( scr, r.x + MULTILINE_LEFT_BUFFER,
								renderTop + (mFont->GetHeight() * z),
								mLines.at(i),
								mFontColor );
			}

			z++;
		}
    }
	scr->PopClip();

	Widget::Render();
}

void Multiline::Event(SDL_Event* event)
{
	int s;
	string url;
	switch (event->type)
	{
        case SDL_MOUSEBUTTONDOWN: {
			if (event->button.button == MOUSE_BUTTON_WHEELUP)
				ScrollUp();
			else if (event->button.button == MOUSE_BUTTON_WHEELDOWN)
				ScrollDown();
			else if (event->button.button == SDL_BUTTON_RIGHT)
			{
				s = GetLineUnderXY(event->button.x, event->button.y);
				if (s != -1 && !onRightSingleClickCallback)
				{
					url = GetUrl(s);
					if (!url.empty())
						new OpenUrl(url);
				}
			}
		} break;
		case SDL_MOUSEBUTTONUP: {
			s = GetLineUnderXY(event->button.x, event->button.y);
			if (s != -1)
			{
				SetSelected(s);

				if (mClickedOnce)
				{
					if (event->button.button == SDL_BUTTON_LEFT)
					{
						if (onLeftDoubleClickCallback)
							onLeftDoubleClickCallback(this);
					}
					mClickedOnce = false;
				}
				else
				{
					if (event->button.button == SDL_BUTTON_LEFT)
					{
						mClickedOnce = true;

						if (onLeftSingleClickCallback)
							onLeftSingleClickCallback(this);
					}
					else if (event->button.button == SDL_BUTTON_RIGHT)
					{
						if (onRightSingleClickCallback)
							onRightSingleClickCallback(this);
					}
				}
			}
		} break;
		case SDL_MOUSEMOTION: {
			if (HasMouseFocus())
			{
				mClickedOnce = false;
				if (mSelectOnHover)
				{
					s = GetLineUnderXY(event->motion.x, event->motion.y);
					if (s != -1)
					{
						SetSelected(s);
					}
					FlagRender();
				}
				else // Check for urls, and give a hint box
				{
					s = GetLineUnderXY(event->motion.x, event->motion.y);
					if (s != -1 && !GetUrl(s).empty())
						mHoverText = "Right click to open url";
					else
						mHoverText.clear();
				}
			}
		} break;
		default: break;
	}
	Widget::Event(event);
}

void Multiline::AddMessage(string msg)
{
	//grab any url's from the msg first
	string s;

	int i = 0;
	while (g_sSupportedUrls[i])
	{
		if (msg.find(g_sSupportedUrls[i], 0) != string::npos)
		{
			s = g_sSupportedUrls[i];
			break;
		}
		++i;
	}

	if (!s.empty()) //we have a url
	{
		size_t start = msg.find(s, 0);
		string url = "";

		if (msg.find(" ", start)) //there's a space
			url = msg.substr(start, msg.find(" ", start) - start);
		else //goes to end~
			url = msg.substr(start);

		mUrls.push_back(url);
	}

	mRawText.push_back(msg);
	//	dbgout(stderr, "MSG:%s\nRAWTEXT:%s\n", msg.c_str(), mRawText.c_str());
	SplitLines(msg, mMaxTextWidth);

	FlagRender();
}

void Multiline::SetLine(int line, string msg) //unwrapped multilines only
{
	if (line < mRawText.size() && line < mLines.size() && !mWrap)
	{
		mRawText.at(line) = msg;
		mLines.at(line) = msg;
	}

	FlagRender();
}

void Multiline::RecalculateScrollbarMax()
{
	int s = mLines.size() - GetNumberOfLinesVisible();
	if (s > 0)
	{
		mScrollbar->SetMax(s);

		if (!mHideScrollbar)
			mScrollbar->SetVisible(true);
	}
	else
	{
		mScrollbar->SetMax(1);
		mScrollbar->SetVisible(false);
	}
}

void Multiline::_setWidth(int w) //move scrollbar and resplit all lines
{
	if (!mScrollbar) return;

	if (mPosition.x + w > gui->Width())
		w = gui->Width() - mPosition.x;

	if (w < mScrollbar->Width() + MULTILINE_LEFT_BUFFER + MULTILINE_RIGHT_BUFFER)
		w = mScrollbar->Width() + MULTILINE_LEFT_BUFFER + MULTILINE_RIGHT_BUFFER;

	mPosition.w = w;

	ReflowLines(w - mScrollbar->Width() - MULTILINE_LEFT_BUFFER - MULTILINE_RIGHT_BUFFER);
}

void Multiline::_setHeight(int h)
{
	if (!mScrollbar) return; //TODO: idk what to do here.

	//make sure it doesn't go off the screen
	if (mPosition.y + h > gui->Height())
		h = gui->Height() - mPosition.y;

	if (h < mScrollbar->ScrollerSize())
		h = mScrollbar->ScrollerSize();

	mPosition.h = h;

	mBottomLine = mLines.size(); // - 1; //bump it down

	RecalculateScrollbarMax();

	mScrollbar->SetValue(mBottomLine - GetNumberOfLinesVisible());
}

//erase all line surfaces and re-splits the raw data into the lines vector (a lot of processing here...)
//TODO: This. A lot of lines will LAG LIKE HELL.
void Multiline::ReflowLines(int w)
{
	//split the raw data into lines based on \n
	mMaxTextWidth = w;

	mLines.clear();

	for (int i = 0; i < mRawText.size(); i++)
	{
		SplitLines(mRawText.at(i), w);
	}

	if (mBottomLine > mLines.size() - 1) //make sure it didn't go out of range
		mBottomLine = mLines.size() - 1;
}

//Cuts up this line adds to lines list as multiple lines - Note: This does calculate in different character widths
void Multiline::SplitLines(string line, int maxWidth)
{
	vString v;
	string lastColor;
	int i, index;

	if (!mWrap || maxWidth == 0) //just clone (TODO: SHOULD just use the original list, but we need to add that check everywhere
	{
		_addLine(line);
		return;
	}

	ASSERT(mFont);

	mFont->CharacterWrapMessage(v, line, maxWidth);
	for (i = 0; i < v.size(); ++i)
	{
		_addLine( lastColor + v.at(i) );

		index = v.at(i).rfind("\\c");
		if (index != string::npos && v.at(i).length() > index + 4)
			lastColor = v.at(i).substr(index, 5);
	}
}

//return number of lines visible
int Multiline::GetNumberOfLinesVisible()
{
	if (!mFont)
		return 1;
	else
		return (mPosition.h / mFont->GetHeight());
}

void Multiline::_addLine(string msg)
{
	mLines.push_back(msg);

	int linesVisible = GetNumberOfLinesVisible();

	if (mLines.size() > MULTILINE_MAX_LINES)
		mLines.erase(mLines.begin());

	RecalculateScrollbarMax();

	if (mBottomLine == mLines.size() - 1)
	{
		mBottomLine++;
		if (mBottomLine - linesVisible < 0)
			mScrollbar->SetValue(0);
		else
			mScrollbar->SetValue(mBottomLine - linesVisible);
	}

}

void Multiline::ScrollUp() //cloned from List
{
	int linesVisible = GetNumberOfLinesVisible();

	if (mBottomLine > linesVisible)
	{
		mBottomLine--;
		if (mBottomLine - linesVisible < 0)
			mScrollbar->SetValue(0);
		else
			mScrollbar->SetValue(mBottomLine - linesVisible);
	}
}

void Multiline::ScrollDown() //cloned from List
{
	int linesVisible = GetNumberOfLinesVisible();

	if (mBottomLine < mLines.size())
	{
		mBottomLine++;
		if (mBottomLine - linesVisible < 0)
			mScrollbar->SetValue(0);
		else
			mScrollbar->SetValue(mBottomLine - linesVisible);
	}
}

void Multiline::Clear()
{
	mLines.clear();
	mUrls.clear();
	mRawText.clear();

	mBottomLine = 0;

	RecalculateScrollbarMax();

	FlagRender();
}

int Multiline::GetLineUnderXY(int x, int y) //TODO: FUCKING CLEAN THIS SHIT UP
{
	if (!mFont || mLines.empty()) return -1;

	rect r = GetScreenPosition();
	r.w = mMaxTextWidth;

	r.y += 2;
	r.h -= 4;

	if (!isPointInRect(r, x, y))
		return -1;

	int topLine = mBottomLine - GetNumberOfLinesVisible(); //-1 to get an extra line up top for overflow
	if (topLine < 0)
		topLine = 0;

	int height = GetNumberOfLinesVisible();
	height *= (mFont->GetHeight());
	height += mFont->GetHeight();

	int renderTop;
	if (topLine == 0)
		renderTop = r.y - mFont->GetHeight();
	else
		renderTop = (r.y + r.h) - height;

	r.h = mFont->GetHeight();
	if (topLine > 0)
	{
		if (isPointInRect(rect(r.x + MULTILINE_LEFT_BUFFER, renderTop,
						r.w, r.h), x, y))
			return topLine - 1;

	}

	int z = 1;
	for (int i = topLine; i < mBottomLine; i++)
	{
		if (isPointInRect(rect(r.x + MULTILINE_LEFT_BUFFER,
						renderTop + (r.h * z), r.w, r.h), x, y))
			return i;
		z++;
	}

    return -1;
}

/**	From the provided line index, it will attempt to search both up and down for the beginning and end of a url,
	compare that collected url with the list of collected ones when messages were added, find a match, and return.

	@todo Something less complicated :(
*/
string Multiline::GetUrl(int line)
{
	int t, i, index;
	bool found, forwardSearch, stopSearching;
	string url;

	if (line >= mLines.size() || mUrls.empty())
		return "";

	// search up until we find a supported URL prefix
	t = line;
	found = false;
	forwardSearch = false;
	stopSearching = false;
	while (!found && t > -1 && !stopSearching)
	{
		i = 0;
		while (g_sSupportedUrls[i] && !found) // search for valid prefix
		{
			index = mLines.at(t).rfind(g_sSupportedUrls[i]);
			if (index != string::npos) //found prefix!
			{
				//make sure there's no space between the prefix and the body we clicked on
				if (mLines.at(t).find(" ", index) == string::npos)
				{
					url = mLines.at(t).substr(index);
					found = true;
					forwardSearch = true;
				}
				else //there CAN be a space if this url is on the line we're trying to use
				{
					if (line == t)
					{
						url = mLines.at(t).substr(index, mLines.at(t).find(" ", index) - index);
						found = true;
						forwardSearch = false;
					}
				}
			}
			++i;
		}

		// To speed up searching, if this line has any whitespace, then it's safe to assume the url matching the line
		// we are trying to access does not exist.
		if (!found)
		{
			if (mLines.at(t).find(" ", 0) != string::npos)
			{
				stopSearching = true;
			}

			--t;
		}
	}

	if (found)
	{
		++t; // skip ahead a line as the above search ends too far back

		// search down until we find whitespace
		if (forwardSearch)
		{
			found = false;
			while (t < mLines.size() && !found)
			{
				index = mLines.at(t).find(' ');
				if (index == string::npos) //keep going
				{
					url += mLines.at(t);
				}
				else //we have a space
				{
					url += mLines.at(t).substr(0, index);
					found = true; //grabbed it
				}
				++t;
			}
		}

		url = stripColorCodes(url); //erase any \c that may have been inserted

		//now we have a url, however it may have some shit tagged on (from the next line)
		//soooo, compare it to the collected url list. TODO: A better method?
		found = false;
		for (t = mUrls.size() - 1; t > -1 && !found; --t) //reverse cuz most recent are @ the bottom
		{
			if (mUrls.at(t) == url.substr(0, mUrls.at(t).length())) //compares length along with contents
			{
				url = mUrls.at(t);
				found = true;
			}
		}
	}

	return url;
}

//TODO: eraseable while wrapping?
void Multiline::EraseLine(int line)
{
	if (line >= mLines.size() || mWrap) return;

	mLines.erase(mLines.begin() + line);
	mRawText.erase(mRawText.begin() + line);

	RecalculateScrollbarMax();

	SetSelected(mSelected); //shift if mSelected was the deleted down

//	mScrollbar->SetVisible(GetNumberOfLinesVisible() >= mLines.size());

	FlagRender();
}

void Multiline::SetTopLine(int val)
{
	mScrollbar->SetValue(val); //calls below version
}

void Multiline::_setTopLine(int val)
{
	int linesVisible = GetNumberOfLinesVisible();
	if (mBottomLine < linesVisible) return; //don't do

	mBottomLine = val + linesVisible;

	if (mBottomLine > mLines.size())
		mBottomLine = mLines.size(); //make sure it's not set to an invalid line

	FlagRender();
}

//Overloaded so we can move the scrollbar
void Multiline::SetPosition(rect r)
{
	FlagRender(); //for old position
	mPosition.x = r.x;
	mPosition.y = r.y;
	_setWidth(r.w);
	_setHeight(r.h);
	mScrollbar->SetPosition( rect(Width() - mScrollbar->Width(), 0, mScrollbar->Width(), Height()) );
	FlagRender(); //for new position
}

void Multiline::SetSelected(int line)
{
	mSelected = line;
	if (mSelected >= mLines.size())
		mSelected = mLines.size() - 1;

	FlagRender();
}

