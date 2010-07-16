
#ifndef _AVATARFAVORITESDIALOG_H_
#define _AVATARFAVORITESDIALOG_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

const char* const AVATAR_FAVORITES_FILENAME = "avatars.xml";
 
const int AVYCHANGE_INTERVAL_MS = 5000;
 
//TODO: Just use Avatar class!
struct avatarProperties
{
	string id;
	string url;
	string pass;
	uShort w;
	uShort h;
	uShort delay;
	bool loopStand;
	bool loopSit;
};

class Label;
struct avatarProperties;
class AvatarEdit : public Frame 
{
  public:
	AvatarEdit(avatarProperties* prop);
	~AvatarEdit();
	
	Label* mAlertLabel;
	
	avatarProperties* mCurrentProperties;
};

/*
	In the future, it'd be nice if this held Avatar classes instead of
	Avatar Properties. This way we can then say something like "test avatar"
	and it'll download and display in another window.
*/
class Button;
class Multiline;
class AvatarCreator;
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
	avatarProperties* Add(avatarProperties* prop);
	
	avatarProperties* Add(string url, uShort w, uShort h, string pass, 
							uShort delay, bool loopStand, bool loopSit);
	
	avatarProperties* Find(string url);
	
	void EraseSelected();
	bool Load();
	bool Save();
	
	Button* mNew;
	Button* mUse;
	Button* mDelete;
	Button* mEdit;
	Button* mDesign;
	Multiline* mList;

	std::vector<avatarProperties*> mAvatars;

	AvatarEdit* mAvatarEdit;
	AvatarCreator* mAvatarCreator;
};

extern AvatarFavorites* avatarFavorites; //<-- todo: GetInstance()

#endif //_AVATARFAVORITESDIALOG_H_
