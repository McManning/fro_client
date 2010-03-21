
#include "Console.h"
#include "Input.h"
#include "Button.h"
#include "Label.h"
#include "Scrollbar.h"
#include "Multiline.h"
#include "../Screen.h"
#include "../GuiManager.h"
#include "../ResourceManager.h"
#include "../FontManager.h"
#include "../io/XmlFile.h"

Console* console;

void callback_consoleInput(Input* i)
{
	Console* c = (Console*)(i->GetParent());
	
	if (i->GetText().empty())
		return;
	
	string cmd = lowercase( i->GetText().substr(0, i->GetText().find(" ")) );
	
	//global console commands
	if (cmd == "/save")
	{
		c->SaveText();
	}
	else if (cmd == "/clear")
	{
		if (c->mOutput)
			c->mOutput->Clear();
	}
	else if (cmd == "/exit")
	{
		appState = APPSTATE_CLOSING;	
	}
	else if (cmd == "/ss")
	{
		gui->Screenshot();	
	}
	else
	{
		c->DoCommand(i->GetText());	
	}
	i->Clear();	
}

void callback_consoleExit(Button* b)
{
	if (b->GetParent())
		b->GetParent()->Die();
}

Console::Console(string id, string title, string imageFile, string savePrefix, 
				bool hasExit, bool hasInput)
{
	mFont = fonts->Get();
	mId = id;
	mType = WIDGET_FRAME;
	mMoveable = true;
	SetSizeable(true);
	//mResizingBackgroundColor = color(0,64,128,100);
	mResizingBorderColor = color(0,64,200);
	mBackgroundColor = color(0,0,0,config.GetParamInt("console", "alpha"));
	mBackground = NULL;
	mShowTimestamps = true;
	mOutput = NULL;
	mSavePrefix = savePrefix;
	
	SetPosition( rect(gui->Width()-300,gui->Height()-176,300,176) );

	if (hasInput)
	{			
		mBottomImage = makeImage(this, "btm", imageFile, rect(0,16,26,26), rect(0,0,0,26), 
								WIDGETIMAGE_HEDGE, false, false);
								
		mInput = new Input(this, "input", rect(26,0,0,20), "", 120, true, callback_consoleInput);
			makeImage(mInput, "", imageFile, rect(104,0,2,20), rect(0,0,0,0), 
							WIDGETIMAGE_HEDGE, false, false);
		mInput->mHighlightBackground = HIGHLIGHT_COLOR;
		mInput->mFontColor = color(255,255,255);
		
		mSizer = makeImage(this, "sizer", imageFile, rect(78,0,26,26), rect(0,0,26,26), 
						WIDGETIMAGE_FULL, false, false);
	}
	else
	{
		//add short bottom								
		mBottomImage = makeImage(this, "btm", imageFile, rect(0,42,26,16), rect(0,0,0,16), 
									WIDGETIMAGE_HEDGE, false, false);
		mInput = NULL;
		
		//small sizer
		mSizer = makeImage(this, "sizer", imageFile, rect(78,42,16,16), rect(0,0,16,16), 
						WIDGETIMAGE_FULL, false, false);
	}
	
	mTopImage = makeImage(this, "top", imageFile, rect(0,0,26,16), rect(0,0,0,16), 
							WIDGETIMAGE_HEDGE, false, false);
				

	mOutput = new Multiline();
	mOutput->mFont = fonts->Get("", 0, TTF_STYLE_BOLD); //load default font, but bold.
	mOutput->mFontColor = color(255,255,255);

	mOutput->mScrollbar = new Scrollbar();	
	rect rPosition = rect(0, mTopImage->mDst.h, 20, mOutput->Height());
	
	mOutput->mScrollbar->mFont = fonts->Get();
	mOutput->mScrollbar->onValueChangeCallback = callback_multilineScrollbar;

	rPosition.w = 18;
	rPosition.h = 18;
		
	mOutput->mScrollbar->mBackgroundImage = makeImage(mOutput->mScrollbar, "bg", imageFile, rect(78,26,18,5), 
												rect(0,0,18,0), WIDGETIMAGE_VEDGE, false, false);
	mOutput->mScrollbar->mTabImage = makeImage(mOutput->mScrollbar, "tab", imageFile, rect(96,26,18,15), 
										rect(0,0,18,15), WIDGETIMAGE_FULL, false, false);
	
	mOutput->mScrollbar->mValueDown = new Button(mOutput->mScrollbar, "down", rect(0,0,18,14), "", callback_scrollbarButtonDown);
		makeImage(mOutput->mScrollbar->mValueDown, "", imageFile, rect(114,0,18,14), 
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);

	mOutput->mScrollbar->mValueUp = new Button(mOutput->mScrollbar, "up", rect(0,0,18,14), "", callback_scrollbarButtonUp);
		makeImage(mOutput->mScrollbar->mValueUp, "", imageFile, rect(132,0,18,14), 
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);			
				
	mOutput->mScrollbar->SetPosition(rPosition);
	mOutput->Add(mOutput->mScrollbar);
	mOutput->SetPosition( rect(0, 16, mPosition.w, mPosition.h - 42) );
	Add(mOutput);
	
	mExit = NULL;
	if (hasExit)
	{
		mExit = new Button(this, "exit", rect(0, 2, 12, 12), "", callback_consoleExit);
			makeImage(mExit, "", imageFile, rect(150,0,12,12), 
						rect(0,0,12,12), WIDGETIMAGE_FULL, true, false);
	}
		
	mTitle = NULL;
	if (!title.empty())
	{
		mTitle = new Label(this, "title", rect(5, 1), title);
		//bold is ugly mTitle->mFont = fonts->Get("", 0, TTF_STYLE_BOLD);
	}
	
	ResizeChildren();

}

