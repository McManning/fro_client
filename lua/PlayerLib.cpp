
#include <lua.hpp>
#include "PlayerLib.h"
#include "LuaCommon.h"
#include "../core/TimerManager.h"
#include "../entity/LocalActor.h"
#include "../entity/Lunem.h"
#include "../game/GameManager.h"
#include "../interface/Inventory.h"
#include "../interface/PlayerActionMenu.h"
#include "../interface/LunemParty.h"
#include "../map/Map.h"

struct queuedPlayerWarp
{
	string id;
	point2d point;
	string obj;
};

uShort timer_doWarpQueue(timer* t, uLong ms)
{
	queuedPlayerWarp* w = (queuedPlayerWarp*)t->userData;
	game->mPlayer->Warp(w->id, w->point, w->obj);
	
	return TIMER_DESTROY;
}

void timer_destroyWarpQueue(timer* t)
{
	delete (queuedPlayerWarp*)t->userData;
	t->userData = NULL;
}

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
	
	if (lua_isnumber(ls, 1) && numArgs > 1) //change coordinates on current map
	{
		p.x = (int)lua_tonumber(ls, 1);
		p.y = (int)lua_tonumber(ls, 2);
		game->mPlayer->Warp(p);
	}
	else if (lua_isstring(ls, 1))
	{
		id = lua_tostring(ls, 1);

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
		// By calling Warp here, we destroy the lua script from within. This cannot be done.
		// So it's time to rig a timer!
		//game->mPlayer->Warp(id, p, name);
		
		queuedPlayerWarp* w = new queuedPlayerWarp;
		w->id = id;
		w->point = p;
		w->obj = name;
		timers->Add("", 0, true, timer_doWarpQueue, timer_destroyWarpQueue, w); 
	}

	return 0;
};

/********************************
*	INVENTORY RELATED
********************************/

//.GiveItem(tItem) - Adds the detailed item to our inventory. 
int player_GiveItem(lua_State* ls)
{
	luaCountArgs(ls, 1);

	ASSERT(inventory);
	inventory->LuaReadItemTable(ls, 1);

	return 0;
}

// tItem = .GetItem("id") - Returns the item table of the item if we have it, nil otherwise
int player_GetItem(lua_State* ls)
{
	luaCountArgs(ls, 1);

	ASSERT(inventory);
	string id = lua_tostring(ls, 1);
	
	return inventory->LuaWriteItemTable(ls, id);
}

//.TakeItem("id", amount) Takes up to amount number of "id" from our inventory. id can't be blank.
int player_TakeItem(lua_State* ls)
{
	luaCountArgs(ls, 2);

	ASSERT(inventory);

	inventory->Erase(lua_tostring(ls, 1), (uShort)lua_tonumber(ls, 2), true);

	return 0;
}

// .GetCash() - Returns total dorra 
int player_GetCash(lua_State* ls)
{
	ASSERT(inventory);
	
	lua_pushnumber(ls, inventory->GetCash());
	return 1;
}

// .AddCash(amount) - Adds (or subtracts if the amount is negative) to our dorra total
int player_AddCash(lua_State* ls)
{
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

// .GetProp("property") returns a cptr, number, or string based on the property we're retrieving
int player_GetProp(lua_State* ls) 
{
	PRINT("player_GetProp");
	luaCountArgs(ls, 1);

	ASSERT(game->mPlayer);
	
	string prop = lowercase(lua_tostring(ls, 1));
	int result = game->mPlayer->LuaGetProp(ls, prop);

	if (!result)
		console->AddMessage("Player.GetProp() '" + prop + "' Unknown");
	
	return result;
}

// .SetProp("property", value) Sets the property to the specified value. Value can be num, string, ptr, depends on the property.
int player_SetProp(lua_State* ls)
{
	PRINT("player_SetProp");
	luaCountArgs(ls, 2);

	ASSERT(game->mPlayer);
	
	string prop = lowercase(lua_tostring(ls, 1));
	int result = game->mPlayer->LuaSetProp(ls, prop, 2);

	if (!result)
		console->AddMessage("Player.SetProp() '" + prop + "' Unknown");

	return 0;
}

//	.EarnAchievement(title, desc<can only be set once>, max<can only be set once>)
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

	game->EarnAchievement(title, desc, max);
	
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

	lua_pushnil(ls);
	return 1;
}

