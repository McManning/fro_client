
#include <lua.hpp>
#include "PlayerLib.h"
#include "LuaCommon.h"
#include "../entity/LocalActor.h"
#include "../game/GameManager.h"
#include "../interface/Inventory.h"

/********************************
*	MISC
********************************/

// .GetActor() return cptr to our controlled entity, so that it can be manipulated via the Entity lib
int player_GetActor(lua_State* ls)
{
	PRINT("player_GetActor");

	lua_pushlightuserdata(ls, game->mPlayer);
	return 1;
}

int player_IsInChatMode(lua_State* ls)
{
	lua_pushboolean(ls, game->mGameMode == GameManager::MODE_CHAT);
	return 1;	
}

//.Warp("id", x<-1>, y<-1>) - Warp to the specified map. If x, y are not supplied, will warp to default spawn point.
// OR .Warp("id", "objectname") - Warp to the specified object on the target map
// OR .Warp(x, y) - Warp to specified coordinates on the current map
int player_Warp(lua_State* ls)
{
	PRINT("player_Warp");
	luaCountArgs(ls, 1);
	
	int numArgs = lua_gettop(ls);
	
	string id;
	string name;
	point2d p;
	
	if (lua_isstring(ls, 1))
	{
		name = lua_tostring(ls, 1);
		
		if (numArgs > 1)
		{
			if (lua_isstring(ls, 2))
			{
				name = lua_tostring(ls, 2);
			}
			else if (numArgs > 2)
			{
				p.x = (int)lua_tonumber(ls, 2);
				p.y = (int)lua_tonumber(ls, 3);
			}
		}
		
		game->mPlayer->Warp(id, p, name);
	}
	else if (lua_isnumber(ls, 1) && numArgs > 1) //change coordinates on current map
	{
		p.x = (int)lua_tonumber(ls, 1);
		p.y = (int)lua_tonumber(ls, 2);
		game->mPlayer->SetPosition(p);
	}

	return 0;
};

/********************************
*	INVENTORY RELATED
********************************/

//.GiveItem("id", "description", amount, cost) - Adds the detailed item to our inventory. 
//cost and amount default to 1 if not supplied, description defaults to blank string. Amount must be >= 1. id can't be blank.
int player_GiveItem(lua_State* ls)
{
	PRINT("player_GiveItem");
	luaCountArgs(ls, 1);

	int numArgs = lua_gettop(ls);
	int amount = 1, cost = 1;
	string description, id;
	
	id = lua_tostring(ls, 1);
	if (id.empty())
		return 0;
	
	if (numArgs > 1)
		description = lua_tostring(ls, 2);
	
	if (numArgs > 2)
		amount = (int)lua_tonumber(ls, 3);
	
	if (numArgs > 3)
		cost = (int)lua_tonumber(ls, 4);
	
	ASSERT(inventory);

	inventory->Add(id, description, amount, cost);

	return 0;
}

//.HasItem("id", amount) returns 1 if has at least amount of the item, 0 otherwise. id can't be blank. Amount defaults to 1.
int player_HasItem(lua_State* ls)
{
	PRINT("player_HasItem");
	luaCountArgs(ls, 1);

	ASSERT(inventory);
	
	int numArgs = lua_gettop(ls);
	
	int amount = 1;
	if (numArgs > 1)
		amount = (int)lua_tonumber(ls, 2);
		
	if (amount < 1)
		amount = 1;
	
	lua_pushnumber( ls, inventory->Has(lua_tostring(ls, 1), amount) );
	return 1;
}

//.TakeItem("id", amount) Takes up to amount number of "id" from our inventory. id can't be blank.
int player_TakeItem(lua_State* ls)
{
	PRINT("player_TakeItem");
	luaCountArgs(ls, 2);

	ASSERT(inventory);

	inventory->Erase(lua_tostring(ls, 1), (uShort)lua_tonumber(ls, 2), true);

	return 0;
}

// .GetCash() - Returns total dorra 
int player_GetCash(lua_State* ls)
{
	PRINT("player_GetCash");
	
	ASSERT(inventory);
	
	lua_pushnumber(ls, inventory->GetCash());
	return 1;
}

