
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


#ifndef _AVATARFAVORITES_H_
#define _AVATARFAVORITES_H_

#include "../core/widgets/Frame.h"
#include "../avatar/AvatarProperties.h"

const char* const AVATAR_FAVORITES_FILENAME = "avatars.lua";
 
const int AVYCHANGE_INTERVAL_MS = 5000;

/*
	In the future, it'd be nice if this held Avatar classes instead of
	Avatar Properties. This way we can then say something like "test avatar"
	and it'll download and display in another window.
*/
class Button;
class Multiline;
class AvatarCreator;
class AvatarEdit;

struct lua_State;
class AvatarFavorites : public Frame 
{
  public:
	AvatarFavorites();
	~AvatarFavorites();
	
	void ResizeChildren();
	void SetPosition(rect r);
	
	void EditSelected();
	void UseSelected();
	void AddNew();
	void EraseSelected();

	void LoadAvatarManager();
	int GetTotalFolders();
	int GetTotalAvatars(int iFolder);
	void GetAvatarProperties(int iFolder, int iIndex, AvatarProperties& result);
	void RemoveAvatar(int iFolder, int iIndex);
	void AddAvatar(int iFolder, AvatarProperties& props);
	void UpdateAvatar(int iFolder, int iIndex, AvatarProperties& props);
	bool SetWorkingFolder(int iFolder);
	
	Button* mNew;
	Button* mUse;
	Button* mDelete;
	Button* mEdit;
	Button* mDesign;
	Multiline* mList;

	AvatarEdit* mAvatarEdit;
	AvatarCreator* mAvatarCreator;
	
	int mWorkingFolder;
	lua_State* mLuaState;
};

extern AvatarFavorites* avatarFavorites; //<-- todo: GetInstance()

#endif //_AVATARFAVORITES_H_
