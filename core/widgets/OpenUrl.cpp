
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


#include "OpenUrl.h"

#ifdef WIN32
	#include <windows.h>
#endif

#include "Button.h"
#include "Label.h"
#include "../io/FileIO.h"

void callback_OpenUrlDefaultBrowser(Button* b)
{
	OpenUrl* o = (OpenUrl*)b->GetParent();
	
	#ifdef WIN32
		//Will open the url with the default browser. TODO: This has crashed some clients! If the user win+r runs a url and it does nothing, this too will fail, but worse.
		ShellExecute(NULL, "open", o->mUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
	#endif
	
	o->Die();
}

void callback_OpenUrlCopy(Button* b)
{
	OpenUrl* o = (OpenUrl*)b->GetParent();
	
	sendStringToClipboard(o->mUrl);
	
	o->Die();
}

OpenUrl::OpenUrl(string url)
	: Frame(gui, "OpenUrl", rect(0,0,300,100), "Open Url", true, false, true, true)
{
	mUrl = url;
	
	url = "How do you want to open: \\c239" + url;
	
	Label* l;
	l = new Label(this, "caption", rect(5, 30, 0, 0));
	l->mMaxWidth = OPENURL_MAX_WIDTH - 10;
	l->SetCaption(url);
	
	//set width based on label caption, up to max
	uShort w = l->Width();
	uShort h = l->Height();

	w += 10;
	h += 30 + 30;
	
	if (w < OPENURL_MIN_WIDTH)
		w = OPENURL_MIN_WIDTH;

	SetSize(w, h);
	
	//center label
	l->Center();

	Button* b;
	b = new Button(this, "", rect(Width()-25,Height()-25,20,20), "", callback_OpenUrlDefaultBrowser);
		b->mHoverText = "Open in default web browser";
		b->SetImage("assets/buttons/web.png");
					
	b = new Button(this, "", rect(Width()-50,Height()-25,20,20), "", callback_OpenUrlCopy);
		b->mHoverText = "Copy to clipboard";
		b->SetImage("assets/buttons/clipboard.png");

	ResizeChildren();
	Center();
	DemandFocus();
}

OpenUrl::~OpenUrl()
{
	
}

void OpenUrl::Render()
{
	//if (gui->GetDemandsFocus() == this)
	//	gui->RenderDarkOverlay();
	
	Frame::Render();
}



