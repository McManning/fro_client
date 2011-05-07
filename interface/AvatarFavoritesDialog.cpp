
#include <lua.hpp>
#include "AvatarFavoritesDialog.h"
#include "AvatarCreator.h"

#include "../core/widgets/Button.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/Checkbox.h"
#include "../core/widgets/YesNoPopup.h"

#include "../entity/LocalActor.h"
#include "../avatar/Avatar.h"
#include "../game/GameManager.h"
//#include "../game/Achievements.h"

AvatarFavorites* avatarFavorites;

void callback_avatarEditClose(Button* b)
{
	//cancel operation
	avatarFavorites->mAvatarEdit->Die();
	avatarFavorites->mAvatarEdit = NULL;
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
		
		prop->flags = 0;
		if (getCheckboxState(avatarFavorites->mAvatarEdit, "loopsit"))
		  prop->flags |= AVATAR_FLAG_LOOPSIT;
		  
		if (getCheckboxState(avatarFavorites->mAvatarEdit, "loopstand"))
		  prop->flags |= AVATAR_FLAG_LOOPSTAND;

		avatarFavorites->Add(prop);
	}
	avatarFavorites->mAvatarEdit->Die();
	avatarFavorites->mAvatarEdit = NULL;
	avatarFavorites->SetActive(true);
	avatarFavorites->Save();
}

AvatarEdit::AvatarEdit(avatarProperties* prop)
	: Frame(NULL, "AvatarEdit_" + string((prop->url.empty()) ? "Add" : "Edit"), rect(),
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
		c->SetState(prop->flags & AVATAR_FLAG_LOOPSTAND);

	c = new Checkbox(this, "loopsit", rect(10,130), "Loop Sit Animation", 0);
		c->SetState(prop->flags & AVATAR_FLAG_LOOPSIT);

	mAlertLabel = new Label(this, "", rect(10,152), "");
	mAlertLabel->mFontColor = color(255,0,0);

	Button* b;
	b = new Button(this, "save",rect(230,152,20,20), "", callback_avatarEditSave);
		b->mHoverText = "Save Avatar";
		b->SetImage("assets/buttons/okay.png");

	mCurrentProperties = prop;
	
	Center();
	ResizeChildren();
	
	if (avatarFavorites)
		avatarFavorites->SetActive(false);
}

AvatarEdit::~AvatarEdit()
{
	if (avatarFavorites)
		avatarFavorites->SetActive(true);
}

/////////////////////////////////////////////////////////////////////////////////

void callback_yesDeleteAvatar(YesNoPopup* yn)
{
	if (avatarFavorites)
	{
		avatarFavorites->SetActive(true);	
		avatarFavorites->EraseSelected();
	}
}

void callback_noDeleteAvatar(YesNoPopup* yn)
{
	if (avatarFavorites)
	{
		avatarFavorites->SetActive(true);
	}
}

