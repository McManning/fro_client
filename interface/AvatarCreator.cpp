
#include "AvatarCreator.h"
#include "AvatarFavoritesDialog.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Scrollbar.h"
#include "../core/io/FileIO.h"
#include "../game/GameManager.h"
#include "../entity/LocalActor.h"

#define MAX_AVATARBASES 3

const char* avatarBases[MAX_AVATARBASES] = 
{
	"adult",
	"teen",
	"child"
};

void callback_AvatarCreatorBase(Button* b)
{
	AvatarCreator* a = (AvatarCreator*)gui->Get("AvatarCreator");
	ASSERT(a);
	a->CycleBase( (b->mId == "n") );
}
		
void callback_AvatarCreatorBody(Button* b)
{
	AvatarCreator* a = (AvatarCreator*)gui->Get("AvatarCreator");
	ASSERT(a);
	a->CycleBody( (b->mId == "n") );
}

void callback_AvatarCreatorHair(Button* b)
{
	AvatarCreator* a = (AvatarCreator*)gui->Get("AvatarCreator");
	ASSERT(a);
	a->CycleHair( (b->mId == "n") );
}

void callback_AvatarCreatorHead(Button* b)
{
	AvatarCreator* a = (AvatarCreator*)gui->Get("AvatarCreator");
	ASSERT(a);
	a->CycleHead( (b->mId == "n") );
}
	
void callback_AvatarCreatorToggleTab(Button* b)
{
	AvatarCreator* a = (AvatarCreator*)gui->Get("AvatarCreator");
	ASSERT(a);
	a->Toggle(b->mId);
}

void callback_AvatarCreatorColor(Scrollbar* s)
{
	AvatarCreator* a = (AvatarCreator*)gui->Get("AvatarCreator");
	ASSERT(a);
	a->Update();
}

void callback_AvatarCreatorAddToFavorites(Button* b)
{
	AvatarCreator* a = (AvatarCreator*)gui->Get("AvatarCreator");
	ASSERT(a);
	a->AddDesignToFavorites();
}

void callback_AvatarCreatorSavePNG(Button* b)
{
	AvatarCreator* a = (AvatarCreator*)gui->Get("AvatarCreator");
	ASSERT(a);
	a->SaveToFile();
}



AvatarCreator::AvatarCreator() :
	Frame(gui, "AvatarCreator", rect(), "Avatar Designer (BETA)", true, false, true, true)
{
	SetSize(230, 225);
	Center();
	
	mHeadColor = mBodyColor = mHairColor = color(255,255,255);
	mHead = mBody = mHair = mFullComposite = mCompositePreview = NULL;
	mRedraw = false;

	CreateControls();
	ResizeChildren();
	
	mBase = avatarBases[0];
	mHeadFile = mBase + ".head.default.png";
	mBodyFile = mBase + ".body.default.png";
	mHairFile = mBase + ".hair.default.png";
	
	Update();
}

AvatarCreator::~AvatarCreator()
{
	resman->Unload(mHead);
	resman->Unload(mBody);
	resman->Unload(mHair);
	resman->Unload(mCompositePreview);
	resman->Unload(mFullComposite);
}

