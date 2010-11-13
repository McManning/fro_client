
#ifndef _ITEMSDATABASEMANAGER_H_
#define _ITEMSDATABASEMANAGER_H_

#include "../core/Core.h"

const char* const BACKPACK_ITEMS_DATABASE_FILE = "items.lua";

struct lua_State;
class BackpackItem;
class ItemsDatabaseManager
{
  public:
	
	ItemsDatabaseManager();
	~ItemsDatabaseManager();

	/**
		Loads the item database and prepares it for access
	*/
	void LoadDatabase();
	
	/**	
		@param item The backpack item that requests to be set up. It must already have a valid index set before calling GetDetails
		@return true if the BackpackItem has been successfully detailed, false if something goes wrong. (Item doesn't exist, etc)
	*/
	bool GetDetails(BackpackItem* item);
	
  private:
	void _callGlobal(const char* func, int index, string& result);
	
	lua_State* mDatabase;
};

extern ItemsDatabaseManager g_itemsDabaseManager;

#endif //_ITEMSDATABASEMANAGER_H_
