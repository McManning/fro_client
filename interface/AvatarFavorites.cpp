
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


#include <lua.hpp>
#include "AvatarFavorites.h"
#include "AvatarCreator.h"
#include "AvatarEdit.h"

#include "../core/widgets/Button.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/YesNoPopup.h"

#include "../entity/LocalActor.h"
#include "../avatar/Avatar.h"
#include "../game/GameManager.h"
//#include "../game/Achievements.h"

AvatarFavorites* avatarFavorites;

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
			YesNoPopup* yn = new YesNoPopup("", "Confirm Erase Avatar", 
											"Are you sure you want to erase " 
											+ avatarFavorites->mList->GetSelectedText(), false);
											
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
	mWorkingFolder = 1;

	LoadAvatarManager();

	mList->SetTopLine(0); //set it to display the top of the list.

}

AvatarFavorites::~AvatarFavorites()
{
	if (mLuaState)
		lua_close(mLuaState);
		
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

	AvatarProperties ap;
	GetAvatarProperties(mWorkingFolder, mList->mSelected+1, ap);

	// If it's an avy://, edit with AvatarCreator
	if (ap.url.find("avy://", 0) == 0)
	{
		mAvatarCreator = new AvatarCreator(mList->mSelected+1, ap.url);
		SetActive(false);
	}
	else //edit with AvatarEdit
	{
		mAvatarEdit = new AvatarEdit(mList->mSelected+1, &ap); //add a new one~
		SetActive(false);
	}

}

void AvatarFavorites::UseSelected()
{
	if (mList->mSelected == -1 || !game->mPlayer)
		return;

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
	
	
	AvatarProperties ap;
	GetAvatarProperties(mWorkingFolder, mList->mSelected+1, ap);
	
	//achievement_FashionAddict();
	game->mPlayer->LoadAvatar(	ap.url, ap.pass, 
								ap.w, ap.h, ap.delay, 
								ap.flags
							);
}

void AvatarFavorites::AddNew()
{
	mAvatarEdit = new AvatarEdit(); //add a new one
	SetActive(false);
}

void AvatarFavorites::EraseSelected()
{
	if (mList->mSelected > -1)
	{
		RemoveAvatar(mWorkingFolder, mList->mSelected+1);
	}
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

void AvatarFavorites::LoadAvatarManager()
{
	mLuaState = luaL_newstate();
	luaL_openlibs( mLuaState ); 

	if ( luaL_dofile( mLuaState, "assets/lua/AvatarManager.lua" ) != 0 )
	{
		string err = lua_tostring(mLuaState, -1);
		console->AddMessage("Failed to create AvatarManager: " + err);
	}
	
	SetWorkingFolder(mWorkingFolder);
}

int AvatarFavorites::GetTotalFolders()
{
	lua_getglobal(mLuaState, "GetTotalFolders");
	if (lua_pcall(mLuaState, 0, 1, 0) != 0)
	{
		console->AddMessage(lua_tostring(mLuaState, -1));	
	}


	int result = (int)lua_tonumber(mLuaState, -1);
	lua_pop(mLuaState, 1);
	
	return result;
}

int AvatarFavorites::GetTotalAvatars(int iFolder)
{
	lua_getglobal(mLuaState, "GetTotalAvatars");
		lua_pushnumber(mLuaState, iFolder);
		if (lua_pcall(mLuaState, 1, 1, 0) != 0)
		{
			console->AddMessage(lua_tostring(mLuaState, -1));	
		}
	
	int result = (int)lua_tonumber(mLuaState, -1);
	lua_pop(mLuaState, 1);
	
	return result;
}

void AvatarFavorites::GetAvatarProperties(int iFolder, int iIndex, AvatarProperties& result)
{
	lua_getglobal(mLuaState, "GetAvatarProperties");
		lua_pushnumber(mLuaState, iFolder);
		lua_pushnumber(mLuaState, iIndex);
		if (lua_pcall(mLuaState, 2, 1, 0) != 0)
		{
			FATAL(lua_tostring(mLuaState, -1));	
		}

	result.FromLuaTable(mLuaState, -1);
	lua_pop(mLuaState, 1);
}

void AvatarFavorites::RemoveAvatar(int iFolder, int iIndex)
{
	lua_getglobal(mLuaState, "RemoveAvatar");
		lua_pushnumber(mLuaState, iFolder);
		lua_pushnumber(mLuaState, iIndex);
		if (lua_pcall(mLuaState, 2, 0, 0) != 0)
		{
			console->AddMessage(lua_tostring(mLuaState, -1));	
		}
		
	if (iFolder == mWorkingFolder)
		SetWorkingFolder(iFolder);
}

/*
void AvatarFavorites::SortAvatars(int iSortMode)
{
	lua_getglobal(mLuaState, "SortAvatars");
		lua_pushnumber(mLuaState, iSortMode);
		lua_pcall(mLuaState, 1, 0, 0);

	// reload our view
	SetWorkingFolder(mWorkingFolder);
}
*/
void AvatarFavorites::AddAvatar(int iFolder, AvatarProperties& props)
{
	lua_getglobal(mLuaState, "AddAvatar");
		lua_pushnumber(mLuaState, iFolder);
		props.ToLuaTable(mLuaState);
		if (lua_pcall(mLuaState, 2, 0, 0) != 0)
		{
			console->AddMessage(lua_tostring(mLuaState, -1));	
		}
		
	if (iFolder == mWorkingFolder)
		SetWorkingFolder(iFolder);
}

void AvatarFavorites::UpdateAvatar(int iFolder, int iIndex, AvatarProperties& props)
{
	if (iIndex < 1)
	{
		AddAvatar(iFolder, props);
	}
	else
	{
		lua_getglobal(mLuaState, "UpdateAvatar");
			lua_pushnumber(mLuaState, iFolder);
			lua_pushnumber(mLuaState, iIndex);
			props.ToLuaTable(mLuaState);
			if (lua_pcall(mLuaState, 3, 0, 0) != 0)
			{
				console->AddMessage(lua_tostring(mLuaState, -1));	
			}
	
		if (iFolder == mWorkingFolder)
			SetWorkingFolder(iFolder);
	}
}

bool AvatarFavorites::SetWorkingFolder(int iFolder)
{
	// @todo GetFolderData() this folder for custom display garbage
	
	int avycount = GetTotalAvatars(iFolder);
	const char* name;
	
	if (avycount < 0)
		return false;
	
	mWorkingFolder = iFolder; // @todo I don't like this logic

	mList->Clear();
	for (int i = 1; i <= avycount; ++i)
	{
		lua_getglobal(mLuaState, "GetAvatarDisplayName");
			lua_pushnumber(mLuaState, iFolder);
			lua_pushnumber(mLuaState, i);
			if (lua_pcall(mLuaState, 2, 1, 0) != 0)
			{
				console->AddMessage(lua_tostring(mLuaState, -1));	
			}
		
		name = lua_tostring(mLuaState, -1);
		lua_pop(mLuaState, 1);
		
		if (name)
			mList->AddMessage(name);
	}

	mList->SetTopLine(0);
	return true;
}

