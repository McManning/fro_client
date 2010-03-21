
#include "AvatarViewer.h"

#include "../core/widgets/MessagePopup.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/SmallSelect.h"
#include "../core/widgets/FilePicker.h"

#include "../entity/Avatar.h"

void callback_avyViewOpen(Button* b)
{
	AvatarViewer* a = (AvatarViewer*)b->GetParent();
	a->Load();
}

void callback_avyViewReload(Button* b)
{
	AvatarViewer* a = (AvatarViewer*)b->GetParent();
	a->Reload();
}

void callback_avyViewChangePositions(SmallSelect* s)
{
	AvatarViewer* a = (AvatarViewer*)s->GetParent();
	a->ChangePosition();
}

void callback_avyViewSelectFile(FilePicker* f)
{
	AvatarViewer* a = (AvatarViewer*)f->mUserdata;
	a->LoadFile(f->GetFile());	
}

AvatarViewer::AvatarViewer() :
	Frame(gui, "avyview", rect(), "Avatar Viewer (BETA)", true, false, true, true)
{	
	mAvatar = NULL;
	
	Button* b;

//Create section frames

	uShort y, x;
	
	x = 110;
	y = 30;
	
	/*	Open/Reload File controls */

	b = new Button(this, "",rect(x,y,20,20), "", callback_avyViewOpen);
		b->mHoverText = "Open Avatar";
		makeImage(b, "", "assets/options_tabs.png", rect(0,0,20,20),
				rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	
	b = new Button(this, "",rect(x+25,y,20,20), "", callback_avyViewReload);
		b->mHoverText = "Reload Avatar";
		makeImage(b, "", "assets/options_tabs.png", rect(20,0,20,20),
				rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	y += 25;
	
	/*	Manual PNG Avatar properties */
	
	new Label(this, "", rect(x,y), "Width:");
	mAvyWidth = new Input(this, "width", rect(x+50,y,50,20), "0123456789", 3, true, NULL);
		mAvyWidth->mHoverText = "Single frame width in pixels \\c600(For PNGs Only)";
		mAvyWidth->SetText("32");
		mAvyWidth->SetMenuEnabled(false);
	y += 25;
	
	new Label(this, "", rect(x,y), "Height:");
	mAvyHeight = new Input(this, "height", rect(x+50,y,50,20), "0123456789", 3, true, NULL);
		mAvyHeight->mHoverText = "Single frame height in pixels \\c600(For PNGs Only)";
		mAvyHeight->SetText("64");
		mAvyHeight->SetMenuEnabled(false);
	y += 25;
	
	new Label(this, "", rect(x,y), "Delay:");
	mAvyDelay = new Input(this, "delay", rect(x+50,y,50,20), "0123456789", 4, true, NULL);
		mAvyDelay->mHoverText = "Time to display each frame in milliseconds \\c600(For PNGs Only)";
		mAvyDelay->SetText("1000");
		mAvyDelay->SetMenuEnabled(false);
	y += 25;
	
	/*	Avatar viewer controls */
	
	new Label(this, "", rect(x,y), "Position:");
	y += 20;
	mAvyPositions = new SmallSelect(this, "pos", rect(x, y, 100, 20), callback_avyViewChangePositions);
	y += 25;
	
	
	SetSize(215, y + 30);
	Center();

	ResizeChildren();
	
	LoadFile("assets/default.png");
}

AvatarViewer::~AvatarViewer()
{

}

void AvatarViewer::Render(uLong ms)
{
	Frame::Render(ms);
	
	Image* scr = Screen::Instance();
	rect r = GetScreenPosition();

	if (mAvatar && mAvatar->GetImage())
	{
		mAvatar->GetImage()->Render(scr, r.x + 55 - mAvatar->GetImage()->Width() / 2, r.y + 50);
	}	
}

void AvatarViewer::Load()
{
	//open up a file chooser dialog
	new FilePicker(callback_avyViewSelectFile, this);
}

void AvatarViewer::Reload()
{
	if (mAvatar)
		LoadFile(mAvatar->mUrl);
}

void AvatarViewer::ChangePosition()
{
	console->AddMessage("Changing to " + mAvyPositions->GetSelectedItem());
	if (mAvatar && mAvatar->GetImage())
	{
		mAvatar->GetImage()->SetFrameset(mAvyPositions->GetSelectedItem());
		mAvatar->GetImage()->Reset();
	}
}

void AvatarViewer::LoadFile(string file)
{
	SAFEDELETE(mAvatar);
	
	mAvatar = new Avatar();
	
	mAvatar->mUrl = file;
	mAvatar->mPass.clear();
	mAvatar->mWidth = sti(mAvyWidth->GetText());
	mAvatar->mHeight = sti(mAvyHeight->GetText());
	mAvatar->mDelay = sti(mAvyDelay->GetText());
	mAvatar->mLoopStand = true;
	mAvatar->mLoopSit = false;

	mAvatar->Load();

	if (!mAvatar->GetImage())
	{
		SAFEDELETE(mAvatar);
		new MessagePopup("", "Bad Avatar", "Could not load " + file);
		return;
	}
	
	mAvatar->Convert();
		
	mAvyPositions->mItems.clear();
	mAvyPositions->mSelectedIndex = 0;
		
	Image* img = mAvatar->GetImage();
		
	//fill mAvyPositions with our keys
	for (uShort i = 0; i < img->mImage->framesets.size(); i++)
	{
		mAvyPositions->AddItem( img->mImage->framesets.at(i).key );	
	}

}
