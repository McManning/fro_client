
#include "AvatarFavoritesDialog.h"
#include "AvatarCreator.h"

#include "../core/widgets/Button.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/Checkbox.h"

#include "../entity/LocalActor.h"
#include "../game/GameManager.h"

AvatarFavorites* avatarFavorites;

void callback_avatarEditClose(Button* b)
{
	//cancel operation
	avatarFavorites->mAvatarEdit->Die();
	avatarFavorites->mAvatarEdit = NULL;
	avatarFavorites->SetActive(true);
}

void callback_avatarEditSave(Button* b)
{
	avatarProperties* prop = avatarFavorites->mAvatarEdit->mCurrentProperties;
	//Verify values and set properties, then give the result back to avatarFavorites.
	if (prop)
	{
		string url = getInputText(avatarFavorites->mAvatarEdit, "url");
		if (url.empty()) // || url.find("http://", 0) != 0)
		{
			avatarFavorites->mAvatarEdit->mAlertLabel->SetCaption("Invalid Url");
			return;
		}
		prop->url = url;

		int i;
		i = sti( getInputText(avatarFavorites->mAvatarEdit, "width") );
		if (i < 0 || i > MAX_AVATAR_WIDTH)
		{
			avatarFavorites->mAvatarEdit->mAlertLabel->SetCaption(
						"Invalid Width. (0 to " + its(MAX_AVATAR_WIDTH) + " pixels)");
			return;
		}
		prop->w = i;

		i = sti( getInputText(avatarFavorites->mAvatarEdit, "height") );
		if (i < 0 || i > MAX_AVATAR_HEIGHT)
		{
			avatarFavorites->mAvatarEdit->mAlertLabel->SetCaption(
						"Invalid Height. (0 to " + its(MAX_AVATAR_HEIGHT) + " pixels)");
			return;
		}
		prop->h = i;

		i = sti( getInputText(avatarFavorites->mAvatarEdit, "delay") );
		if (i < 1)
		{
			avatarFavorites->mAvatarEdit->mAlertLabel->SetCaption("Invalid Delay. (1 to 9999)");
			return;
		}
		prop->delay = i;

		prop->pass = getInputText(avatarFavorites->mAvatarEdit, "pass");
		prop->loopSit = getCheckboxState(avatarFavorites->mAvatarEdit, "loopsit");
		prop->loopStand = getCheckboxState(avatarFavorites->mAvatarEdit, "loopstand");

		avatarFavorites->Add(prop);
	}
	avatarFavorites->mAvatarEdit->Die();
	avatarFavorites->mAvatarEdit = NULL;
	avatarFavorites->SetActive(true);
}

AvatarEdit::AvatarEdit(avatarProperties* prop) :
	Frame(NULL, "editavy", rect(),
			(prop->url.empty()) ? "Add Avatar" : "Edit Avatar", true, false, true, true)
{
	mClose->onClickCallback = callback_avatarEditClose; //so we can run some custom code while it closes

	SetSize(260, 180);

	Input* i;
	new Label(this, "", rect(10,30), "Url:");
	i = new Input(this, "url", rect(60,30,190,20), "", 0, true, NULL);
		i->SetText(prop->url);

	new Label(this, "", rect(10,55), "Width:");
	i = new Input(this, "width", rect(60,55,55,20), "0123456789", 3, true, NULL);
		i->mHoverText = "Single frame width in pixels \\c600(For PNGs Only)";
		i->SetText(its(prop->w));
		i->SetMenuEnabled(false);
		
	new Label(this, "", rect(130,55), "Height:");
	i = new Input(this, "height", rect(195,55,55,20), "0123456789", 3, true, NULL);
		i->mHoverText = "Single frame height in pixels \\c600(For PNGs Only)";
		i->SetText(its(prop->h));
		i->SetMenuEnabled(false);

	new Label(this, "", rect(10,80), "Delay:");
	i = new Input(this, "delay", rect(60,80,55,20), "0123456789", 4, true, NULL);
		i->mHoverText = "Time to display each frame in milliseconds \\c600(For PNGs Only)";
		i->SetText(its(prop->delay));
		i->SetMenuEnabled(false);

	new Label(this, "", rect(130,80), "Password:");
	i = new Input(this, "pass", rect(195,80,55,20), "", 0, true, NULL);
		i->mHoverText = "Encryption Password \\c700(Not working yet)";
		i->SetText(prop->pass);
		i->SetMenuEnabled(false);

	Checkbox* c;
	c = new Checkbox(this, "loopstand", rect(10,105), "Loop Stand Animation", 0);
		c->SetState(prop->loopStand);

	c = new Checkbox(this, "loopsit", rect(10,130), "Loop Sit Animation", 0);
		c->SetState(prop->loopSit);

	mAlertLabel = new Label(this, "", rect(10,152), "");
	mAlertLabel->mFontColor = color(255,0,0);

	Button* b;
	b = new Button(this, "save",rect(230,152,20,20), "", callback_avatarEditSave);
		b->mHoverText = "Save Avatar";
		b->SetImage("assets/buttons/go.png");

	mCurrentProperties = prop;
	
	Center();
	ResizeChildren();
}