// .AddCash(amount) - Adds (or subtracts if the amount is negative) to our dorra total
int player_AddCash(lua_State* ls)
{
	PRINT("player_AddCash");
	luaCountArgs(ls, 1);
	
	ASSERT(inventory);
	
	int amt = (int)lua_tonumber(ls, 1);
	int mydorra = inventory->GetCash();
	
	if (mydorra + amt < 0)
		amt = mydorra * -1;

	game->mChat->AddMessage("\\c990 * " + string( (amt < 0) ? "Lost " : "Gained " ) + 
							its((amt < 0) ? -amt : amt) + " Dorra!");
	
	inventory->SetCash( mydorra + amt );
	return 0;
}

// .IsLocked() - Returns nonzero if player input is being ignored
int player_IsLocked(lua_State* ls)
{
	PRINT("player_IsLocked");
	
	lua_pushnumber( ls, game->mPlayer->mIsLocked );
	return 1;	
}

// .Lock(1 or 0) - Lock the player. If locked (1), player cannot move their actor.
int player_Lock(lua_State* ls)
{
	PRINT("player_Lock");
	luaCountArgs(ls, 1);
	
	game->mPlayer->mIsLocked = lua_tonumber(ls, 1);

	return 0;	
}

//	.EarnAchievement(title, desc<can only be set once>, max<can only be set once>, file<can only be set once>)
//		Will +1 the total for the achievement. If total == max, display to the player
int player_EarnAchievement(lua_State* ls)
{
	PRINT("player_EarnAchievement");
	luaCountArgs(ls, 1);
	int numArgs = lua_gettop(ls);
	
	string title, desc, file;
	int max = 1;
	
	title = lua_tostring(ls, 1);
	if (numArgs > 1)
		desc = lua_tostring(ls, 2);
	
	if (numArgs > 2)
		max = (int)lua_tonumber(ls, 3);
		
	if (numArgs > 3)
		file = lua_tostring(ls, 4);
	
	game->EarnAchievement(title, desc, max, file);
	
	return 0;
}

//	.GetAchievement(id) - Returns a table in the form: table[id => "", description => "", max => 1, total => 1]
//		if they have earned the achievement. Or nothing if we don't have it.
int player_GetAchievement(lua_State* ls)
{
	PRINT("player_GetAchievement");
	luaCountArgs(ls, 1);
	
	if (!lua_isstring(ls, 1))
		return 0;
	
	string title = lua_tostring(ls, 1);

	XmlFile* xf = &game->mPlayerData;
		
	//Get master element
	TiXmlElement* top = xf->mDoc.FirstChildElement();
	TiXmlElement* e;
	
	ASSERT(top);

	e = top->FirstChildElement("achievements");
	if (e)
	{
		e = e->FirstChildElement();
		while (e)
		{
			if (xf->GetParamString(e, "title") == title)
			{
				lua_newtable(ls);
				int top = lua_gettop(ls);
				
				//insert our data into a new table
				lua_pushstring( ls, "title" );
				lua_pushstring( ls, title.c_str() );
				
				lua_pushstring( ls, "description" );
				lua_pushstring( ls, xf->GetParamString(e, "desc").c_str() );
				
				lua_pushstring( ls, "max" );
				lua_pushnumber( ls, xf->GetParamInt(e, "max") );
				
				lua_pushstring( ls, "total" );
				lua_pushnumber( ls, xf->GetParamInt(e, "total") );
				
				lua_settable(ls, top);
				
				return 1;
			}
			e = e->NextSiblingElement();
		}
	}

	return 0;
}

static const luaL_Reg functions[] = {
	{"GetActor", player_GetActor},
	{"IsInChatMode", player_IsInChatMode},
	{"Warp", player_Warp},
	{"GiveItem", player_GiveItem},
	{"HasItem", player_HasItem},
	{"TakeItem", player_TakeItem},
	{"GetCash", player_GetCash},
	{"AddCash", player_AddCash},
	{"IsLocked", player_IsLocked},
	{"Lock", player_Lock},
	{"EarnAchievement", player_EarnAchievement},
	{"GetAchievement", player_GetAchievement},
	{NULL, NULL}
};

void RegisterPlayerLib(lua_State* ls)
{
	luaL_register( ls, "Player", functions );
}
