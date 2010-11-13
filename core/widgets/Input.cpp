
#include "Input.h"
#include "Button.h"
#include "ColorPicker.h"
#include "RightClickMenu.h"
#include "../GuiManager.h"
#include "../Image.h"
#include "../Screen.h"
#include "../FontManager.h"
#include "../ResourceManager.h"
#include "../io/FileIO.h"
#include "Console.h"

const int MAX_PASSWORD_SYMBOLS = 10;

string getInputText(Widget* parent, string id)
{
	Input* i = (Input*)parent->Get(id, false, WIDGET_INPUT);
	if (i)
		return i->GetText();
	else
		return "";
}

void callback_inputMenuCut(RightClickMenu* m, void* userdata)
{
	Input* i = (Input*)userdata;
	i->SetKeyFocus();
	i->Cut();
}

void callback_inputMenuCopy(RightClickMenu* m, void* userdata)
{
	Input* i = (Input*)userdata;
	i->SetKeyFocus();
	i->Copy();
}

void callback_inputMenuPaste(RightClickMenu* m, void* userdata)
{
	Input* i = (Input*)userdata;
	i->SetKeyFocus();
	i->Paste();
}

void callback_inputMenuSelectAll(RightClickMenu* m, void* userdata)
{
	Input* i = (Input*)userdata;
	i->SetKeyFocus();
	i->SelectAll();
}

void callback_inputMenuPasteColor(RightClickMenu* m, void* userdata)
{
	Input* i = (Input*)userdata;
	i->SetKeyFocus();
	i->PasteColor();
}

/*
InputMenu::InputMenu(Input* parent)
	: Frame(parent, "inputmenu", rect())
{

	Button* b;
	
	uShort y = 0;
	uShort x = 0;
	
	uShort buttonSize = 18;

	b = new Button(this, "cut", rect(x, y, buttonSize, buttonSize), "", callback_inputMenuCut);
		b->mHoverText = "Cut (Ctrl+X)";
		b->SetImage("assets/gui/input_cut.png");
	x += buttonSize;

	b = new Button(this, "copy", rect(x, y, buttonSize, buttonSize), "", callback_inputMenuCopy);
		b->mHoverText = "Copy (Ctrl+C)";
		b->SetImage("assets/gui/input_copy.png");
	x += buttonSize;
			
	b = new Button(this, "paste", rect(x, y, buttonSize, buttonSize), "", callback_inputMenuPaste);
		b->mHoverText = "Paste (Ctrl+V)";
		b->SetImage("assets/gui/input_paste.png");
	x += buttonSize;
				
	b = new Button(this, "selectall", rect(x, y, buttonSize, buttonSize), "", callback_inputMenuSelectAll);
		b->mHoverText = "Select All (Ctrl+A)";
		b->SetImage("assets/gui/input_selectall.png");
	x += buttonSize;
			
	b = new Button(this, "color", rect(x, y, buttonSize, buttonSize), "", callback_inputMenuPasteColor);
		b->mHoverText = "Paste Color (Ctrl+B)";
		b->SetImage("assets/gui/input_color.png");
	x += buttonSize;

	b = new Button(this, "close", rect(x, y, buttonSize, buttonSize), "", callback_closeFrame);
		b->mHoverText = "Close Menu";
		b->SetImage("assets/gui/input_close.png");
	x += buttonSize;

	rect r;
	r.x = 1;
	r.y = 1;
	r.w = x;
	r.h = buttonSize;

	SetPosition(r);
}

InputMenu::~InputMenu()
{

}

void InputMenu::Event(SDL_Event* event)
{
	Frame::Event(event);
	
	//If the user hits any keys, destroy this info frame
	if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP)
		Die();
}
*/

/////////////////////////////////////////////////////////////

Input::Input(Widget* wParent, string sId, rect rPosition, string sMask, 
				uShort uMaxLen, bool bSpecialKeys, void (*callbackOnEnter)(Input*))
{
	mNeedUpdate = false;
	mIsPassword = false;
	mMaxLength = uMaxLen;
	mType = WIDGET_INPUT;
	mFont = fonts->Get();

	mHistoryPos = 0;

	SetSelection(0, 0);
	SetMenuEnabled(true);
	
	mCaretPos = 0;
	mPixelX = 0;
	mSelecting = false;
	mAllowSpecialKeys = bSpecialKeys;
	mDrawCaret = true;
	mLastBlink = 0;
	mReadOnly = false;
	mClickedOnce = false;
	onEnterCallback = callbackOnEnter;
	onChangeCallback = NULL;
	mTextImage = NULL;

	//we need to know mouse positions outside our widget for highlight dragging
	gui->AddGlobalEventHandler(this);
	
	mHighlightBackground = HIGHLIGHT_COLOR;
	mId = sId;
	mCharacterMask = sMask;

	mImage = resman->LoadImg("assets/gui/input_bg.png");
	
	SetPosition(rPosition);
	
	if (wParent)
		wParent->Add(this);
	
}

