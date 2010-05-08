
#ifndef _USERLIST_H_
#define _USERLIST_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "../core/widgets/Button.h"


class UserList : public Console 
{
  public:
	UserList();
	~UserList();
	
	void ClickSelected();
	
	void ChangeNick(string oldNick, string newNick);
	void RemoveNick(string nick);
	void AddNick(string nick);
};

extern UserList* userlist;

/*
class UserlistButton : public Button
{
  public:
	UserlistButton();
	~UserlistButton();
	
	void Render();
	
  private:
	Image* mNumbersImage; //renderable numbers to indicate user count
};
*/

#endif //_USERLIST_H_