Console::~Console()
{
	resman->Unload(mBackground);
}

void Console::Render(uLong ms)
{
	Image* scr = Screen::Instance();
	
	//draw translucent background
	if (mBackground && !mResizing && mOutput)
	{
		rect r = mOutput->GetScreenPosition();
		mBackground->Render(scr, r.x, r.y, 
							rect(0,0,mBackground->Width(), mBackground->Height()));
	}

	//TODO: Anything?
	Frame::Render(ms);
}

void Console::Event(SDL_Event* event)
{
	if (mInput && mOutput->HasKeyFocus())
		mInput->SetKeyFocus();	

	Frame::Event(event);
}

void Console::ResizeChildren()
{
	if (mInput)
		mInput->SetPosition( rect(26, mPosition.h - mInput->Height() - 3, mPosition.w - 60, mInput->Height()) );

	if (mOutput)
		mOutput->SetPosition( rect(0, 16, mPosition.w, mPosition.h - mTopImage->mSrc.h - mBottomImage->mSrc.h) );

	if (mExit)
		mExit->SetPosition( rect(mPosition.w - 16, 2, mExit->Width(), mExit->Height()) );
	
	if (mBottomImage)
	{
		mBottomImage->mDst.y = mPosition.h - mBottomImage->mDst.h;	
	}
	resman->Unload(mBackground);
	
	if (isDefaultColor(mBackgroundColor))
	{
		mBackground = NULL;
	}
	else
	{
		mBackground = resman->NewImage(mOutput->Width(), mOutput->Height(), color(255,255,255), true);
	
		if (mBackground)
			mBackground->DrawRect(rect(0,0,mBackground->Width(),mBackground->Height()), mBackgroundColor);
	}
	
	Frame::ResizeChildren();
}

void Console::SetPosition(rect r)
{
	if (r.w < 150)
		r.w = 150;
		
	if (r.h < 100)
		r.h = 100;

	Widget::SetPosition(r);
}

void Console::AddMessage(string msg)
{
	if (msg.empty()) return;
	
	if (mShowTimestamps && config.GetParamInt("console", "timestamps") == 1)
		msg = shortTimestamp() + " " + msg;
	
	PRINT("[Console] " + msg);

	mOutput->AddMessage(msg);
}

void Console::AddFormattedMessage(string msg)
{
	if (msg.empty()) return;
	
	if (mShowTimestamps && config.GetParamInt("console", "timestamps") == 1)
		msg = shortTimestamp() + " " + msg;
	
	PRINT("[Console] " + msg);

	mOutput->AddFormattedMessage(msg);
}