Input::~Input() 
{
	mHistory.clear();
	resman->Unload(mTextImage);
}

void Input::Render() 
{
	Image* scr = Screen::Instance();
	rect oldclip;
	rect r = GetScreenPosition();

	if (mNeedUpdate) { //rerender text if necessary
		_updateText();
		mNeedUpdate = false;
	}

	//Do a little bit of calculating
	if (gui->GetTick() > mLastBlink && !mReadOnly) 
	{
		mLastBlink = gui->GetTick() + 600;
		mDrawCaret = !mDrawCaret;
	}
	
	if (!HasKeyFocus() && mSelectionStart != mSelectionEnd)
	{
		if (!Get("inputmenu") || !Get("inputmenu")->HasKeyFocus())
			SetSelection(0, 0);
	}

	if (mImage)
		mImage->RenderBox(scr, rect(0, CalculateImageOffset(15), 5, 5), r);

	//shrink the actual window a bit so we can get a 1px border
	r.w -= 4;
	r.x += 2;
	//r.h -= 2;
	r.y += (r.h / 2 - mFont->GetHeight() / 2);

	if (mIsPassword)
	{
		_renderPassword(scr, r);	
	}
	else
	{
		// TODO: Good calculations that don't need clipping!
		oldclip = scr->GetClip();
		scr->SetClip(r);
		
		_renderText(scr, r);
		
		scr->SetClip(oldclip);	
	}

	Widget::Render();
}

void Input::_renderPassword(Image* scr, rect& r)
{
	if (mText.empty())
		return;
		
	//render the visible text
	mTextImage->Render(scr, r.x, r.y-1, 	
						rect(mPixelX-2, 0, r.w, r.h)
					);
}

void Input::_renderText(Image* scr, rect& r)
{
	int x, w, selStart, selEnd;
	
	if (mText.empty() || !mTextImage)
		return;

	//r.y += ((r.h / 2) - (mTextImage->Height() / 2));
		
	//render the highlight if we got it
	if (mSelectionEnd != mSelectionStart && mFont) 
	{
		selStart = mSelectionStart;
		selEnd = mSelectionEnd;
		if (selStart > selEnd)
			std::swap(selStart, selEnd);
			
		x = mFont->GetWidth(mText.substr(0, selStart));
		x -= mPixelX;

		//x is now the distance from the start to render text to the selection end.
		if (x < 0) 
			x = 0; //don't need to highlight shit not visible
			
		w = mFont->GetWidth(mText.substr(selStart, selEnd - selStart));
		if (x + w > r.w) 
			w = r.w - x;
		
		//now have a clipped width~ Now draw the rect.
		scr->DrawRect( rect(r.x + x, r.y, w, mTextImage->Height()), mHighlightBackground );
	}
	
	//render the visible text
	mTextImage->Render(scr, r.x, r.y, 	
					rect(mPixelX, 0, r.w, r.h)
				);

	x = r.x + (CaretPosToPixel() - mPixelX);// - 1;

	if (x < r.x + r.w && mDrawCaret == true
		&& !mReadOnly && HasKeyFocus() && x > r.x) 
	{
		//render the caret (Same color as text)
		scr->DrawRect(rect(x-1, r.y /*+ 1*/, 2, mTextImage->Height()/* - 2*/), mFontColor);
	}
}

void Input::RecalculatePixelX() 
{
	//grab what section to render
	int c = CaretPosToPixel();

	if (mCaretPos == 0) 
	{
		mPixelX = 0;
	} 
	else if (mPixelX > c) //caret is too far left
	{
		//uShort i = (mCaretPos - INPUT_JUMPBACK_NUM < 0) ? 0 : mCaretPos - INPUT_JUMPBACK_NUM;
		//mPixelX = c - mFont->GetWidth(mText.substr(i, INPUT_JUMPBACK_NUM));
		if (mCaretPos-1 < 0) 
		{
			mPixelX = 0;
		}
		else
		{
			if (mIsPassword)
				mPixelX -= mText.substr(mCaretPos-1, 1).length() * 16;
			else
				mPixelX -= mFont->GetWidth(mText.substr(mCaretPos-1, 1));
		}
	} 
	else if (mPixelX + mPosition.w < c) //caret is too far right
	{ 
		mPixelX = c - mPosition.w + 7; //Compensate for the cursor-off-the-edge issue 
	}
}

