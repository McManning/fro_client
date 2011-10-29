
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


#ifndef _INPUT_H_
#define _INPUT_H_

#include "Frame.h"

#define INPUT_JUMPBACK_NUM 7
#define INPUT_PASSWORD_CHAR 'x'
#define MAX_INPUT_HISTORY 100

class Font;
class Image;
class Input: public Widget {
  public:
	Input(Widget* wParent, string sId, rect rPosition, string sMask, 
				uShort uMaxLen, bool bSpecialKeys, void (*callbackOnEnter)(Input*));
	~Input();

	void Render();
	
	void Event(SDL_Event* event);
	
	//BEGIN INPUT-WIDGET-ONLY STUFF
	
	void AddText(string msg);
	void Clear();

	void Cut();
	void Copy();
	void Paste();
	void SelectAll();
	
	//Opens up a color picker menu that we can select a color from.
	//Will disable this input and force focus on the picker until it is done.
	void PasteColor();
	
	/*	Will activate onEnterCallback and add to history */
	void PushInput(string msg);
	
	string GetText() { return mText; };
	void SetText(string msg) { Clear(); AddText(msg); };

	void Changed();
	
	//void SetAlign(byte align) { mHorizontalAlign = align; Changed(); };
	
	void AddToHistory(string msg);
	
	int CaretPosToPixel();
	void SetCaretPos(int rx, int ry);
	void SetSelection(int start, int end);
	
	void RecalculatePixelX();
	
	bool IsMenuEnabled() const { return mMenuEnabled && !mIsPassword; };
	void SetMenuEnabled(bool b) { mMenuEnabled = b; };
	
	//VARIABLES AND SUCH -> GO!
	string mCharacterMask; //list of allowed characters

	std::vector<string> mHistory;
	byte mHistoryPos;

	bool mIsPassword;
	bool mAllowSpecialKeys; //arrow keys, end, home, etc keys.
	bool mReadOnly;
	uShort mMaxLength; //max input length

	Image* mTextImage;
	
	void (*onEnterCallback)(Input*);
	void (*onChangeCallback)(Input*);
	
  protected:
	void _insertText(string msg);
	void _updateText(); //redraws subsurf w/ text
	
	void _renderPassword(Image* scr, rect& r);
	void _renderText(Image* scr, rect& r);
	
	bool mNeedUpdate;
	bool mClickedOnce; //double clicking
	bool mDrawCaret; //caret blinking shit.
	string mText; //the input itself
	uLong mLastBlink;
	bool mHasFocusCheck;

	bool mMenuEnabled;

	bool mSelecting;
	int mSelectionStart, mSelectionEnd, mCaretPos, mPixelX;
};

/*	
//Right click menu for Input widgets
class InputMenu : public Frame 
{
  public:
	InputMenu(Input* parent);
	~InputMenu();
	
	void Event(SDL_Event* event);
};
*/

//helper function
string getInputText(Widget* parent, string id);

#endif //_INPUT_H_