//	.RequestDuelAction(pEntity, iTimeoutInSeconds)
int player_RequestDuelAction(lua_State* ls)
{
	luaCountArgs(ls, 2);
	
	Actor* a = NULL;
	if (!lua_isnil(ls, 1))
		a = (Actor*)lua_touserdata(ls, 1);

	int timeout = (int)lua_tonumber(ls, 2);

	PlayerActionMenu* m = new PlayerActionMenu(timeout, a);
	game->mMap->Add(m);
	
	return 0;
}

//	pLunem = Player.GetPartySlot(iSlot)
//		Returns lunem in the slot, or nil if none or slot number is invalid
int player_GetPartySlot(lua_State* ls)
{
	luaCountArgs(ls, 1);
	int slot = (int)lua_tonumber(ls, 1);
	
	Entity* e;
	if (slot == -1)
		e = game->mPlayer;
	else
		e = game->mParty->GetSlot( slot );
	
	if (e)
		lua_pushlightuserdata(ls, e);
	else
		lua_pushnil(ls);

	return 1;
}

//	.AddLunemToParty(pLunem) - Returns false if the party is full or the lunem is invalid, true otherwise
int player_AddLunemToParty(lua_State* ls)
{
	luaCountArgs(ls, 1);
	Entity* e = (Entity*)lua_touserdata(ls, 1);
	
	lua_pushboolean( ls, (e && e->mType == ENTITY_LUNEM && game->mParty->AddLunem((Lunem*)e)) );
	
	return 1;
}

//	.ClearPartySlot(iSlot) - returns false if the slot was already empty or invalid, true otherwise.
//		Will delete the Lunem from that slot. If the lunem exists on the map too, it will remain 
//		on the map. Otherwise, the class is fully deleted
int player_ClearPartySlot(lua_State* ls)
{
	luaCountArgs(ls, 1);
	
	int slot = (int)lua_tonumber(ls, 1);
	lua_pushboolean( ls, game->mParty->ClearSlot(slot) );
	
	return 1;
}

//	bool = .IsPartyFull()
int player_IsPartyFull(lua_State* ls)
{
	lua_pushboolean( ls, game->mParty->IsFull() );
	return 1;
}

int player_IsPartyEmpty(lua_State* ls)
{
	
}

int player_HasUsablePartyMember(lua_State* ls)
{
	lua_pushboolean( ls, game->mParty->HasLivingMember() );
	return 1;
}	

/*	.UseItemOnPartyMember("id")
		Will open the party dialog for the player to select a target party member. 
		Once a member is selected, will send out ITEM_USE with an additional table item: slot
*/				
int player_UseItemOnPartyMember(lua_State* ls)
{
	luaCountArgs(ls, 1);
	
	string id = lua_tostring(ls, 1);
	
	game->mParty->SetVisible(true);
	game->mParty->SetUseItemMode(id);
	game->mParty->MoveToTop();
	
	return 0;
}

int player_EndDuelTurn(lua_State* ls)
{	
	game->EndPlayersDuelTurn();
	return 0;	
}

static const luaL_Reg functions[] = {
	{"GetActor", player_GetActor},
	{"IsInChatMode", player_IsInChatMode},
	{"Warp", player_Warp},
	{"GiveItem", player_GiveItem},
	{"GetItem", player_GetItem},
	{"TakeItem", player_TakeItem},
	{"GetCash", player_GetCash},
	{"AddCash", player_AddCash},
	{"GetProp", player_GetProp},
	{"SetProp", player_SetProp},
	{"EarnAchievement", player_EarnAchievement},
	{"GetAchievement", player_GetAchievement},
	{"RequestDuelAction", player_RequestDuelAction},
	{"GetPartySlot", player_GetPartySlot},
	{"AddLunemToParty", player_AddLunemToParty},
	{"ClearPartySlot", player_ClearPartySlot},
	{"IsPartyFull", player_IsPartyFull},
	{"UseItemOnPartyMember", player_UseItemOnPartyMember},
	{"EndDuelTurn", player_EndDuelTurn},
	{"HasUsablePartyMember", player_HasUsablePartyMember},
	{NULL, NULL}
};

void RegisterPlayerLib(lua_State* ls)
{
	luaL_register( ls, "Player", functions );
}