//Convert pixel coordinates to a position between letters
void Input::SetCaretPos(int rx, int ry)
{
	if (!mFont) return;
	int mx = GetScreenX();

	mDrawCaret = false; //false because update() will fix it
	mLastBlink = 0;

	for (int i = 0; i < mText.length(); i++) 
	{
		if (mFont->GetWidth(mText.substr(0, i).c_str()) > rx - mx + mPixelX) 
		{
			//TODO: Bug: can't set between the LAST and LAST-1 character.
			if (i == 0)
				mCaretPos = i;
			else
				mCaretPos = i-1;
			return;
		}
	}

	mCaretPos = mText.length();
	
	FlagRender();
}

//Convert position between letters right back to a pixel pos
int Input::CaretPosToPixel() 
{
	if (!mFont) 
		return 0;
	else if (mIsPassword)
		return mText.substr(0, mCaretPos).length() * 16;
	else
		return mFont->GetWidth(mText.substr(0, mCaretPos));
}

void Input::SetSelection(int start, int end) 
{
    //Clamp the selection to the size of the string
    if(start > mText.size()) 
		start = mText.size();
		
    if(end > mText.size()) 
		end = mText.size();
		
	if (start < 0) 
		start = 0;
		
	if (end < 0) 
		end = 0;
		
    mSelectionStart = start;
    mSelectionEnd = end;
    
    FlagRender();
}

