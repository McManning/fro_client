
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



#include "AvatarEdit.h"
#include "AvatarFavorites.h"
#include "../avatar/Avatar.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/Checkbox.h"

void callback_avatarEditClose(Button* b)
{
	//cancel operation
	avatarFavorites->mAvatarEdit->Die();
	avatarFavorites->mAvatarEdit = NULL;
}

void callback_avatarEditSave(Button* b)
{
	AvatarEdit* editor = (AvatarEdit*)b->GetParent();
	
	if (!editor)	
		return;
	
	AvatarProperties props;

	props.url = getInputText(editor, "url");
	if (props.url.empty()) // || url.find("http://", 0) != 0)
	{
		editor->mAlertLabel->SetCaption("Invalid Url");
		return;
	}

	props.w = sti( getInputText(editor, "width") );
	if (props.w < 20 || props.w > MAX_AVATAR_WIDTH)
	{
		editor->mAlertLabel->SetCaption(
					"Invalid Width. (20 to " + its(MAX_AVATAR_WIDTH) + " pixels)");
		return;
	}

	props.h = sti( getInputText(editor, "height") );
	if (props.h < 20 || props.h > MAX_AVATAR_HEIGHT)
	{
		editor->mAlertLabel->SetCaption(
					"Invalid Height. (20 to " + its(MAX_AVATAR_HEIGHT) + " pixels)");
		return;
	}

	props.delay = sti( getInputText(editor, "delay") );
	if (props.delay < 1)
	{
		editor->mAlertLabel->SetCaption("Invalid Delay. (1 to 9999)");
		return;
	}

	props.pass = getInputText(editor, "pass");
	
	props.flags = 0;
	if (getCheckboxState(editor, "loopsit"))
	  props.flags |= AVATAR_FLAG_LOOPSIT;
	  
	if (getCheckboxState(editor, "loopstand"))
	  props.flags |= AVATAR_FLAG_LOOPSTAND;


	avatarFavorites->UpdateAvatar(avatarFavorites->mWorkingFolder, editor->mWorkingIndex, props);
	
	avatarFavorites->mAvatarEdit = NULL;
	avatarFavorites->SetActive(true);

	editor->Die();
}

AvatarEdit::AvatarEdit(int workingIndex, AvatarProperties* props)
	: Frame(gui, "AvatarEdit_" + string((props) ? "Edit" : "Add"), rect(),
			(props) ? "Edit Avatar" : "Add Avatar", true, false, true, true)
{
	mClose->onClickCallback = callback_avatarEditClose; //so we can run some custom code while it closes

	SetSize(260, 180);
	
	mWorkingIndex = workingIndex;

	Input* i;
	new Label(this, "", rect(10,30), "Url:");
	i = new Input(this, "url", rect(60,30,190,20), "", 0, true, NULL);
		if (props) i->SetText(props->url);

	new Label(this, "", rect(10,55), "Width:");
	i = new Input(this, "width", rect(60,55,55,20), "0123456789", 3, true, NULL);
		i->mHoverText = "Single frame width in pixels \\c600(For PNGs Only)";
		if (props) i->SetText(its(props->w));
		i->SetMenuEnabled(false);
		
	new Label(this, "", rect(130,55), "Height:");
	i = new Input(this, "height", rect(195,55,55,20), "0123456789", 3, true, NULL);
		i->mHoverText = "Single frame height in pixels \\c600(For PNGs Only)";
		if (props) i->SetText(its(props->h));
		i->SetMenuEnabled(false);

	new Label(this, "", rect(10,80), "Delay:");
	i = new Input(this, "delay", rect(60,80,55,20), "0123456789", 4, true, NULL);
		i->mHoverText = "Time to display each frame in milliseconds \\c600(For PNGs Only)";
		if (props) i->SetText(its(props->delay));
		i->SetMenuEnabled(false);

	new Label(this, "", rect(130,80), "Password:");
	i = new Input(this, "pass", rect(195,80,55,20), "", 0, true, NULL);
		i->mHoverText = "Encryption Password \\c700(Not working yet)";
		if (props) i->SetText(props->pass);
		i->SetMenuEnabled(false);

	Checkbox* c;
	c = new Checkbox(this, "loopstand", rect(10,105), "Loop Stand Animation", 0);
		if (props) c->SetState(props->flags & AVATAR_FLAG_LOOPSTAND);

	c = new Checkbox(this, "loopsit", rect(10,130), "Loop Sit Animation", 0);
		if (props) c->SetState(props->flags & AVATAR_FLAG_LOOPSIT);

	mAlertLabel = new Label(this, "", rect(10,152), "");
	mAlertLabel->mFontColor = color(255,0,0);

	Button* b;
	b = new Button(this, "save",rect(230,152,20,20), "", callback_avatarEditSave);
		b->mHoverText = "Save Avatar";
		b->SetImage("assets/buttons/okay.png");

	Center();
	ResizeChildren();
}

AvatarEdit::~AvatarEdit()
{
	if (avatarFavorites)
		avatarFavorites->SetActive(true);
}

