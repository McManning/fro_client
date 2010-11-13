
#include <lua.hpp>

#include "ItemsDatabaseManager.h"
#include "BackpackItem.h"

ItemsDatabaseManager g_itemsDabaseManager;

ItemsDatabaseManager::ItemsDatabaseManager()
{
	mDatabase = NULL;
	LoadDatabase();
}

ItemsDatabaseManager::~ItemsDatabaseManager()
{
	if (mDatabase)
		lua_close(mDatabase);
}

void ItemsDatabaseManager::LoadDatabase()
{
	mDatabase = luaL_newstate();
	luaL_openlibs( mDatabase );
	
	if ( luaL_dofile( mDatabase, BACKPACK_ITEMS_DATABASE_FILE ) != 0 )
	{
		string err = lua_tostring(mDatabase, -1);
		lua_close(mDatabase);

		FATAL("Could not load items database.\n\n" + err);
		return;
	}
}

void ItemsDatabaseManager::_callGlobal(const char* func, int index, string& result)
{
	lua_getglobal(mDatabase, func);

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(mDatabase, -1))
	{
		FATAL("No global " + string(func));
	}
	
	lua_pushnumber(mDatabase, index); //first argument

	if (lua_pcall(mDatabase, 0, 1, 0) != 0) // result = Build(), return of 0 indicates error.
	{
		FATAL("Failed to call global " + string(func) + "\n\n" + string(lua_tostring(mDatabase, -1)));
	}

	result = lua_tostring(mDatabase, -1);
	lua_pop(mDatabase, 1); //get rid of result from stack
}

/**	
	@param item The backpack item that requests to be set up. It must already have a valid index set before calling GetDetails
	@return true if the BackpackItem has been successfully detailed, false if something goes wrong. (Item doesn't exist, etc)
*/
bool ItemsDatabaseManager::GetDetails(BackpackItem* item)
{
	string s;
	int index = item->mIndex;
	if (index == 0)
		return false;
	
	_callGlobal("GetTitle", index, s);
	item->mId = s;
	
	_callGlobal("GetDescription", index, s);
	item->mDescription = s;
		
	_callGlobal("GetImage", index, s);
	item->mIconFile = s;
	
	_callGlobal("GetUseType", index, s);
	item->mUseType = sti(s);

	return true;
}



