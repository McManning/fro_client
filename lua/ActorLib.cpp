
#include <lua.hpp>
#include "ActorLib.h"
#include "EntityLib.h"
#include "LuaCommon.h"
#include "../entity/Actor.h"
#include "../entity/Avatar.h"
#include "../entity/LocalActor.h"
#include "../game/GameManager.h"
#include "../map/Map.h"

// Returns true if the entity is a valid actor. (TODO: Make sure it's on the map) 
bool _verifyActor(Entity* e)
{
	return (e != NULL && e->mType >= ENTITY_ACTOR && e->mType < ENTITY_END_ACTORS);
}

// Referenced actor is always the first parameter of the state
// This should always return a valid entity pointer. If it's invalid, there will be a longjmp from within lua.
Actor* getReferencedActor(lua_State* ls, int index)
{
	Actor* a = (Actor*)(lua_touserdata(ls, 1));
	if (!_verifyActor(a))
	{
		string err = "index " + its(index) + " not a valid actor pointer.";
		lua_pushstring( ls, err.c_str() );
		lua_error( ls );
	}
	
	return a;
}

/*************************************************************************************
*	ACTOR AND ACTOR-INHERITED ENTITIES
/************************************************************************************/

// .IsIdle(actor)
int actor_IsIdle(lua_State* ls)
{
	PRINT("actor_IsIdle");
	luaCountArgs(ls, 1);

	Actor* a = getReferencedActor(ls);
	
	lua_pushnumber( ls, !a->IsMoving() );
	return 1;
}

// .IsJumping(actor)
int actor_IsJumping(lua_State* ls)
{
	PRINT("actor_IsJumping");
	luaCountArgs(ls, 1);

	Actor* a = getReferencedActor(ls);
	
	lua_pushnumber( ls, !a->IsJumping() );
	return 1;
}

// .Emote(actor, type)	
int actor_Emote(lua_State* ls)
{
	PRINT("actor_Emote");
	luaCountArgs(ls, 2);

	Actor* a = getReferencedActor(ls);
	
	a->Emote( (uShort)lua_tonumber(ls, 2) );
	return 0;
}

// .Jump(actor, type) - Type being: 0: Standing Jump, 1: Walking Jump, 2: Running Jump
int actor_Jump(lua_State* ls)
{
	PRINT("actor_Jump");
	luaCountArgs(ls, 2);

	Actor* a = getReferencedActor(ls);
	
	a->Jump( (byte)lua_tonumber(ls, 2) );
	return 0;
}

// .Fall(actor, height, velocity)
int actor_Fall(lua_State* ls)
{
	luaCountArgs(ls, 3);
	Actor* a = getReferencedActor(ls);
	
	a->Fall( (int)lua_tonumber(ls, 2), (int)lua_tonumber(ls, 3) );
	return 0;
}

// .Rise(actor, velocity)
int actor_Rise(lua_State* ls)
{
	luaCountArgs(ls, 2);
	Actor* a = getReferencedActor(ls);
	
	a->Rise( (int)lua_tonumber(ls, 2) );
	return 0;
}

// x, y = .GetDestination(actor)
int actor_GetDestination(lua_State* ls) 
{
	PRINT("actor_GetDestination");
	luaCountArgs(ls, 1);

	Actor* a = getReferencedActor(ls);
	
	point2d p = a->GetDestination();
	lua_pushnumber(ls, p.x);
	lua_pushnumber(ls, p.y);
	
	return 2;	
}

// .CanMove(actor, direction, distance) - Direction corrosponding with a keypad number.
int actor_CanMove(lua_State* ls)
{
	PRINT("actor_CanMove");
	luaCountArgs(ls, 3);

	Actor* a = getReferencedActor(ls);
	
	bool canMove = a->CanMove( stringToDirection(lua_tostring(ls, 2)), 
												(sShort)lua_tonumber(ls, 3)
							);
	
	lua_pushboolean( ls, canMove );
	return 1;
}

// .MoveTo(actor, x, y, speed) - Will try to move to x, y. (Will use pathfinding, and return 1 if it's possible, when implemented)
int actor_MoveTo(lua_State* ls)
{
	PRINT("actor_MoveTo");
	luaCountArgs(ls, 3);

	int numArgs = lua_gettop( ls );
	
	Actor* a = getReferencedActor(ls);
	
	byte speed = 0;
	if (numArgs > 3)
		speed = (byte)lua_tonumber(ls, 4);
	
	point2d p((sShort)lua_tonumber(ls, 2), (sShort)lua_tonumber(ls, 3));
	a->MoveTo(p, speed);
	
	return 0;
}