void Input::Event(SDL_Event* event)
{
	switch (event->type)
	{
		case SDL_MOUSEMOTION: {
			if (HasMouseFocus())
			{
				mClickedOnce = false;
				rect r = GetScreenPosition();
				
				if (mSelecting)
				{
					if (event->motion.x < r.x)
					{
						mCaretPos--;
						if (mCaretPos < 0)
							mCaretPos = 0;
						SetSelection(mSelectionStart, mCaretPos);
					} 
					else if (event->motion.x > r.x + r.w)
					{
						mCaretPos++;
						if (mCaretPos > mText.length()) 
							mCaretPos = mText.length();
						SetSelection(mSelectionStart, mCaretPos);
					} 
					else 
					{
						SetCaretPos(event->motion.x, event->motion.y);
						SetSelection(mSelectionStart, mCaretPos);
					}
					RecalculatePixelX();
				}
				FlagRender();
			}
			else if (DidMouseLeave())
			{
				FlagRender();	
			}
			
		} break;
		case SDL_MOUSEBUTTONUP: {
			mSelecting = false;
		} break;
		case SDL_MOUSEBUTTONDOWN: {
			if (event->button.button == SDL_BUTTON_LEFT && HasMouseFocus())
			{
				/*if (HasKeyFocus())
				{
					InputMenu* im = (InputMenu*)Get("inputmenu");
					if (im)
						im->Die();	
				}*/
				if (!mIsPassword)
				{
					SetCaretPos(event->button.x, event->button.y);
					mSelecting = true;
					SetSelection(mCaretPos, mCaretPos);
				}
			}
			else if (event->button.button == SDL_BUTTON_RIGHT && HasMouseFocus() && !mReadOnly)
			{
				if (IsMenuEnabled())
				{
					//new InputMenu(this);	
					RightClickMenu* m = new RightClickMenu();
						m->AddOption("Cut", callback_inputMenuCut, this);
						m->AddOption("Copy", callback_inputMenuCopy, this);
						m->AddOption("Paste", callback_inputMenuPaste, this);
						m->AddOption("Paste Color", callback_inputMenuPasteColor, this);
						m->AddOption("Select All", callback_inputMenuSelectAll, this);
				}
			}
		} break;
		case SDL_KEYDOWN: {
			int selStart = mSelectionStart;
			int selEnd = mSelectionEnd;
			if (selStart > selEnd)
				std::swap(selStart, selEnd);

			if (!mReadOnly && HasKeyFocus())
			{
				if (event->key.keysym.mod & KMOD_CTRL) //handle ctrl+? shortcuts
				{
					switch (event->key.keysym.sym) 
					{
						case SDLK_v:
							Paste();
							break;
						case SDLK_x:
							Cut();
							break;
						case SDLK_c:
							Copy();
							break;
						case SDLK_a:
							SelectAll();
							break;
						case SDLK_b:
							PasteColor();
							break;
						default: break;	
					}
				}
				else if (event->key.keysym.mod & KMOD_ALT)
				{
					//ignore	
				}
				else //regular input
				{
					switch (event->key.keysym.sym)
					{
						case SDLK_BACKSPACE: {
							if (selStart != selEnd) 
							{
								mText.erase(mText.begin() + selStart,
												mText.begin() + selEnd);
								mCaretPos = selStart;
								SetSelection(0, 0);
							} 
							else if (!mText.empty() && mCaretPos > 0) 
							{
								mCaretPos--;
								mText.erase(mText.begin() + mCaretPos);	
							}
						} break;
						case SDLK_DELETE: {
							if (selStart != selEnd) 
							{
								mText.erase(mText.begin() + selStart,
												mText.begin() + selEnd);
								mCaretPos = selStart;
								SetSelection(0, 0);
							} 
							else if (!mText.empty() && mCaretPos > 0) 
							{
								if (mCaretPos < mText.size())
									mText.erase(mText.begin() + mCaretPos);	
							}
						} break;
						case SDLK_LEFT: {
							if (mAllowSpecialKeys && mCaretPos > 0)
							{
								mCaretPos--;
								SetSelection(0, 0);
								mLastBlink = 0;
								mDrawCaret = false;
							}
						} break;
						case SDLK_RIGHT: {
							if (mAllowSpecialKeys && mCaretPos < mText.length())
							{
								mCaretPos++;
								SetSelection(0, 0);
								mLastBlink = 0;
								mDrawCaret = false;
							}
						} break;
						case SDLK_UP: //fall down
							if (!mAllowSpecialKeys) break;
						case SDLK_PAGEUP: {
							if (mHistoryPos > 0) {
								mHistoryPos--;
								if (!mHistory.empty()) {
									Clear();
									AddText(mHistory.at(mHistoryPos));
								}
							}
						} break;
						case SDLK_DOWN: //fall down
							if (!mAllowSpecialKeys) break;
						case SDLK_PAGEDOWN: {
							if (mHistoryPos < mHistory.size()-1) {
								mHistoryPos++;
								if (!mHistory.empty()) {
									Clear();
									AddText(mHistory.at(mHistoryPos));
								}
							} else { //set it to blank
								mHistoryPos = mHistory.size();
								Clear();
							}
						} break;
						case SDLK_KP_ENTER:
						case SDLK_RETURN: {
							AddToHistory(mText);
							if (onEnterCallback)
								onEnterCallback(this);
						} break;
						case SDLK_TAB: {
							TabToNextSibling();
						} break;
						//IGNORE KEYS
						case SDLK_ESCAPE:
						case SDLK_PRINT:
							break;
						default: {
							//if (isalnum(key) || ispunct(key) || isspace(key)) //only accept ascii for now
						//	if (isgraph(event->key.keysym.unicode) || isspace(event->key.keysym.unicode)) //isgraph should only return 1 if it's any PRINTABLE character (accents support?)
						//	{
								if (event->key.keysym.unicode != 0)
									AddText((char*)&event->key.keysym.unicode);
						//	}
						/*	const int INTERNATIONAL_MASK = 0xFF80, UNICODE_MASK = 0x7F;
							if (event->key.keysym.unicode != 0)
							{
								char c;
								if( ( event->key.keysym.unicode & INTERNATIONAL_MASK ) == 0 )
									c = static_cast<char>(event->key.keysym.unicode & UNICODE_MASK);
								else
									c = '$';
								string s;
								s += c;
								AddText(s);
							}*/
						} break;
					}
				}
				
				Changed();
			}
		} break;
		default: break;
	}
	
	Widget::Event(event);
}

void Input::AddToHistory(string msg) 
{
	//If the message has content, add it up
	if (!msg.empty() && (msg.find_first_not_of(' ', 0) != string::npos)) 
	{ 
		mHistory.push_back(msg);
		if (mHistory.size() > MAX_INPUT_HISTORY)
			mHistory.erase(mHistory.begin());
			
		mHistoryPos = mHistory.size();
	}
}