AvatarEdit::~AvatarEdit()
{
	//lol nothing
}

/////////////////////////////////////////////////////////////////////////////////

void callback_avatarFavorites(Button* b)
{
	if (b->mId == "del")
	{
		avatarFavorites->EraseSelected();
	}
	else if (b->mId == "new")
	{
		avatarFavorites->AddNew();
	}
	else if (b->mId == "use")
	{
		avatarFavorites->UseSelected();
		avatarFavorites->Die();
	}
	else if (b->mId == "edit")
	{
		avatarFavorites->EditSelected();
	}
}

void callback_createAvatar(Button* b)
{
	new AvatarCreator();	
}

AvatarFavorites::AvatarFavorites() :
	Frame(gui, "avyfavs", rect(0, 0, 250,250),
			"Avatar Favorites", true, true, true, true)
{
	mList = makeList(this, "list", rect(0,0,0,0));

	//make bottom buttons
	mNew = new Button(this, "new", rect(0,0,20,20), "", callback_avatarFavorites);
		mNew->mHoverText = "Add New Avatar";
		mNew->SetImage("assets/buttons/star_plus.png");

	mEdit = new Button(this, "edit", rect(0,0,20,20), "", callback_avatarFavorites);
		mEdit->mHoverText = "Edit Selected";
		mEdit->SetImage("assets/buttons/star_bar.png");

	mDelete = new Button(this, "del", rect(0,0,20,20), "", callback_avatarFavorites);
		mDelete->mHoverText = "Delete Selected";
		mDelete->SetImage("assets/buttons/star_minus.png");

	mUse = new Button(this, "use", rect(0,0,20,20), "", callback_avatarFavorites);
		mUse->mHoverText = "Use Selected";
		mUse->SetImage("assets/buttons/go.png");

	mDesign = new Button(this, "", rect(0,0,20,20), "", callback_createAvatar);
		mDesign->mHoverText = "Open Avatar Designer";
		mDesign->SetImage("assets/buttons/star_plus.png");

	ResizeChildren(); //get them into position
	Center();
	
	avatarFavorites = this;
	mAvatarEdit = NULL;

	Load();

	mList->SetTopLine(0); //set it to display the top of the list.

}

AvatarFavorites::~AvatarFavorites()
{
	Save();

	for (int i = 0; i < mAvatars.size(); i++)
		delete mAvatars.at(i);

	avatarFavorites = NULL;
}

void AvatarFavorites::EditSelected()
{
	if (mList->mSelected == -1) //nothing selected
		return;

	mAvatarEdit = new AvatarEdit(mAvatars.at(mList->mSelected)); //add a new one~
	gui->Add(mAvatarEdit); //Not a child of ME, but instead the gui system.
	SetActive(false);
}

void AvatarFavorites::UseSelected()
{
	if (mList->mSelected == -1 || !game->mPlayer)
		return;

	avatarProperties* ap = mAvatars.at(mList->mSelected);

/*	if (!mapManager->map->mLockAvatar)
	{
*/		game->mPlayer->LoadAvatar(	ap->url, ap->pass, 
									ap->w, ap->h, ap->delay, 
									ap->loopStand, ap->loopSit
									);
/*	}
	else //tell them they've been locked out
	{
		new ChoiceDialog("", "Error", "Avatars have been disabled in this area.",
                        false, false);
	}*/
}

void AvatarFavorites::AddNew()
{
	//create new and set defaults
	avatarProperties* prop = new avatarProperties;
	prop->w = 0;
	prop->h = 0;
	prop->delay = 1000;
	prop->loopStand = false;
	prop->loopSit = false;

	mAvatarEdit = new AvatarEdit(prop); //add a new one~
	gui->Add(mAvatarEdit); //Not a child of ME, but instead the gui system.
	SetActive(false);
}

void AvatarFavorites::EraseSelected()
{
	if (mList->mSelected > -1)
	{
		delete mAvatars.at(mList->mSelected);
		mAvatars.erase(mAvatars.begin() + mList->mSelected);
		mList->EraseLine(mList->mSelected);
	}
}