void AvatarCreator::CreateControls()
{
	uShort y, x;
	Scrollbar* s;
	Button* b;
	
	uShort controlsX = Width() - 155;
	uShort controlsY = 75;
	
	mErrorLabel = new Label(this, "", rect(10,35), "");
	mErrorLabel->mFontColor = color(255,0,0);
		
	b = new Button(this, "Add", rect(10,200,20,20), "", callback_AvatarCreatorAddToFavorites);
		b->mHoverText = "Set as avatar and add to favorites";
		b->SetImage("assets/buttons/options_user.png");
			
	b = new Button(this, "Save", rect(35,200,20,20), "", callback_AvatarCreatorSavePNG);
		b->mHoverText = "Save to PNG";
		b->SetImage("assets/buttons/options_user.png");
	
//Base edit frame
	y = 0;
		
	mBaseFrame = new Frame(this, "base", rect());
	
	new Label(mBaseFrame, "", rect(0,y), "Edit Base Model");
	y += 25;
	
	b = new Button(mBaseFrame, "p", rect(0,y,20,20), "", callback_AvatarCreatorBase);
		b->mHoverText = "Previous Model";
		b->SetImage("assets/buttons/options_user.png");
				
	b = new Button(mBaseFrame, "n", rect(150-20,y,20,20), "", callback_AvatarCreatorBase);
		b->mHoverText = "Next Model";
		b->SetImage("assets/buttons/options_user.png");
	y += 25;

	mBaseFrame->SetPosition( rect(controlsX, controlsY, 150, y) );
	mBaseFrame->ResizeChildren();
	mBaseFrame->SetVisible(false);
	
//Hair Edit Frame
	y = 0;
	
	mHairFrame = new Frame(this, "hair", rect());
	
	new Label(mHairFrame, "", rect(0,y), "Edit Hair Model");
	y += 25;
	
	s = new Scrollbar(mHairFrame, "r", rect(0,y,150,20), HORIZONTAL, 255, 10, 255, callback_AvatarCreatorColor);
		s->mHoverText = "Red Value";
		//s->mTabImage->mSrc.x = 90;
	y += 25;
	s = new Scrollbar(mHairFrame, "g", rect(0,y,150,20), HORIZONTAL, 255, 10, 255, callback_AvatarCreatorColor);
		s->mHoverText = "Green Value";
		//s->mTabImage->mSrc.x = 105;
	y += 25;
	s = new Scrollbar(mHairFrame, "b", rect(0,y,150,20), HORIZONTAL, 255, 10, 255, callback_AvatarCreatorColor);
		s->mHoverText = "Blue Value";
		//s->mTabImage->mSrc.x = 120;
	y += 25;
	
	b = new Button(mHairFrame, "p", rect(0,y,20,20), "", callback_AvatarCreatorHair);
		b->mHoverText = "Previous Model";
		b->SetImage("assets/buttons/options_user.png");
				
	b = new Button(mHairFrame, "n", rect(150-20,y,20,20), "", callback_AvatarCreatorHair);
		b->mHoverText = "Next Model";
		b->SetImage("assets/buttons/options_user.png");
	y += 25;
		
	mHairFrame->SetPosition( rect(controlsX, controlsY, 150, y) );
	mHairFrame->ResizeChildren();
	mHairFrame->SetVisible(false);
	
//Head Edit Frame
	y = 0;
	
	mHeadFrame = new Frame(this, "head", rect());
	
	new Label(mHeadFrame, "", rect(0,y), "Edit Head Model");
	y += 25;
	
	s = new Scrollbar(mHeadFrame, "r", rect(0,y,150,20), HORIZONTAL, 255, 10, 255, callback_AvatarCreatorColor);
		s->mHoverText = "Red Value";
		//s->mTabImage->mSrc.x = 90;
	y += 25;
	s = new Scrollbar(mHeadFrame, "g", rect(0,y,150,20), HORIZONTAL, 255, 10, 255, callback_AvatarCreatorColor);
		s->mHoverText = "Green Value";
		//s->mTabImage->mSrc.x = 105;
	y += 25;
	s = new Scrollbar(mHeadFrame, "b", rect(0,y,150,20), HORIZONTAL, 255, 10, 255, callback_AvatarCreatorColor);
		s->mHoverText = "Blue Value";
		//s->mTabImage->mSrc.x = 120;
	y += 25;
	
	b = new Button(mHeadFrame, "p", rect(0,y,20,20), "", callback_AvatarCreatorHead);
		b->mHoverText = "Previous Model";
		b->SetImage("assets/buttons/options_user.png");
				
	b = new Button(mHeadFrame, "n", rect(150-20,y,20,20), "", callback_AvatarCreatorHead);
		b->mHoverText = "Next Model";
		b->SetImage("assets/buttons/options_user.png");
	y += 25;
					
	mHeadFrame->SetPosition( rect(controlsX, controlsY, 150, y) );
	mHeadFrame->ResizeChildren();
	mHeadFrame->SetVisible(false);
		
//Body Edit Frame
	y = 0;
	
	mBodyFrame = new Frame(this, "body", rect());
	
	new Label(mBodyFrame, "", rect(0,y), "Edit Body Model");
	y += 25;
	
	s = new Scrollbar(mBodyFrame, "r", rect(0,y,150,20), HORIZONTAL, 255, 10, 255, callback_AvatarCreatorColor);
		s->mHoverText = "Red Value";
	//	s->mTabImage->mSrc.x = 90;
	y += 25;
	s = new Scrollbar(mBodyFrame, "g", rect(0,y,150,20), HORIZONTAL, 255, 10, 255, callback_AvatarCreatorColor);
		s->mHoverText = "Green Value";
	//	s->mTabImage->mSrc.x = 105;
	y += 25;
	s = new Scrollbar(mBodyFrame, "b", rect(0,y,150,20), HORIZONTAL, 255, 10, 255, callback_AvatarCreatorColor);
		s->mHoverText = "Blue Value";
	//	s->mTabImage->mSrc.x = 120;
	y += 25;
	
	b = new Button(mBodyFrame, "p", rect(0,y,20,20), "", callback_AvatarCreatorBody);
		b->mHoverText = "Previous Model";
		b->SetImage("assets/buttons/options_user.png");
				
	b = new Button(mBodyFrame, "n", rect(150-20,y,20,20), "", callback_AvatarCreatorBody);
		b->mHoverText = "Next Model";
		b->SetImage("assets/buttons/options_user.png");
	y += 25;
				
	mBodyFrame->SetPosition( rect(controlsX, controlsY, 150, y) );
	mBodyFrame->ResizeChildren();
	mBodyFrame->SetVisible(true);

//main dialog
	//width: 10 + 60 + 5 + 150 + 10
	//height: at LEAST 30 + 80 + 10
	
	y = 55;
	x = controlsX;
	b = new Button(this, "base", rect(x,y,20,20), "", callback_AvatarCreatorToggleTab);
		b->mHoverText = "Edit Base Model";
		b->SetImage("assets/buttons/options_user.png");
	x += 25;
	
	b = new Button(this, "head", rect(x,y,20,20), "", callback_AvatarCreatorToggleTab);
		b->mHoverText = "Edit Head Model";
		b->SetImage("assets/buttons/options_user.png");
	x += 25;
	
	b = new Button(this, "body", rect(x,y,20,20), "", callback_AvatarCreatorToggleTab);
		b->mHoverText = "Edit Body Model";
		b->SetImage("assets/buttons/options_user.png");
	x += 25;
	
	b = new Button(this, "hair", rect(x,y,20,20), "", callback_AvatarCreatorToggleTab);
		b->mHoverText = "Edit Hair Model";
		b->SetImage("assets/buttons/options_user.png");
	
	//TODO: save buttons, some background for the preview to be rendered in front of, etc.

}

