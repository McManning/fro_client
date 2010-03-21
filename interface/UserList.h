
#ifndef _USERLIST_H_
#define _USERLIST_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

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

#endif //_USERLIST_H_