void Input::_updateText() 
{
	int i, s;
	color c;
	
	//TODO: we should probably render the selected text outline & color change here instead of during rendering
	RecalculatePixelX();

	if (mText.empty() || !mFont) return;
	resman->Unload(mTextImage);
/*	TODO: Password mode is screwing up calculations
	string msg;
	if (mIsPassword) msg.resize(mText.length(), INPUT_PASSWORD_CHAR);
	else msg = mText;
*/
	
	if (!mIsPassword)
	{
		c = mFontColor; 
		if (!IsActive() || mReadOnly)
			c.r = c.g = c.b = 128;
		mTextImage = resman->ImageFromSurface( mFont->RenderToSDL(mText.c_str(), c) );
	}
	else
	{
		mTextImage = resman->NewImage(mText.length() * 16, 16, color(255,0,255), false);
		for (i = 0; i < mText.length(); ++i)
		{
			// draw a symbol for each character, chosen based off that character (loosely)
			s = mText.at(i) - 20;
			while (s >= MAX_PASSWORD_SYMBOLS)
				s -= MAX_PASSWORD_SYMBOLS;
			
			if (mImage)
				mImage->Render(mTextImage, i * 16, rnd(0, 3), rect((s>9)?29:15, 0+s*14, 14, 14));
		}	
	}	
}

//internal call, adds no matter what. Text must pass the checks
//before calling this to finalize the add.
void Input::_insertText(string msg) 
{
	int selStart = mSelectionStart;
	int selEnd = mSelectionEnd;
	
	if (selStart > selEnd)
		std::swap(selStart, selEnd);
		
	if (selStart != selEnd) 
	{
		mText.erase(mText.begin() + selStart,
                		mText.begin() + selEnd);
		mCaretPos = selStart;
		SetSelection(0, 0);
	}

	if (mMaxLength > 0)
	{
		if (mText.length() < mMaxLength)
			msg = msg.substr(0, mMaxLength - mText.length());
		else
			msg.clear(); //invalid
	}

	mText.insert(mCaretPos, msg);
	mCaretPos += msg.length();
	mDrawCaret = false; //false because update() will fix it. Wait what? TODO: Check into why this is necessary again?
	mLastBlink = 0;

	Changed();	
}

void Input::AddText(string msg) 
{
    //if we find any new lines, erase everything after the first
    //TODO: Maybe we should convert to space? Or send back some sort of warning that content was clipped?
    if (msg.find("\n", 0) != string::npos) 
	{
        msg.erase(msg.find("\n", 0));
    }

    //check if the characters are allowed in the mask, if not, don't add any
    if (!mCharacterMask.empty()) 
	{
		for (uShort i = 0; i < msg.length(); i++) 
		{
			bool found = false;
			for (uShort c = 0; c < mCharacterMask.length(); c++) 
			{
				if (mCharacterMask.at(c) == msg.at(i)) 
				{
					found = true; 
					break; 
				}
			}
			if (!found) return;
		}
	}
    
	_insertText(msg);

}

void Input::Clear() 
{
	mText.clear();
	mCaretPos = 0;
	SetSelection(0, 0);
	Changed();
	//No need to delete mTextImage here, if mText is clear, mTextImage won't render. 
}

void Input::Changed() 
{
	mNeedUpdate = true; 

	if (onChangeCallback)
		onChangeCallback(this);
}

void Input::PushInput(string msg)
{
	SetText(msg);
	AddToHistory(msg);
	if (onEnterCallback)
		onEnterCallback(this);
}

void Input::Cut()
{
	int selStart = mSelectionStart;
	int selEnd = mSelectionEnd;
	if (selStart > selEnd)
		std::swap(selStart, selEnd);

	if (selStart != selEnd) 
	{
		sendStringToClipboard(mText.substr(selStart, selEnd - selStart));
		mText.erase(mText.begin() + selStart,
						mText.begin() + selEnd);
		mCaretPos = selStart;
		SetSelection(0, 0);
	}
	Changed();
}

void Input::Copy()
{
	int selStart = mSelectionStart;
	int selEnd = mSelectionEnd;
	if (selStart > selEnd)
		std::swap(selStart, selEnd);

	sendStringToClipboard(mText.substr(selStart, selEnd - selStart));
	Changed();
}

void Input::Paste()
{
	AddText(getClipboardString());
	Changed();
}

void Input::SelectAll()
{
	SetSelection(mText.length(), 0);
	mCaretPos = mText.length();
	Changed();
}

void Input::PasteColor()
{
	ColorPicker* c = new ColorPicker(this);
	gui->Add(c);
}

void Input::SetImage(string file)
{
	resman->Unload(mImage);
	mImage = resman->LoadImg(file);
}