void AvatarCreator::Toggle(string id)
{
	mBaseFrame->SetVisible( (id == "base") );
	mHeadFrame->SetVisible( (id == "head") );
	mBodyFrame->SetVisible( (id == "body") );
	mHairFrame->SetVisible( (id == "hair") );
}

void AvatarCreator::SetError(string error)
{
	mErrorLabel->SetCaption(error);
}

void AvatarCreator::CycleBase(bool forward)
{

	for (uShort i = 0; i < MAX_AVATARBASES; i++)
	{
		if (mBase == avatarBases[i])
		{
			if (i == MAX_AVATARBASES - 1)
				mBase = avatarBases[0];
			else
				mBase = avatarBases[i+1];
		}
	}
	
	mHeadFile = mBase + ".head.default.png";
	mBodyFile = mBase + ".body.default.png";
	mHairFile = mBase + ".hair.default.png";
	
	Update();
}

void AvatarCreator::CycleHead(bool forward)
{
	if (forward)
		mHeadFile = getNextFileMatchingPattern(mHeadFile, DIR_AVA + mBase + ".head.*.png");
	else
		mHeadFile = getPreviousFileMatchingPattern(mHeadFile, DIR_AVA + mBase + ".head.*.png");
		
	Update();
}

void AvatarCreator::CycleBody(bool forward)
{
	if (forward)
		mBodyFile = getNextFileMatchingPattern(mBodyFile, DIR_AVA + mBase + ".body.*.png");
	else
		mBodyFile = getPreviousFileMatchingPattern(mBodyFile, DIR_AVA + mBase + ".body.*.png");
	
	Update();
}

void AvatarCreator::CycleHair(bool forward)
{
	if (forward)
		mHairFile = getNextFileMatchingPattern(mHairFile, DIR_AVA + mBase + ".hair.*.png");
	else
		mHairFile = getPreviousFileMatchingPattern(mHairFile, DIR_AVA + mBase + ".hair.*.png");
		
	Update();
}

