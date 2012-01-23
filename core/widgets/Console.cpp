
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
#include "../io/FileIO.h"

Console* console;

const int MAX_CONSOLE_INPUT_LENGTH = 256;

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

void callback_consoleToggleBackground(Button* b)
{
	Console* c = (Console*)b->GetParent();
	if (c)
	{
		c->mDrawBackground = !c->mDrawBackground;
		c->FlagRender();
	}
}

Console::Console(string id, string title, string savePrefix, color c,
				bool hasExit, bool hasInput, bool hasExtraControls)
	: Frame(NULL, "", rect(), "", true, false, false, false)
{
	mFont = fonts->Get();
	mId = id;
	mType = WIDGET_FRAME;

	mBackgroundColor = color(0,0,0,230);
	mBackground = NULL;
	mShowTimestamps = false;
	mDrawBackground = true;
	mSizeable = true; //will not create mSizer, and instead create a custom one
	mOutput = NULL;
	mSavePrefix = savePrefix;

	SetPosition( rect(gui->Width()-300,gui->Height()-176,300,176) );

	if (hasInput)
	{
		mInput = new Input(this, "input", rect(26,0,0,20), "", MAX_CONSOLE_INPUT_LENGTH, true, callback_consoleInput);
		mInput->SetImage("assets/gui/console/input.png");
		mInput->mFontColor = color(255,255,255);
	}
	else
	{
		mInput = NULL;
	}

	mOutput = new Multiline(this, "output", rect(0, 16, mPosition.w, mPosition.h - 42));
		mOutput->mScrollbar->SetImageBase("assets/gui/console/vscroller");

		// Colorize to match
	//	mOutput->mScrollbar->mTabImage->ColorizeGreyscale(c);
	//	mOutput->mScrollbar->mValueUp->mImage->ColorizeGreyscale(c);
	//	mOutput->mScrollbar->mValueDown->mImage->ColorizeGreyscale(c);
	//	mOutput->mScrollbar->mImage->ColorizeGreyscale(c);

	// We have our own background drawn
	resman->Unload(mOutput->mImage);
	mOutput->mImage = NULL;

	mOutput->mFont = fonts->Get("", 0, TTF_STYLE_BOLD); //load default font, but bold.
	mOutput->mFontColor = color(255,255,255);

	mExit = NULL;
	if (hasExit)
	{
		mExit = new Button(this, "exit", rect(0, 2, 12, 12), "", callback_consoleExit);
		mExit->SetImage("assets/gui/console/exit.png");
		mExit->mHoverText = "Close";

	//	mExit->mImage->ColorizeGreyscale(c);
	}

	mBackgroundToggle = NULL;
	if (hasExtraControls)
	{
		mBackgroundToggle = new Button(this, "", rect(0, 2, 12, 12), "", callback_consoleToggleBackground);
		mBackgroundToggle->SetImage("assets/gui/console/toggle.png");
		mBackgroundToggle->mHoverText = "Toggle Background";

	//	mBackgroundToggle->mImage->ColorizeGreyscale(c);

//		mFontSizeChange = new Button(this, "", rect(0, 2, 12, 12), "", callback_consoleChangeFontSize);
//		mBackgroundToggle->SetImage(imageFile + "font.png");
//		mBackgroundToggle->mHoverText = "Change Font Size";
	}

	mTitle = new Label(this, "title", rect(5, 1), title);

	//don't use mImage b/c Frame will try to render it. Use a custom.
	mBackgroundImage = resman->LoadImg("assets/gui/console/bg.png");
	//mBackgroundImage->ColorizeGreyscale(c);

	ResizeChildren();

}

Console::~Console()
{
	resman->Unload(mBackground);
	resman->Unload(mBackgroundImage);
}

void Console::Render()
{
	Image* scr = Screen::Instance();

	rect r = GetScreenPosition();

	//draw translucent background
	if (mDrawBackground && mBackground && !mResizing && mOutput)
	{
		rect rr = mOutput->GetScreenPosition();
		mBackground->Render(scr, rr.x, rr.y,
							rect(0,0,mBackground->Width(), mBackground->Height()));
	}

	if (mBackgroundImage && !mResizing)
	{
		mBackgroundImage->RenderHorizontalEdge(scr, rect(0, 0, 26, CONSOLE_HEADER_HEIGHT),
												rect(r.x, r.y, r.w, CONSOLE_HEADER_HEIGHT));

		if (mInput)
		{
			mBackgroundImage->RenderHorizontalEdge(scr, rect(0, 16, 26, CONSOLE_FOOTER_WITH_INPUT_HEIGHT),
												rect(r.x, r.y + r.h - CONSOLE_FOOTER_WITH_INPUT_HEIGHT,
												r.w, CONSOLE_FOOTER_WITH_INPUT_HEIGHT));
			if (mSizeable)
				mBackgroundImage->Render(scr, r.x + r.w - 26, r.y + r.h - 26, rect(78, 0, 26, 26));
		}
		else
		{
			mBackgroundImage->RenderHorizontalEdge(scr, rect(0, 42, 26, CONSOLE_FOOTER_NO_INPUT_HEIGHT),
												rect(r.x, r.y + r.h - CONSOLE_FOOTER_NO_INPUT_HEIGHT,
												r.w, CONSOLE_FOOTER_NO_INPUT_HEIGHT));
			if (mSizeable)
				mBackgroundImage->Render(scr, r.x + r.w - 16, r.y + r.h - 16, rect(78, 42, 16, 16));
		}
	}

	Frame::Render();
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
	{
		int h = (mInput) ? CONSOLE_FOOTER_WITH_INPUT_HEIGHT : CONSOLE_FOOTER_NO_INPUT_HEIGHT;
		mOutput->SetPosition( rect(0, 16, mPosition.w, mPosition.h - CONSOLE_HEADER_HEIGHT - h) );
	}

	if (mExit)
		mExit->SetPosition( rect(mPosition.w - 16, 2, mExit->Width(), mExit->Height()) );

	if (mBackgroundToggle)
		mBackgroundToggle->SetPosition(
								rect(mPosition.w - 16 - ((mExit) ? 16 : 0), 2,
									mBackgroundToggle->Width(),
									mBackgroundToggle->Height())
								);


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

	if (mShowTimestamps)
		msg = shortTimestamp() + " " + msg;

	DEBUGOUT("[Console] " + msg);

	mOutput->AddMessage(msg);
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
		AddMessage("\\c700* Command " + stripCodes(cmd) + " not recognized");
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
			case consoleCommand::VOIDPTR: out += pts( c.link ); break;
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
	if (!mOutput || mSavePrefix.empty()) return;

	string s;
	s = "saved/" + mSavePrefix + timestamp(true) + ".html";
	buildDirectoryTree(s);

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
	AddMessage("\\c090* Log saved to: " + s);
}

void Console::SetTitle(string s)
{
	mTitle->SetCaption(s);
}