void Console::ShowCommands()
{
	string s = mInput->GetText();
	for (uShort i = 0; i < mCommandHooks.size(); i++)
	{
		if (s.empty() || mCommandHooks.at(i).cmd.find(s, 0) == 0)
			AddMessage(mCommandHooks.at(i).cmd);
	}
}

void Console::HookCommand(string cmd, void (*callback)(Console*, string))
{
	UnhookCommand(cmd);
	
	consoleCommand c;
	c.cmd = cmd;
	c.callback = callback;
	c.link = NULL;
	mCommandHooks.push_back(c);
}

void Console::HookCommand(string cmd, consoleCommand::commandType type, void* link)
{
	UnhookCommand(cmd);
	
	consoleCommand c;
	c.cmd = cmd;
	c.callback = NULL;
	c.type = type;
	c.link = link;
	mCommandHooks.push_back(c);
}

void Console::UnhookCommand(string cmd)
{
	for (uShort i = 0; i < mCommandHooks.size(); i++)
	{
		if (mCommandHooks.at(i).cmd == cmd)
		{
			mCommandHooks.erase(mCommandHooks.begin() + i);
			return;
		}
	}
}

void Console::DoCommand(string s)
{
	string cmd = lowercase( s.substr(0, s.find(" ")) );
	
	consoleCommand* c = NULL;
	for (uShort i = 0; i < mCommandHooks.size(); i++)
	{
		if (mCommandHooks.at(i).cmd == cmd)
		{
			_runHookedCommand(mCommandHooks.at(i), s);
			return;
		}
		else if (mCommandHooks.at(i).cmd.empty())
			c = &mCommandHooks.at(i);
	}
	
	//not found, but if we have a catch-all, run it
	if (c)
		_runHookedCommand(*c, s);
	else
		AddMessage("\\c400* Command " + stripCodes(cmd) + " not recognized");
}

void Console::_runHookedCommand(consoleCommand& c, string s)
{
	if (c.callback)
	{
		c.callback(this, s);
		return;
	}
	
	//Command isn't a callback type, output its linked variable.
	string out = c.cmd + " = ";

	if (!c.link)
	{
		out += "NULL";
	}
	else
	{
		//Convert the type c is containing, and output it.
		switch (c.type)
		{
			case consoleCommand::INT: out += its( *(int*)c.link ); break;
			case consoleCommand::DOUBLE: out += dts( *(double*)c.link ); break;
			case consoleCommand::STRING: out += *(string*)c.link; break;
			case consoleCommand::VOID: out += pts( c.link ); break;
			case consoleCommand::RECT: out += rts( *(rect*)c.link ); break;
			case consoleCommand::POINT2D: out += p2dts( *(point2d*)c.link ); break;
			default: out += "?"; break;
		}
	}
	
	AddMessage(out);
}

//logs text to file
void Console::SaveText()
{
	if (!mOutput) return;
	
	string s;
	s = "saved/" + mSavePrefix + timestamp(true) + ".html";
	
	FILE* f = fopen(s.c_str(), "w");
	if (!f)
	{
		AddMessage("\\c900* Could not open output file");
		return;
	}
	
	fprintf(f, "<html>\n<head>\n"
				"<title>Saved %s</title>\n"
				"</head>\n<body bgcolor=\"#000000\">\n", timestamp(true).c_str());
	vString v;
	string c;
	for (int i = 0; i < mOutput->mRawText.size(); i++)
	{
		//Split up the line via \cRGB 
		explode(&v, &mOutput->mRawText.at(i), "\\c");
		for (int ii = 0; ii < v.size(); ii++)
		{
			if (ii == 0 && mOutput->mRawText.at(i).find("\\c", 0) != 0) 
			{
				//if we don't have a font color defined, use default
				c = "FFFFFF";
			}
			else
			{
				c = colorToHex( slashCtoColor( v.at(ii).substr(0, 3) ) );
				v.at(ii).erase(0, 3); //erase RGB
			}
			fprintf(f, "<font color=\"#%s\">%s</font>", c.c_str(), v.at(ii).c_str());
		}
		v.clear();
		fprintf(f, "<br/>\n");
	}
	fprintf(f, "</body>\n</html>\n");
	fclose(f);
	AddMessage("\\c090* Chat saved to: " + s);
}