void callback_avatarFavorites(Button* b)
{
	if (b->mId == "del")
	{
		if (avatarFavorites->mList->mSelected > -1)
		{
			avatarProperties* ap = avatarFavorites->mAvatars.at(avatarFavorites->mList->mSelected);
			YesNoPopup* yn = new YesNoPopup("", "Confirm Erase Avatar", 
											"Are you sure you want to erase " + ap->id, false);
											
			yn->onYesCallback = callback_yesDeleteAvatar;
			yn->onNoCallback = callback_noDeleteAvatar;
			
			avatarFavorites->SetActive(false);
		}
		//avatarFavorites->EraseSelected();
	}
	else if (b->mId == "new")
	{
		avatarFavorites->AddNew();
	}
	else if (b->mId == "use")
	{
		avatarFavorites->UseSelected();
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

AvatarFavorites::AvatarFavorites()
	: Frame(gui, "AvatarFavorites", rect(0, 0, 250,250),
			"My Avatars", true, true, true, true)
{
	mList = makeList(this, "list", rect(0,0,0,0));

	//make bottom buttons
	mNew = new Button(this, "new", rect(0,0,20,20), "", callback_avatarFavorites);
		mNew->mHoverText = "Add New Avatar";
		mNew->SetImage("assets/buttons/new_avatar.png");

	mEdit = new Button(this, "edit", rect(0,0,20,20), "", callback_avatarFavorites);
		mEdit->mHoverText = "Edit Selected";
		mEdit->SetImage("assets/buttons/edit_avatar.png");

	mDelete = new Button(this, "del", rect(0,0,20,20), "", callback_avatarFavorites);
		mDelete->mHoverText = "Delete Selected";
		mDelete->SetImage("assets/buttons/delete_avatar.png");

	mUse = new Button(this, "use", rect(0,0,20,20), "", callback_avatarFavorites);
		mUse->mHoverText = "Use Selected";
		mUse->SetImage("assets/buttons/use_avatar.png");

	mDesign = new Button(this, "", rect(0,0,20,20), "", callback_createAvatar);
		mDesign->mHoverText = "Open Avatar Designer";
		mDesign->SetImage("assets/buttons/design_avatar.png");

	ResizeChildren(); //get them into position
	Center();
	
	avatarFavorites = this;
	mAvatarEdit = NULL;
	mAvatarCreator = NULL;

	Load();

	mList->SetTopLine(0); //set it to display the top of the list.

}

AvatarFavorites::~AvatarFavorites()
{
	Save();

	for (int i = 0; i < mAvatars.size(); i++)
		delete mAvatars.at(i);

	avatarFavorites = NULL;	
	
	if (mAvatarEdit)
		mAvatarEdit->Die();
		
	if (mAvatarCreator)
		mAvatarCreator->Die();
}

void AvatarFavorites::EditSelected()
{
	if (mList->mSelected == -1) //nothing selected
		return;

	// If it's an avy://, edit with AvatarCreator
	if (mAvatars.at(mList->mSelected)->url.find("avy://", 0) == 0)
	{
		mAvatarCreator = new AvatarCreator();
		mAvatarCreator->ImportUrl(mAvatars.at(mList->mSelected)->url);
	}
	else //edit with AvatarEdit
	{
		mAvatarEdit = new AvatarEdit(mAvatars.at(mList->mSelected)); //add a new one~
		gui->Add(mAvatarEdit); //Not a child of ME, but instead the gui system.
	}

}

void AvatarFavorites::UseSelected()
{
	if (mList->mSelected == -1 || !game->mPlayer)
		return;

	avatarProperties* ap = mAvatars.at(mList->mSelected);
	
	if (!game->mPlayer->mCanChangeAvatar && game->GetChat())
	{
		game->GetChat()->AddMessage("\\c900 * You cannot change avatars on this map!");
		return;
	}
	
	timer* t = timers->Find("avywait");
	if (t)
	{
		int seconds = t->lastMs + t->interval - gui->GetTick();
		if (seconds < 0)
			seconds = 0;
		else
			seconds /= 1000;

        if (game->GetChat())
		  game->GetChat()->AddMessage("\\c900 * You must wait " + its(seconds+1) + " seconds.");
		  
		return;
	}
	else
	{
		timers->Add("avywait", AVYCHANGE_INTERVAL_MS, false, NULL, NULL, NULL);
	}
	
	//achievement_FashionAddict();
	game->mPlayer->LoadAvatar(	ap->url, ap->pass, 
								ap->w, ap->h, ap->delay, 
								ap->flags
							);
}

void AvatarFavorites::AddNew()
{
	//create new and set defaults
	avatarProperties* prop = new avatarProperties;
	prop->w = 0;
	prop->h = 0;
	prop->delay = 1000;
	prop->flags = 0;

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
	if (!prop)
		return NULL;
		
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

	Save();

	return prop;
}

avatarProperties* AvatarFavorites::Add(string url, uShort w, uShort h, string pass, 
                                        uShort delay, uShort flags)
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
    prop->flags = flags;

	return Add(prop);
}

avatarProperties* AvatarFavorites::Find(string url)
{
	for (int i = 0; i < mAvatars.size(); ++i)
	{
		if (mAvatars.at(i)->url == url)
			return mAvatars.at(i);
	}
	
	return NULL;
}

bool AvatarFavorites::AvatarPropertiesFromLuaTable(lua_State* ls, avatarProperties* props)
{
    string key;
    
    if (!lua_istable(ls, -1))
    	return false;

	lua_pushnil(ls);
	while (lua_next(ls, -2) != 0)
	{
		if (lua_isstring(ls, -2))
		{
			key = lua_tostring(ls, -2);
			if (key == "url")
				props->url = lua_tostring(ls, -1);
			else if (key == "w")
				props->w = (uShort)lua_tonumber(ls, -1);
			else if (key == "h")
				props->h = (uShort)lua_tonumber(ls, -1);
			else if (key == "pass")
				props->pass = lua_tostring(ls, -1);
			else if (key == "delay")
				props->delay = (uShort)lua_tonumber(ls, -1);
			else if (key == "flags")
				props->flags = (uShort)lua_tonumber(ls, -1);
		}
		lua_pop(ls, 1); //pop value	
    }
    
    return true;
}

bool AvatarFavorites::Load()
{
	string filename = DIR_PROFILE;
	filename += AVATAR_FAVORITES_FILENAME;

    lua_State* ls = luaL_newstate();
	
	if ( luaL_dofile( ls, filename.c_str() ) != 0 )
	{
		string err = lua_tostring(ls, -1);
		
		console->AddMessage("[AvatarFavorites] lua parse error: " + err);
        lua_close(ls);
		return false;
	}
    
	lua_getglobal(ls, "Avatars");

	// if this isn't a table, file is broken
	if (!lua_istable(ls, -1))
	{
		console->AddMessage("[AvatarFavorites] no Avatars table");
		lua_close(ls);
		return false;
	}

	// parse through the avatars
	lua_pushnil(ls);
	
	avatarProperties* props;
	while (lua_next(ls, -2) != 0)
	{
        props = new avatarProperties;
		AvatarPropertiesFromLuaTable(ls, props);
		Add(props);
		
		lua_pop(ls, 1); //pop value	
	}
	
	lua_close(ls);
	return true;
}

/*
    Avatars = {
		["FF00FF"] = 
			{
		        { ["url"] = "url", ["w"] = w, ["h"] = h, ["delay"] = delay, ["pass"] = "pass", ["flags"] = flags },
		        { ... },
		        ...  
			},
		... 
    }
*/
bool AvatarFavorites::Save()
{
    string filename = DIR_PROFILE;
	filename += AVATAR_FAVORITES_FILENAME;

    FILE* f;
	f = fopen(filename.c_str(), "w");

	if (!f)
	   return false;

    fprintf(f, "Avatars = {\n");

	//Write all the items
	avatarProperties* props;
    for (int i = mAvatars.size() - 1; i > -1; i--)
	{
        props = mAvatars.at(i);
        
        fprintf(f, "\t{ [\"url\"] = \"%s\", [\"w\"] = %i, [\"h\"] = %i, [\"delay\"] = %i, [\"pass\"] = \"%s\", [\"flags\"] = %i },\n", 
                    props->url.c_str(),
                    props->w,
                    props->h,
                    props->delay,
                    props->pass.c_str(),
                    props->flags
                );
	}
	
	fprintf(f, "}\n");
	fclose(f);
	return true;
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
	if (r.w > 155 && r.h > 99)
		Frame::SetPosition(r);
}
