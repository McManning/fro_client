
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