void AvatarCreator::_redraw()
{
	uShort w, h;
	
	mRedraw = false;
	
	mHeadColor.r = getScrollbarValue(mHeadFrame, "r");
	mHeadColor.g = getScrollbarValue(mHeadFrame, "g");
	mHeadColor.b = getScrollbarValue(mHeadFrame, "b");
	if (isGreyscale(mHeadColor))
	{
		if (mHeadColor.r > 0)
			--mHeadColor.r;
		else
			++mHeadColor.r;
	}
	
	mBodyColor.r = getScrollbarValue(mBodyFrame, "r");
	mBodyColor.g = getScrollbarValue(mBodyFrame, "g");
	mBodyColor.b = getScrollbarValue(mBodyFrame, "b");
	if (isGreyscale(mBodyColor))
	{
		if (mBodyColor.r > 0)
			--mBodyColor.r;
		else
			++mBodyColor.r;
	}
	
	mHairColor.r = getScrollbarValue(mHairFrame, "r");
	mHairColor.g = getScrollbarValue(mHairFrame, "g");
	mHairColor.b = getScrollbarValue(mHairFrame, "b");
	if (isGreyscale(mHairColor))
	{
		if (mHairColor.r > 0)
			--mHairColor.r;
		else
			++mHairColor.r;
	}
	
	resman->Unload(mCompositePreview);

	resman->Unload(mHead);
	mHead = resman->LoadImg(DIR_AVA + mHeadFile);
	if (!mHead)
	{
		SetError("Could not load " + mHeadFile);
		return;
	}

	resman->Unload(mBody);
	mBody = resman->LoadImg(DIR_AVA + mBodyFile);
	if (!mBody)
	{
		SetError("Could not load " + mBodyFile);
		return;
	}

	resman->Unload(mHair);
	mHair = resman->LoadImg(DIR_AVA + mHairFile);
	if (!mHair)
	{
		SetError("Could not load " + mHairFile);
		return;
	}

	//calculate our dimensions based on the body. (All parts should have the same dimensions with this system)
	//Each should contain 2 frames, 5 rows (4 dirs and 1 sit)
	w = mBody->Width() / 2;
	h = mBody->Height() / 5;

	mCompositePreview = resman->NewImage(mBody->Width(), mBody->Height(), color(255,0,255), true);

	/*
		Simple method. Assuming each image is the same size and we don't crop parts (as we SHOULD),
		just render it directly to mCompositePreview (0, 0)
		After EACH PART is rendered, call a color replacement on mCompositePreview so that we can replace
		the colors on that rather than on each part seperately. (This way we don't have to continuously reload 
		the same files when the colors are changed, and can keep the originals in memory)
		
		The more complex technique will be... much more complex as we must determine the base type, and from that
			determine the source coordinates for each section. Overall, annoying. 
	*/

	mHead->Render(mCompositePreview, 0, 0);
	mCompositePreview->ColorizeGreyscale(mHeadColor);

	mBody->Render(mCompositePreview, 0, 0);
	mCompositePreview->ColorizeGreyscale(mBodyColor);

	mHair->Render(mCompositePreview, 0, 0);
	mCompositePreview->ColorizeGreyscale(mHairColor);

	resman->Unload(mFullComposite);
	mFullComposite = resman->NewImage(mBody->Width(), mBody->Height(), color(), false);
	
	if (mFullComposite)
	{
		mFullComposite->DrawRect(rect(0, 0, mFullComposite->Width(), mFullComposite->Height()), color(255,0,255));
		mCompositePreview->Render(mFullComposite, 0, 0);
	}
	
	//Finally convert the resulting avatar to something we can control in terms of frames/animation
	mCompositePreview->ConvertToAvatarFormat(w, h, 1000, true, false);
	mCompositePreview->Stop();

}

void AvatarCreator::Render(uLong ms)
{
	//redraw (if necessary) and render mCompositePreview and the background image.
	if (mRedraw)
		_redraw();
		
	Image* scr = Screen::Instance();
	rect r = GetScreenPosition();
	
	Frame::Render(ms);	
	
	if (mCompositePreview)
	{
		mCompositePreview->Render(scr, r.x + 10, r.y + 50);	
	}
}

void AvatarCreator::AddDesignToFavorites()
{
	//Construct the url in the form avy://Base.HeadId.HeadHex.BodyId.BodyHex.HairId.HairHex
	
	string url = "avy://";
	url += mBase + ".";

	string s;
	s = mHeadFile.substr(0, mHeadFile.length() - 4); //get rid of .png
	s.erase(0, s.find_last_of(".")+1); //erase everything before the id
	url += s + "." + colorToHex(mHeadColor) + ".";
	
	s = mBodyFile.substr(0, mBodyFile.length() - 4); //get rid of .png
	s.erase(0, s.find_last_of(".")+1); //erase everything before the id
	url += s + "." + colorToHex(mBodyColor) + ".";
	
	s = mHairFile.substr(0, mHairFile.length() - 4); //get rid of .png
	s.erase(0, s.find_last_of(".")+1); //erase everything before the id
	url += s + "." + colorToHex(mHairColor);
	
	//add to favorites
	bool createdFavorites = false;
	if (!avatarFavorites)
	{
		//create the window in the background to access it
		createdFavorites = true;
		avatarFavorites = new AvatarFavorites();
		avatarFavorites->SetVisible(false);
	}
		
	avatarFavorites->Add(url, mCompositePreview->Width(), mCompositePreview->Height(), 
							"", 1000, false, false);
		
	if (game && game->mPlayer)
		game->mPlayer->LoadAvatar(url, "", mCompositePreview->Width(), mCompositePreview->Height(), 1000, false, false);
		
	//if we were the ones to bring this up, destroy it again
	if (createdFavorites)
		avatarFavorites->Die();
	
	new MessagePopup("", "Avatar Saved", "Saved in Favorites as: " + url);
	
}

void AvatarCreator::SaveToFile()
{
	if (!mFullComposite)
		return;
	
	string file = "saved/creation_" + timestamp(true) + ".png";
	mFullComposite->SavePNG(file);
	new MessagePopup("", "Avatar Saved", "Saved to " + file);
}


