
#include <lua.hpp>
#include "EntityLib.h"
#include "LuaCommon.h"
#include "../entity/Actor.h"
#include "../entity/Avatar.h"
#include "../entity/SceneActor.h"
#include "../entity/LocalActor.h"
#include "../game/GameManager.h"
#include "../map/Map.h"

// Returns true if the entity is a valid actor. (TODO: Make sure it's on the map) 
bool _verifyActor(Entity* e)
{
	return (e != NULL && e->mType > ENTITY_ACTOR && e->mType < ENTITY_END_ACTORS);
}

// Referenced actor is always the first parameter of the state
// This should always return a valid entity pointer. If it's invalid, there will be a longjmp from within lua.
Actor* _getReferencedActor(lua_State* ls, int index = 1)
{
	Actor* a = (Actor*)lua_touserdata(ls, 1);
	if (!_verifyActor(a))
	{
		string err = "index " + its(index) + " not a valid entity pointer.";
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

	Actor* a = _getReferencedActor(ls);
	
	lua_pushnumber( ls, !a->IsMoving() );
	return 1;
}

// .IsJumping(actor)
int actor_IsJumping(lua_State* ls)
{
	PRINT("actor_IsJumping");
	luaCountArgs(ls, 1);

	Actor* a = _getReferencedActor(ls);
	
	lua_pushnumber( ls, !a->IsJumping() );
	return 1;
}

// .Emote(actor, type)	
int actor_Emote(lua_State* ls)
{
	PRINT("actor_Emote");
	luaCountArgs(ls, 2);

	Actor* a = _getReferencedActor(ls);
	
	a->Emote( (uShort)lua_tonumber(ls, 2) );
	return 0;
}

// .Jump(actor, type) - Type being: 0: Standing Jump, 1: Walking Jump, 2: Running Jump
int actor_Jump(lua_State* ls)
{
	PRINT("actor_Jump");
	luaCountArgs(ls, 2);

	Actor* a = _getReferencedActor(ls);
	
	a->Jump( (byte)lua_tonumber(ls, 2) );
	return 0;
}

// .GetDestination(actor) Returns x, y. (Use as:  x, y = Entity.GetDestination(myActor); )
int actor_GetDestination(lua_State* ls) 
{
	PRINT("actor_GetDestination");
	luaCountArgs(ls, 1);

	Actor* a = _getReferencedActor(ls);
	
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

	Actor* a = _getReferencedActor(ls);
	
	lua_pushnumber( ls, a->CanMove( stringToDirection(lua_tostring(ls, 2)), (sShort)lua_tonumber(ls, 3)) );
	return 1;
}

// .MoveTo(actor, x, y, speed) - Will try to move to x, y. (Will use pathfinding, and return 1 if it's possible, when implemented)
int actor_MoveTo(lua_State* ls)
{
	PRINT("actor_MoveTo");
	luaCountArgs(ls, 3);

	int numArgs = lua_gettop( ls );
	
	Actor* a = _getReferencedActor(ls);
	
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

	Actor* a = _getReferencedActor(ls);
	
	a->Move( stringToDirection(lua_tostring(ls, 2)), (sShort)lua_tonumber(ls, 3), (byte)lua_tonumber(ls, 4) );
	
	return 0;
}

// .AddToBuffer(actor, "bufferdata")
int actor_AddToBuffer(lua_State* ls)
{
	PRINT("actor_AddToBuffer");
	luaCountArgs(ls, 2);

	Actor* a = _getReferencedActor(ls);
	
	a->AddToActionBuffer( lua_tostring(ls, 2) );
	
	return 0;
}

// .LoadAvatar(actor, "url", "pass", w, h, delay, loopstand, loopsit) Returns 1 on success, 0 otherwise.
int actor_LoadAvatar(lua_State* ls)
{
	PRINT("actor_LoadAvatar");
	luaCountArgs(ls, 8);

	Actor* a = _getReferencedActor(ls);
	
	int result;
	result = a->LoadAvatar( lua_tostring(ls, 2), lua_tostring(ls, 3),
							(uShort)lua_tonumber(ls, 4), (uShort)lua_tonumber(ls, 5),
							(uShort)lua_tonumber(ls, 6), lua_tonumber(ls, 7),
							lua_tonumber(ls, 8)
						);
						
	lua_pushnumber(ls, result);
	return 1;
}

// .GetProp(actor, "property") returns a cptr, number, or string based on the property we're retrieving
int actor_GetProp(lua_State* ls) 
{
	PRINT("actor_GetProp");
	luaCountArgs(ls, 2);
	
	Actor* a = _getReferencedActor(ls);

	string prop = lua_tostring(ls, 2);

	if (prop == "direction") lua_pushnumber( ls, a->GetDirection() );
	else if (prop == "speed") lua_pushnumber( ls, a->GetSpeed() );
	else if (prop == "action") lua_pushnumber( ls, a->GetAction() );
	else if (prop == "noclip") lua_pushnumber( ls, a->IgnoreSolids() );
	else if (prop == "canattack") lua_pushnumber( ls, a->CanAttack() );
	else if (prop == "attacking") lua_pushnumber( ls, a->IsAttacking() );
	else if (prop == "mod" && a->GetAvatar()) lua_pushnumber( ls, a->GetAvatar()->mModifier );
	else return luaError(ls, "Actor.GetProp", prop + " unknown");

	return 1;
}

// .SetProp(actor, "property", value) Sets the property to the specified value. Value can be num, string, ptr, depends on the property.
int actor_SetProp(lua_State* ls) 
{
	PRINT("actor_SetProp");
	luaCountArgs(ls, 3);

	Actor* a = _getReferencedActor(ls);

	string prop = lua_tostring(ls, 2);

	if (prop == "direction") a->SetDirection( stringToDirection(lua_tostring(ls, 3)) );
	else if (prop == "speed") a->SetSpeed( (byte)lua_tonumber(ls, 3) );
	else if (prop == "action") a->SetAction( (byte)lua_tonumber(ls, 3) );
	else if (prop == "noclip") a->SetIgnoreSolids( lua_tonumber(ls, 3) );
	else if (prop == "mod" && a->GetAvatar())
	{
		if ( a->GetAvatar()->Modify( (byte)lua_tonumber(ls, 3) ) )
		{
			//if we modified our local players avatar, we need to send this mod to the network
			if (a == (Actor*)game->mPlayer)
				game->mPlayer->NetSendAvatarMod();
		}
	}
	else return luaError(ls, "Actor.SetProp", prop + " unknown");

	return 0;
}

// .SkipAttack(actor) - Will skip attack animation and cooldown when used in a listener on ENTITY_ATTACK. 
//		This function has no effect outside an ENTITY_ATTACK listener.
int actor_SkipAttack(lua_State* ls)
{
	PRINT("actor_SkipAttack");
	luaCountArgs(ls, 1);

	Actor* a = _getReferencedActor(ls);
	a->SkipAttack();
	
	return 0;
}

// .Attack(actor, cooldown<DEFAULT_ATTACK_COOLDOWN>) - Force the actor to attack
int actor_Attack(lua_State* ls)
{
	PRINT("actor_Attack");
	luaCountArgs(ls, 1);
	int numArgs = lua_gettop(ls);
	
	int cooldown = DEFAULT_ATTACK_COOLDOWN;
	if (numArgs > 1)
		cooldown = (int)lua_tonumber(ls, 2);

	Actor* a = _getReferencedActor(ls);
	a->Attack(cooldown);
	
	return 0;
}

// .NewSceneActor(layer<1>) - Add a new blank SceneActor to the map. You need to manually set all its properties afterwards. 
//	(Avatar, position, direction, name, id, etc). Returns the new actor
int actor_NewSceneActor(lua_State* ls)
{
	PRINT("actor_NewSceneActor");
	luaCountArgs(ls, 1);
	
	byte layer = (byte)lua_tonumber(ls, 1);
	
	SceneActor* a = new SceneActor();
	a->mMap = game->mMap;
	a->mMap->AddEntity( a, layer );
	
	lua_pushlightuserdata(ls, a);
	return 1;
}

//	.Face(actor1, entity) - Change actor1's direction to face entity
int actor_Face(lua_State* ls)
{
	PRINT("actor_Face");
	luaCountArgs(ls, 2);
	
	Actor* a = _getReferencedActor(ls);

	Entity* e = (Entity*)lua_touserdata(ls, 2);
	
	if (!_verifyEntity(e))
		return 0;
	
	a->Face(e);
	return 0;
}

static const luaL_Reg functions[] = {
	{"IsIdle", actor_IsIdle},
	{"IsJumping", actor_IsJumping},
	{"Emote", actor_Emote},
	{"Jump", actor_Jump},
	{"GetDestination", actor_GetDestination},
	{"CanMove", actor_CanMove},
	{"MoveTo", actor_MoveTo},
	{"Move", actor_Move},
	{"AddToBuffer", actor_AddToBuffer},
	{"LoadAvatar", actor_LoadAvatar},
	{"GetProp", actor_GetProp},
	{"SetProp", actor_SetProp},
	{"SkipAttack", actor_SkipAttack},
	{"Attack", actor_Attack},
	{"NewSceneActor", actor_NewSceneActor},
	{"Face", actor_Face},
	{NULL, NULL}
};

void RegisterActorLib(lua_State* ls)
{
	luaL_register( ls, "Actor", functions );
}
