
#include "FilePicker.h"

#include "Button.h"
#include "Label.h"
#include "Input.h"
#include "MessagePopup.h"
#include "../io/FileIO.h"

void callback_FilePickerOkay(Button* b)
{
	FilePicker* f = (FilePicker*)b->GetParent();

	f->Done();
}

FilePicker::FilePicker(void (*cb)(FilePicker*), void* userdata)
	: Frame(gui, "FilePicker", rect(), "Select A File", true, false, true, true)
{
	SetSize(350, 80);
	
	onSelectCallback = cb;
	mUserdata = userdata;

	Input* i;
	i = new Input(this, "path", rect(5,30,Width()-10,20), "", 0, true, NULL);
		i->mHoverText = "Input the full or relative path to your desired file";
		i->SetMenuEnabled(true);
	
	Button* b;
	b = new Button(this, "", rect(Width()-25,Height()-25,20,20), "", callback_FilePickerOkay);
	b->mHoverText = "Okay";
		makeImage(b, "", "assets/button20.png", rect(60,0,20,20), 
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);

	ResizeChildren();
	Center();
	DemandFocus();
}

FilePicker::~FilePicker()
{
	
}

void FilePicker::Render(uLong ms)
{
	if (gui->GetDemandsFocus() == this)
		gui->RenderDarkOverlay();
	
	Frame::Render(ms);
}

string FilePicker::GetFile()
{
	return ((Input*)Get("path"))->GetText();
}

void FilePicker::Done()
{
	if (!fileExists(GetFile()))
	{
		new MessagePopup("", "Error", "File does not exist!");	
		return;
	}
	
	Die();
	
	if (onSelectCallback)
		onSelectCallback(this);
}


