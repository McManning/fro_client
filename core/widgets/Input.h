
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
	
	void SetActive(bool b) { mReadOnly = b; Widget::SetActive(b); };
	
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
	
	/*	Use this instead of accessing mImage directly */
	void SetImage(string file);
	
	//VARIABLES AND SUCH -> GO!
	string mCharacterMask; //list of allowed characters

	std::vector<string> mHistory;
	byte mHistoryPos;

	bool mIsPassword;
	bool mAllowSpecialKeys; //arrow keys, end, home, etc keys.
	bool mReadOnly;
	uShort mMaxLength; //max input length
	
	color mHighlightBackground;

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
