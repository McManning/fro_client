
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
	"steam://",
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

void Multiline::Render(uLong ms)
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

	Widget::Render(ms);
}

void Multiline::Event(SDL_Event* event)
{
	sShort s;
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
				if (s != -1)
				{
					ClickUrl(s);
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
				}
			}
		} break;
		case SDL_MOUSEMOTION: {
			mClickedOnce = false;
			if (mSelectOnHover)
			{
				sShort s = GetLineUnderXY(event->motion.x, event->motion.y);
				if (s != -1)
				{
					mSelected = s;
					if (mSelected >= mLines.size())
						mSelected = mLines.size() - 1;
				}
			}
		} break;
		default: break;
	}
	Widget::Event(event);
}

//splits via \\n into AddMessage (for serverside messages and whatnot that come from config data that strips control characters)
void Multiline::AddFormattedMessage(string msg)
{
    uShort nx = 0;
	string::size_type pos, lastPos = 0;
	do //loop through and break it up based on \n~
	{
		pos = msg.find("\\n", lastPos);
		if (pos != string::npos)
			nx = pos - lastPos;
		else
			nx = msg.size() - lastPos;
		//TODO: Color each one with the color the previous message ended with.
		AddMessage(msg.substr(lastPos, nx));
		lastPos = pos + 2;
	}
	while (pos != string::npos);

#ifdef OPTIMIZED
	Screen::Instance()->Update();
#endif
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
#ifdef OPTIMIZED
	Screen::Instance()->Update();
#endif
}

void Multiline::SetLine(uShort line, string msg) //unwrapped multilines only
{
	if (line < mRawText.size() && line < mLines.size() && !mWrap)
	{
		mRawText.at(line) = msg;
		mLines.at(line) = msg;
	}
#ifdef OPTIMIZED
	Screen::Instance()->Update();
#endif
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
	if (!mWrap || maxWidth == 0) //just clone (TODO: SHOULD just use the original list, but we need to add that check everywhere
	{
		_addLine(line);
		return;
	}

	if (!mFont) return;

	string color, lastColor;
	string temp;
	uShort w, linecount = 0;
	while (true) //TODO: This code is fucking horrible. Rewrite the whole thing.
	{
		if (line.empty()) break;

		if (line.find("\\c", 0) == 0)
		{
			color = line.substr(0, 5); //current color to add text as
			temp += color;
			line.erase(0, 5);
		}
		else
		{
			w = mFont->GetWidth( stripCodes(temp) );
			if (w >= maxWidth) //Add a new line and eat away at the old
			{
				_addLine( lastColor + temp );
				lastColor = color; //color of the end of last line inserted
				temp.clear();
				linecount++;
			}
			else //add a character to temp and eat it from the message
			{
				temp += line.at(0);
				line.erase(0, 1);
			}
		}
	}
	//add rest (should be less than a max line length)
	_addLine( lastColor + temp );
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

	uShort height = GetNumberOfLinesVisible();
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

void Multiline::ClickUrl(uShort line) //TODO: Better version of this.
{
	if (line >= mLines.size() || mUrls.empty()) return;
	uShort start;
	sShort t;
	string s;

	int i = 0;
	while (g_sSupportedUrls[i])
	{
		if (mLines.at(line).find(g_sSupportedUrls[i], 0) != string::npos)
		{
			s = g_sSupportedUrls[i];
			break;
		}
		++i;
	}
	
	if (s.empty()) //not a valid url type
		return;

	start = mLines.at(line).find(s, 0);
	string url;

	t = line;
	//keep going until there's no more lines or a space
	while (true)
	{
		if (t >= mLines.size()) //no more lines
			break;
		if (mLines.at(t).find(' ', start) == string::npos) //keep going
		{
			url += mLines.at(t).substr(start);
		}
		else //we have a space
		{
			url += mLines.at(t).substr(start, mLines.at(t).find(' ', start) - start);
			break; //grabbed it
		}
		start = 0;
		t++;
	}
	url = stripCodes(url); //erase any \c that may have been inserted

	//now we have a url, however it may have some shit tagged on (from the next line)
	//soooo, compare it to the collected url list
	for (t = mUrls.size() - 1; t > -1; t--) //reverse cuz most recent are @ the bottom
	{
		if (mUrls.at(t) == url.substr(0, mUrls.at(t).length())) //compares length along with contents
		{
			new OpenUrl(mUrls.at(t));
			return;
		}
	}
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
