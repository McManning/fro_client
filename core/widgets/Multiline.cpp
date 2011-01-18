
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
	m->mHighlightBackground = HIGHLIGHT_COLOR;
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

	mImage = resman->LoadImg("assets/gui/multi_bg.png");
		
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

	Image* scr = Screen::Instance();

	//TODO: color curColor = color(255,255,255); //current color we're drawing text in

	if (mImage)
		mImage->RenderBox(scr, rect(0, 0, 5, 5), r);
	
	//set target clipping here so it doesn't overflow lines
	r.w -= mScrollbar->Width();
	r.h -= 4;
	r.y += 2;

	scr->SetClip(r);

	if (!mLines.empty() && mFont)
	{

		int z = 0;
		uShort height = GetNumberOfLinesVisible();
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
			/*TODO: curColor = renderSlashCPixelText(curColor, mFont, target, r.x, renderTop - 3,
											mLines.at(topLine - 1)); */
			if (mSelected == topLine - 1 && mHighlightSelected)
				scr->DrawRect( rect(r.x + MULTILINE_LEFT_BUFFER, renderTop,
										r.w, mFont->GetHeight()),
								mHighlightBackground );

			mFont->Render( scr, r.x + MULTILINE_LEFT_BUFFER,
							renderTop,
							mLines.at(topLine - 1),
							mFontColor );

		}
    	for (int i = topLine; i < mBottomLine; i++)
		{
			if (mLines.size() - 1 < i) break; //invalid line

    	    //mFont->renderTextBlended(target, r.x, renderTop + (mFont->GetHeight() * z) - 3,
		//								mLines.at(i).c_str(), mLines.at(i).color);
        	/*TODO: curColor = renderSlashCPixelText(curColor, mFont, target, r.x, renderTop + (mFont->GetHeight() * z) - 3,
											mLines.at(i)); */

			if (mSelected == i && mHighlightSelected)
				scr->DrawRect( rect(r.x + MULTILINE_LEFT_BUFFER, renderTop + (mFont->GetHeight() * z),
									r.w, mFont->GetHeight()),
								mHighlightBackground );

			mFont->Render( scr, r.x + MULTILINE_LEFT_BUFFER,
							renderTop + (mFont->GetHeight() * z),
							mLines.at(i),
							mFontColor );
			z++;
		}
    }
	scr->SetClip();

	Widget::Render();
}

void Multiline::Event(SDL_Event* event)
{
	sShort s;
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
				mSelected = s;
				if (mSelected >= mLines.size())
					mSelected = mLines.size() - 1;
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
						mSelected = s;
						if (mSelected >= mLines.size())
							mSelected = mLines.size() - 1;
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
		uShort start = msg.find(s, 0);
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

void Multiline::SetLine(uShort line, string msg) //unwrapped multilines only
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

void Multiline::SetWidth(uShort w) //move scrollbar and resplit all lines
{
	if (!mScrollbar) return;

	if (mPosition.x + w > gui->Width())
		w = gui->Width() - mPosition.x;

	if (w < mScrollbar->Width() + MULTILINE_LEFT_BUFFER + MULTILINE_RIGHT_BUFFER)
		w = mScrollbar->Width() + MULTILINE_LEFT_BUFFER + MULTILINE_RIGHT_BUFFER;

	mPosition.w = w;

	ReflowLines(w - mScrollbar->Width() - MULTILINE_LEFT_BUFFER - MULTILINE_RIGHT_BUFFER);
}

void Multiline::SetHeight(uShort h) //cloned from List
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
void Multiline::ReflowLines(uShort w)
{
	//split the raw data into lines based on \n
	mMaxTextWidth = w;

	mLines.clear();

	for (uShort i = 0; i < mRawText.size(); i++)
	{
		SplitLines(mRawText.at(i), w);
	}

	if (mBottomLine > mLines.size() - 1) //make sure it didn't go out of range
		mBottomLine = mLines.size() - 1;
}

//Cuts up this line adds to lines list as multiple lines - Note: This does calculate in different character widths
void Multiline::SplitLines(string line, uShort maxWidth)
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
uShort Multiline::GetNumberOfLinesVisible()
{
	if (!mFont)
		return 1;
	else
		return (mPosition.h / mFont->GetHeight());
}

void Multiline::_addLine(string msg)
{
	mLines.push_back(msg);

	uShort linesVisible = GetNumberOfLinesVisible();

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
	uShort linesVisible = GetNumberOfLinesVisible();

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
	uShort linesVisible = GetNumberOfLinesVisible();

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

sShort Multiline::GetLineUnderXY(sShort x, sShort y) //TODO: FUCKING CLEAN THIS SHIT UP
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

	uShort z = 1;
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
	bool found, forwardSearch;
	string url;
	
	if (line >= mLines.size() || mUrls.empty()) 
		return "";
	
	// search up until we find a supported URL prefix
	t = line;
	found = false;
	forwardSearch = false;
	while (!found && t > -1)
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
			if (mLines.at(t).find(" ", index) != string::npos)
				break;
			
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
		
		printf("Final Url: %s\n", url.c_str());
		url = stripCodes(url); //erase any \c that may have been inserted
		
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
void Multiline::EraseLine(uShort line)
{
	if (line >= mLines.size() || mWrap) return;

	mLines.erase(mLines.begin() + line);
	mRawText.erase(mRawText.begin() + line);

	RecalculateScrollbarMax();

	if (mSelected >= mLines.size())
		mSelected = mLines.size() - 1;

//	mScrollbar->SetVisible(GetNumberOfLinesVisible() >= mLines.size());

	FlagRender();
}

void Multiline::SetTopLine(uShort val)
{
	mScrollbar->SetValue(val); //calls below version
}

void Multiline::_setTopLine(uShort val)
{
	uShort linesVisible = GetNumberOfLinesVisible();
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
	SetWidth(r.w);
	SetHeight(r.h);
	mScrollbar->SetPosition( rect(Width() - mScrollbar->Width(), 0, mScrollbar->Width(), Height()) );
	FlagRender(); //for new position
}
