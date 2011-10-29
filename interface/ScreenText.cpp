
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


#include "ScreenText.h"

const int HORIZONTAL_INTERVAL = 20;
const int HORIZONTAL_SPEED = 20;
const int LETTER_CROWDING = 6;
const int SPACE_SIZE = 20;
const int SMASH_IN_INTERVAL = 20;
const int SMASH_IN_SPEED = 50;
const int WAIT_INTERVAL = 800;

ScreenText::ScreenText(string text, color rgb, animationType type, int y)
{
	gui->Add(this);
	mId = lowercase(text);
	mAnimType = type;
	
	Image* img = resman->LoadImg("assets/lettering.png");
	mImage = img->Clone(true);
	resman->Unload(img); //dereference it, since we no longer touch the original

	// Determine letter sizes based on pink pixels found. 
	// Create an array to store this info for quicker access later
	color c;
	for (int x = 0; x < mImage->Width(); ++x)
	{
		c = mImage->GetPixel(x, 0);
		if (c.r == 255 && c.g == 0 && c.b == 255) //pink!
			mBorders.push_back(x);
	}

	// Colorize our version to suit our needs
	mImage->ColorizeGreyscale(rgb);
	
	// Calculate size
	int width = 0;
	rect r;
	for (int i = 0; i < mId.length(); ++i)
	{
		r = GetCharacterRect(mId.at(i));
		width += r.w - LETTER_CROWDING;
	}
	
	 // center on the y coordinate
	SetPosition( rect(0, y - mImage->Height() / 2, width, mImage->Height()) );

	Animate();
}

ScreenText::~ScreenText()
{
	timers->RemoveMatchingUserData(this);
	
	// Blindly throw out an event and hope someone catches it
	//	And by someone, I mean Lua
	MessageData md("SCREENTEXT");
	md.WriteString("text", mId);
	messenger.Dispatch(md);
}

uShort timer_LtoR(timer* t, uLong ms)
{
	ScreenText* st = (ScreenText*)t->userData;
	if (!st)
		return TIMER_DESTROY;
		
	rect r = st->GetPosition();
	r.x += HORIZONTAL_SPEED;
	st->SetPosition(r);
	
	if (r.x > gui->Width())
	{
		st->Die();
		return TIMER_DESTROY;
	}

	return TIMER_CONTINUE;
}

uShort timer_RtoL(timer* t, uLong ms)
{
	ScreenText* st = (ScreenText*)t->userData;
	if (!st)
		return TIMER_DESTROY;
		
	rect r = st->GetPosition();
	r.x -= HORIZONTAL_SPEED;
	st->SetPosition(r);
	
	if (r.x + st->Width() < 0)
	{
		st->Die();
		return TIMER_DESTROY;
	}

	return TIMER_CONTINUE;
}

uShort timer_PostWait(timer* t, uLong ms)
{
	timers->Add("", HORIZONTAL_INTERVAL, false, timer_RtoL, NULL, t->userData);
	return TIMER_DESTROY;	
}

uShort timer_SmashIn(timer* t, uLong ms)
{
	ScreenText* st = (ScreenText*)t->userData;
	if (!st)
		return TIMER_DESTROY;

	if (st->mCurrentLetterX - SMASH_IN_SPEED <= st->mStopX)
	{
		st->mCurrentLetterX = st->mStopX;
			
		++st->mCurrentLetterIndex;
		
		// skip spaces
		if (st->mCurrentLetterIndex < st->mId.length() 
			&& st->mId.at(st->mCurrentLetterIndex) == ' ')
			++st->mCurrentLetterIndex;
		
		// finished all the letters
		if (st->mCurrentLetterIndex >= st->mId.length())
		{
			timers->Add("", WAIT_INTERVAL, false, timer_PostWait, NULL, st);
			
			return TIMER_DESTROY;
		}
		else //fly next letter in
		{
			st->mCurrentLetterX = gui->Width();
			st->mStopX += st->GetCharacterRect(st->mId.at(st->mCurrentLetterIndex)).w - LETTER_CROWDING;
		}
	}
	else
	{
		st->mCurrentLetterX -= SMASH_IN_SPEED;
	}
	
	return TIMER_CONTINUE;
}

void ScreenText::Animate()
{
	rect r = GetPosition();
	switch (mAnimType)
	{
		case LEFT_TO_RIGHT:
			timers->Add("", HORIZONTAL_INTERVAL, false, timer_LtoR, NULL, this);
			r.x = 0 - Width();
			SetPosition(r);
			break;
		case RIGHT_TO_LEFT:
			timers->Add("", HORIZONTAL_INTERVAL, false, timer_RtoL, NULL, this);
			r.x = gui->Width();
			SetPosition(r);
			break;
		case SMASH_IN:
			timers->Add("", SMASH_IN_INTERVAL, false, timer_SmashIn, NULL, this);
			r.x = gui->Width() / 2 - Width() / 2;
			SetPosition(r);
			mCurrentLetterIndex = 0;
			mCurrentLetterX = gui->Width();
			mStopX = GetPosition().x;
			break;
		default: break;
	}
}

void ScreenText::Render()
{
	Image* scr = Screen::Instance();
	rect r = GetScreenPosition();
	rect sr;
	int i;
	
	if (mAnimType == SMASH_IN)
	{
		i = 0;
		while (i < mCurrentLetterIndex && i < mId.length())
		{
			sr = GetCharacterRect(mId.at(i));
			if (mId.at(i) != ' ')
				mImage->Render(scr, r.x, r.y, sr);
				
			r.x += sr.w - LETTER_CROWDING;
			
			++i;	
		}
		
		// draw the fly-in character if we have one
		if (mCurrentLetterIndex < mId.length())
		{
			sr = GetCharacterRect(mId.at(mCurrentLetterIndex));
			if (mId.at(mCurrentLetterIndex) != ' ')
				mImage->Render(scr, mCurrentLetterX, r.y, sr);
		}
	}
	else
	{
		for (i = 0; i < mId.length(); ++i)
		{
			sr = GetCharacterRect(mId.at(i));
	
			if (mId.at(i) != ' ')
				mImage->Render(scr, r.x, r.y, sr);
				
			r.x += sr.w - LETTER_CROWDING;
		}
	}
}

rect ScreenText::GetCharacterRect(char c)
{
	int index;
	rect r;
	r.y = 0;
	r.h = mImage->Height();

	if (c == '!') // special case
	{
		r.x = mBorders.at(mBorders.size()-1) + 1;
		r.w = mImage->Width() - r.x;
	}
	else if (c == ' ')
	{
		r.x = 0;
		r.w = SPACE_SIZE;	
	}
	else if (c >= 'a' && c <= 'z')
	{
		index = c - 'a';
		if (index > 0)
			r.x = mBorders.at(index - 1) + 1;
		else
			r.x = 0;
			
		r.w = mBorders.at(index) - r.x;
	}

	return r;
}