// .Move(actor, direction, distance, speed)
int actor_Move(lua_State* ls)
{
	PRINT("actor_Move");
	luaCountArgs(ls, 4);

	Actor* a = getReferencedActor(ls);
	
	a->Move( stringToDirection(lua_tostring(ls, 2)), (sShort)lua_tonumber(ls, 3), (byte)lua_tonumber(ls, 4) );
	
	return 0;
}

// .AddToBuffer(actor, "bufferdata")
int actor_AddToBuffer(lua_State* ls)
{
	PRINT("actor_AddToBuffer");
	luaCountArgs(ls, 2);

	Actor* a = getReferencedActor(ls);
	
	a->AddToActionBuffer( lua_tostring(ls, 2) );
	
	return 0;
}

// .LoadAvatar(actor, "file", "pass", w, h, delay, flags) Returns 1 on success, 0 otherwise.
int actor_LoadAvatar(lua_State* ls)
{
	PRINT("actor_LoadAvatar");
	luaCountArgs(ls, 8);

	Actor* a = getReferencedActor(ls);

	bool result = a->LoadAvatar( game->mMap->mWorkingDir + lua_tostring(ls, 2), lua_tostring(ls, 3),
							(uShort)lua_tonumber(ls, 4), (uShort)lua_tonumber(ls, 5),
							(uShort)lua_tonumber(ls, 6), (uShort)lua_tonumber(ls, 7)
						);
						
	lua_pushboolean(ls, result);
	return 1;
}

//	.Face(actor1, entity) - Change actor1's direction to face entity
int actor_Face(lua_State* ls)
{
	PRINT("actor_Face");
	luaCountArgs(ls, 2);
	
	Actor* a = getReferencedActor(ls);

	Entity* e = (Entity*)lua_touserdata(ls, 2);
	
	if (!_verifyEntity(e))
		return 0;
	
	a->Face(e);
	return 0;
}
/*
// .GainExp(actor, number)
int actor_GainExp(lua_State* ls)
{
	Actor* a = getReferencedActor(ls);
	a->AddExperience((int)lua_tonumber(ls, 2));
	return 0;
}

// .TakeDamage(actor, attacker, damage) - If attacker is nil, can be considered world damage.
//	Otherwise, attacker is a pointer to another actor
int actor_TakeDamage(lua_State* ls)
{
	Actor* defender = getReferencedActor(ls, 1);
	Actor* attacker;
	if (lua_isnil(ls, 2))
		attacker = NULL;
	else
		attacker = getReferencedActor(ls, 2);

	defender->TakeDamage(attacker, (int)lua_tonumber(ls, 3));
	return 0;
}

//	"str" = .GetSkill(actor, slot) - Returns skill ID string,
//		or nil if there isn't a skill in the slot, or the slot is invalid
int actor_GetSkill(lua_State* ls)
{
	if (!lua_isnumber(ls, 2))
		return luaError(ls, "Actor.GetSkill", "Bad Params");

	Actor* a = getReferencedActor(ls, 1);
	int slot = (int)lua_tonumber(ls, 2);
	if (slot >= 0 && slot < 5 && !a->m_sSkills[slot].id.empty())
	{
		lua_pushstring(ls, a->m_sSkills[slot].id.c_str());
	}
	else
	{
		lua_pushnil(ls);	
	}
	
	return 1;	
}

//	.SetSkill(actor, slot, "id") - Sets the slot to the specified skill ID
int actor_SetSkill(lua_State* ls)
{
	if (!lua_isnumber(ls, 2) || !lua_isstring(ls, 3))
		return luaError(ls, "Actor.SetSkill", "Bad Params");

	Actor* a = getReferencedActor(ls, 1);
	int slot = (int)lua_tonumber(ls, 2);
	string s = lua_tostring(ls, 3);
	
	if (slot >= 0 && slot < 5)
	{
		a->m_sSkills[slot].id = s;
	}
	
	return 0;	
}
*/
static const luaL_Reg functions[] = {
	{"IsIdle", actor_IsIdle},
	{"IsJumping", actor_IsJumping},
	{"Emote", actor_Emote},
	{"Jump", actor_Jump},
	{"Fall", actor_Fall},
	{"Rise", actor_Rise},
	{"GetDestination", actor_GetDestination},
	{"CanMove", actor_CanMove},
	{"MoveTo", actor_MoveTo},
	{"Move", actor_Move},
	{"AddToBuffer", actor_AddToBuffer},
	{"LoadAvatar", actor_LoadAvatar},
	{"Face", actor_Face},
/*	{"GainExp", actor_GainExp},
	{"TakeDamage", actor_TakeDamage},
	{"GetSkill", actor_GetSkill},
	{"SetSkill", actor_SetSkill},*/
	{NULL, NULL}
};

void RegisterActorLib(lua_State* ls)
{
	luaL_register( ls, "Actor", functions );
}