avatarProperties* AvatarFavorites::Add(avatarProperties* prop)
{
	//Build an ID off the url
	int i = prop->url.find_last_of('/')+1;
	if (i == string::npos || i > prop->url.size()-1)
		prop->id = prop->url;
	else
		prop->id = prop->url.substr(i);

	for (i = 0; i < mAvatars.size(); i++)
	{
		if (mAvatars.at(i) == prop)
		{
			mList->SetLine(i, prop->id);
			return prop;
		}
	}

	//didn't find, add as a new one to the top
	mAvatars.insert(mAvatars.begin(), prop);
	
	//rewrite list
	mList->Clear();
	for (i = 0; i < mAvatars.size(); i++)
		mList->AddMessage(mAvatars.at(i)->id);

	mList->SetTopLine(0);

	return prop;
}

avatarProperties* AvatarFavorites::Add(string url, uShort w, uShort h, string pass,
										uShort delay, bool loopStand, bool loopSit)
{
	if (url.empty()) return NULL;

	avatarProperties* prop = new avatarProperties;
	prop->url = url;

	if (w > MAX_AVATAR_WIDTH) w = MAX_AVATAR_WIDTH;
	if (h > MAX_AVATAR_HEIGHT) h = MAX_AVATAR_HEIGHT;

	prop->w = w;
	prop->h = h;

	if (delay < 1) delay = 1000;
	prop->delay = delay;

	prop->pass = pass;
	prop->loopStand = loopStand;
	prop->loopSit = loopSit;

	return Add(prop);
}

int callback_avatarFavoritesXmlParser(XmlFile* xf, TiXmlElement* e, void* userData)
{
	//OLD(v0.6.X): <avatar desc="$" url="$" w="#" h="#" params="$" />
	//NEW: <avatar url="$" w="#" h="#" delay="#" loopsit="1|0" loopstand="1|0" />

	AvatarFavorites* af = (AvatarFavorites*)userData;

	string id = e->Value();
	if (id == "avatar" && af)
	{
		af->Add(xf->GetParamString(e, "url"),
				xf->GetParamInt(e, "w"),
				xf->GetParamInt(e, "h"),
				xf->GetParamString(e, "pass"),
				xf->GetParamInt(e, "delay"),
				xf->GetParamInt(e, "loopstand"),
				xf->GetParamInt(e, "loopsit"));
	}
	return XMLPARSE_SUCCESS;
}

bool AvatarFavorites::Load()
{
	XmlFile xf;
	xf.SetParser(callback_avatarFavoritesXmlParser);

	//Load file contents into XML elements
	if (!xf.LoadFromFile(AVATAR_FAVORITES_FILENAME))
	{
		WARNING(xf.GetError());
		return false;
	}
	
	//backwards support for old version
	if (!xf.SetEntryPoint("avatars"))
		xf.SetEntryPoint("drm::avatars");

	bool success = (xf.Parse(this) == XMLPARSE_SUCCESS);
	return success;
}

bool AvatarFavorites::Save()
{
	//open new XML Doc
	XmlFile xf;

	//Write master element
	TiXmlElement* top = new TiXmlElement("avatars");
	xf.mDoc.LinkEndChild(top);

	//Write all the items
	TiXmlElement* e;
	for (int i = mAvatars.size() - 1; i > -1; i--)
	{
		e = xf.AddChildElement(top, "avatar");
		xf.SetParamString(e, "url", mAvatars.at(i)->url);
		xf.SetParamInt(e, "w", mAvatars.at(i)->w);
		xf.SetParamInt(e, "h", mAvatars.at(i)->h);
		xf.SetParamInt(e, "delay", mAvatars.at(i)->delay);
		xf.SetParamString(e, "pass", mAvatars.at(i)->pass);
		xf.SetParamInt(e, "loopsit", mAvatars.at(i)->loopSit);
		xf.SetParamInt(e, "loopstand", mAvatars.at(i)->loopStand);
	}

	//save and close
	bool success = xf.SaveToFile(AVATAR_FAVORITES_FILENAME);
	return success;
}

void AvatarFavorites::ResizeChildren() //overridden so we can move things around properly
{

	mList->SetPosition( rect(10, 30, Width() - 20, Height() - 60) );

	mUse->SetPosition( rect(Width()-43,Height()-25,20,20) );
	mDelete->SetPosition( rect(Width()-76,Height()-25,20,20) );
	mEdit->SetPosition( rect(Width()-101,Height()-25,20,20) );
	mNew->SetPosition( rect(Width()-126,Height()-25,20,20) );
	mDesign->SetPosition( rect(10,Height()-25,20,20) );
	
	Frame::ResizeChildren(); //takes care of titlebar stuff (close button, sizer, caption, etc)
}

void AvatarFavorites::SetPosition(rect r)
{
	if (r.w > 135 && r.h > 99)
		Frame::SetPosition(r);
}
