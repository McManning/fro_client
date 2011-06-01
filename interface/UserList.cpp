
#include "UserList.h"

#include "../game/GameManager.h"
#include "../entity/EntityManager.h"
#include "../map/Map.h"

#include "../core/widgets/Multiline.h"
#include "../core/widgets/Label.h"

UserList* userlist;

void callback_UserListRightClick(Multiline* m)
{
	UserList* u = (UserList*)m->GetParent();
	if (u)
		u->ClickSelected();	
}

UserList::UserList() :
	Console("UserList", "Characters: 0", "", color(178,204,237), true, false, false)
{
	//convert our output to list format
	mOutput->mWrap = false;
	mOutput->mHighlightSelected = true;
	mOutput->onRightSingleClickCallback = callback_UserListRightClick;
	
	mShowTimestamps = false;
	
	gui->Add(this);
	
	//resize width, default is too big. 
	SetSize(150, 130);
	Center();
	ResizeChildren();
	
	//populate with current list of users online
	if (game->mMap)
	{
		Entity* e;
		//Iterate list and add a line for each remote actor or ourself
		for (int i = 0; i < game->mMap->mEntities.size(); ++i)
		{
			e = game->mMap->mEntities.at(i);
			if (e && (e->mType == ENTITY_REMOTEACTOR || e->mType == ENTITY_LOCALACTOR))
			{
				AddNick(e->GetName());
			}
		}
	}
	
	userlist = this;
}

UserList::~UserList()
{
	userlist = NULL;
}

void UserList::ClickSelected()
{
	if (mOutput->mSelected < 0 || mOutput->mSelected >= mOutput->mLines.size()) return;

	Entity* e = game->mMap->FindEntityByName(mOutput->mLines.at(mOutput->mSelected), ENTITY_REMOTEACTOR);
	
	if (e)
	{
		MessageData md("CLICK_REMOTE_ACTOR");
		md.WriteUserdata("entity", e);
		md.WriteInt("userlist", 1); // selected from the userlist
		messenger.Dispatch(md, e);
	}
}

void UserList::AddNick(string nick)
{
	AddMessage(nick);
	
	if (mTitle)	//change count in title
		mTitle->SetCaption("Characters: " + its(mOutput->mLines.size()));
}

void UserList::RemoveNick(string nick)
{
	for (uShort i = 0; i < mOutput->mLines.size(); i++)
	{
		if (mOutput->mLines.at(i) == nick)
		{
			mOutput->EraseLine(i);
			
			if (mTitle)	//change count in title
				mTitle->SetCaption("Characters: " + its(mOutput->mLines.size()));
				
			return;
		}
	}
}

void UserList::ChangeNick(string oldNick, string newNick)
{
	for (uShort i = 0; i < mOutput->mLines.size(); i++)
	{
		if (mOutput->mLines.at(i) == oldNick)
		{
			mOutput->mLines.at(i) = newNick;
			return;
		}
	}
}

/* ********************** */

/*
void callback_UserlistButtonClick(UserlistButton* b)
{
	if (!userlist)
		userlist = new UserList();
}

UserlistButton::UserlistButton()
	: Button(gui, "", rect(x,y,w,h), "", callback_UserlistButtonClick)
{
	mNumbersImage = resman->LoadImg("assets/hud_level.png"); //HUD_LEVEL IS TEMP
	
	userlist = new Userlist();
	
}

UserlistButton::~UserlistButton()
{
	resman->Unload(mNumbersImage);	
}

void UserlistButton::Render()
{
	Image* scr = Screen::Instance();
	
	string s = its(PLAYERCOUNT);

	//render each number
	sShort yOffset = 1;
	sShort xOffset = 40 + 78;
	
	for (int i = 0; i < s.size(); ++i)
	{
		mNumbersImage->Render( scr, xOffset, r.y + 16 + yOffset, rect(78 + (s.at(i) - '0') * 21, 0, 21, 26) );

		xOffset += 18;
		yOffset *= -1; //invert offset	
	}
}
*/

