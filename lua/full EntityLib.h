
#ifndef _LUA_ENTITYLIB_H_
#define _LUA_ENTITYLIB_H_

class lua_State;
void RegisterEntityLib(lua_State*);

#endif //_LUA_ENTITYLIB_H_

#include <lua.hpp>
#include "EntityLib.h"

/*
	TODO:
		-No game->mMap reference? Or at least, make it better!
		
FUNCTIONS TODO:

Table FindAllMatchingName(String name) - Returns a table with keys 1->tablesize filled with C_Pointer's of entities with matching name.
Table FindAllMatchingId(String id) - Returns a table with keys 1->tablesize filled with C_Pointer's of entities with matching ID.

Num Warp(C_Pointer entity, Num x, Num y, Num IgnoreSolids) - Will warp to (x, y) of the currently loaded map. Returns 1 on success, 0 if it tries to warp the entity into a blocked object and IgnoreSolids is 0.
Num Move(cptr entity, Num dir, Num distance, Num speed) - Move this entity in the particular direction. Returns 1 if it took at least 1 step, 0 if it couldn't move at all.
Num IsIdle(entity) - Returns 0 if the entity is still moving, 1 otherwise.

*/

static const luaL_Reg functions[] = {
	{"FindById", entity_FindById},
	{"FindByName", entity_FindByName},
	{"GetFlag", entity_GetFlag},
	{"SetFlag", entity_SetFlag},
	{"GetPosition", entity_GetPosition},
	{"SetPosition", entity_SetPosition},
	{"GetRect", entity_GetRect},
	{"GetProp", entity_GetProp},
	{"SetProp", entity_SetProp},
	{"IsTouching", entity_IsTouching},
	{"Say", entity_Say},
	{"Clone", entity_Clone},
	{NULL, NULL}
};

void RegisterEntityLib(lua_State* ls)
{
	luaL_register( ls, "Entity", functions );
}

// Returns true if the entity is a valid actor. (TODO: Make sure it's on the map) 
bool _verifyActor(Entity* e)
{
	return (e != NULL && e->mType > ENTITY_ACTOR && a->mType < ENTITY_END_ACTORS);
}

// Referenced actor is always the first parameter of the state
Actor* _getReferencedActor(lua_State* ls)
{
	Actor* a = (Actor*)lua_touserdata(ls, 1);
	if (!_verifyActor(a))
	{
		lua_pushstring( ls, "First parameter not a valid actor pointer." );
		lua_error( ls );
		return NULL;
	}
	
	return a;
}

// Returns true if the entity is valid. (TODO: Make sure it's on the map) 
bool _verifyEntity(Entity* e)
{
	return (e != NULL);
}

// Referenced entity is always the first parameter of the state
Entity* _getReferencedEntity(lua_State* ls)
{
	Entity* e = (Entity*)lua_touserdata(ls, 1);
	if (!_verifyEntity(e))
	{
		lua_pushstring( ls, "First parameter not a valid entity pointer." );
		lua_error( ls );
		return NULL;
	}
	
	return e;
}

// If we don't have the desired amount of arguments, return an error 
bool luaCountArgs(lua_State* ls, int desired)
{
	int numArgs = lua_gettop( ls );
	if (numArgs < desired)
	{
		lua_pushstring( ls, "[_checkArgs] Invalid Parameters" );
		lua_error( ls ); //This function does a longjmp and never returns.
		return false;
	}
	return true;
}

// .FindById("id") returns cptr to entity, 0 if it doesn't exist.
int entity_FindById(lua_State* ls) 
{
	PRINT("entity_FindById");
	if (!luaCountArgs(ls, 1)) return 0;

	Entity* e = game->mMap->FindEntityById( lua_tostring(ls, 1) );
	if (!e) return 0;
	
	lua_pushlightuserdata(ls, e);
	return 1;
}

// .FindByName("name") returns cptr to entity, 0 if it doesn't exist.
int entity_FindByName(lua_State* ls) 
{
	PRINT("entity_FindByName");
	if (!luaCountArgs(ls, 1)) return 0;

	Entity* e = game->mMap->FindEntityByName( lua_tostring(ls, 1) );
	if (!e) return 0;
	
	lua_pushlightuserdata(ls, e);
	return 1;
}

// .GetFlag(entity, "key") Returns string of value. Empty string if key does not exist.
int entity_GetFlag(lua_State* ls) 
{
	PRINT("entity_GetFlag");
	if (!luaCountArgs(ls, 2)) return 0;

	Entity* e = _getReferencedEntity(ls);
	if (!e) return 0;

	string s = e->GetFlag( lua_tostring(ls, 2) );
	lua_pushstring(ls, s.c_str());
	
	return 1;
}

// .SetFlag(entity, "key", "value")
int entity_SetFlag(lua_State* ls) 
{
	PRINT("entity_SetFlag");
	if (!luaCountArgs(ls, 3)) return 0;

	Entity* e = _getReferencedEntity(ls);
	if (!e) return 0;

	e->SetFlag(lua_tostring(ls, 2), lua_tostring(ls, 3));

	return 0;	
}

// .GetPosition(entity)  Returns 2 values, x and y
int entity_GetPosition(lua_State* ls) 
{
	PRINT("entity_GetPosition");
	if (!luaCountArgs(ls, 1)) return 0;

	Entity* e = _getReferencedEntity(ls);
	if (!e) return 0;

	point2d p = e->GetPosition();
	lua_pushnumber(ls, p.x);
	lua_pushnumber(ls, p.y);
	
	return 2;	
}

// .SetPosition(entity, x, y)
int entity_SetPosition(lua_State* ls) 
{
	PRINT("entity_SetPosition");
	if (!luaCountArgs(ls, 3)) return 0;

	Entity* e = _getReferencedEntity(ls);
	if (!e) return 0;

	point2d p(lua_tonumber(ls, 2), lua_tonumber(ls, 3));
	e->SetPosition(p);

	return 0;	
}

// .GetRect(entity)  Returns x, y, w, h. Where x, y are coordinates of the top left our our avatar on the map. w, h are avatar dimensions.
int entity_GetRect(lua_State* ls) 
{
	PRINT("entity_GetRect");
	if (!luaCountArgs(ls, 1)) return 0;

	Entity* e = _getReferencedEntity(ls);
	if (!e) return 0;

	rect r = e->GetBoundingRect();
	lua_pushnumber(ls, r.x);
	lua_pushnumber(ls, r.y);
	lua_pushnumber(ls, r.w);
	lua_pushnumber(ls, r.h);
	
	return 4;	
}

// .GetProp(entity, "property") returns a cptr, number, or string based on the property we're retrieving
int entity_GetProp(lua_State* ls) 
{
	PRINT("entity_GetProp");
	if (!luaCountArgs(ls, 2)) return 0;

	Entity* e = _getReferencedEntity(ls);
	if (!e) return 0;
	
	string prop = lua_tostring(ls, 2);
	
	//TODO: Improve this somehow! (How did Valve do it?)
	if (prop == "id") lua_pushstring( ls, e->mId.c_str() );
	else if (prop == "name") lua_pushstring( ls, e->mName.c_str() );
	else if (prop == "visible") lua_pushnumber( ls, e->IsVisible() );
	else if (prop == "solid") lua_pushstring( ls, e->IsSolid() );
	else if (prop == "nocollide") lua_pushnumber( ls, e->GetCollide() );
	else if (_verifyActor(e)) //Additional actor-specific properties
	{
		Actor* a = e;
		//actor-specific properties
		if (prop == "direction") lua_pushnumber( ls, a->GetDirection() );
		else if (prop == "speed") lua_pushnumber( ls, a->GetSpeed() );
		else if (prop == "action") lua_pushnumber( ls, a->GetAction() );
		else return 0;
	}
	else return 0;

	return 1;
}

// .SetProp(entity, "property", value) Sets the property to the specified value. Value can be num, string, ptr, depends on the property.
int entity_SetProp(lua_State* ls) 
{
	PRINT("entity_SetProp");
	if (!luaCountArgs(ls, 3)) return 0;

	Entity* e = _getReferencedEntity(ls);
	if (!e) return 0;
	
	string prop = lua_tostring(ls, 2);
	
	//TODO: Improve this somehow! (How did Valve do it?)
	if (prop == "id") e->mId = lua_tostring(ls, 3);
	else if (prop == "name") e->mName = lua_tostring(ls, 3);
	else if (prop == "visible") e->SetVisible( lua_tonumber(ls, 3) );
	else if (prop == "solid") e->SetSolid( lua_tonumber(ls, 3) );
	else if (prop == "shadow") e->mShadow = lua_tonumber(ls, 3);
	else if (prop == "nocollide") e->SetCollide( lua_tonumber(ls, 3) );
	else if (_verifyActor(e)) //additional actor specific properties
	{
		Actor* a = e;
		//actor-specific properties
		if (prop == "direction") a->SetDirection( lua_tonumber(ls, 3) );
		else if (prop == "speed") a->SetSpeed( lua_tonumber(ls, 3) );
		else if (prop == "action") a->SetAction( lua_tonumber(ls, 3) );
	}

	return 0;
}

// .IsTouching(ent, ent) - Returns 1 if the two entities are intersecting collision rects, 0 otherwise. 
//Also note, if the second ent is a bad pointer, it'll return 0 also.
int entity_IsTouching(lua_State* ls) 
{
	PRINT("entity_IsTouching");
	if (!luaCountArgs(ls, 2)) return 0;

	Entity* e = _getReferencedEntity(ls);
	if (!e) return 0;

	Entity* e2 = (Entity*)lua_touserdata(ls, 2);
	
	//Return 0 if e2 is invalid, or they're not touching
	if ( !_verifyEntity(e2) || !e->IsCollidingWithEntity(e2) )
		lua_pushnumber(ls, 0);
	else
		lua_pushnumber(ls, 1);

	return 1;
}

// .Say(entity, "msg", showbubble, showinchat) - Say the message. Last two parameters are optional, and default to 1.
int entity_Say(lua_State* ls)
{
	PRINT("entity_Say");
	if (!luaCountArgs(ls, 2)) return 0;

	Entity* e = _getReferencedEntity(ls);
	if (!e) return 0;

	int numArgs = lua_gettop(ls);
	
	string msg = lua_tostring(ls, 2);
	bool showbubble = (numArgs > 2) ? lua_tonumber(ls, 3) : 1;
	bool showinchat = (numArgs > 3) ? lua_tonumber(ls, 4) : 1;

	if (showbubble)
		game->mMap->mBubbles.CreateBubble(e, msg);
	
	if (showinchat)
		game->mChat->AddMessage(e->mName + ": " + msg);

	return 0;	
}

// ent = .Clone(entity) - Create a replica of the entity, place it in the same position and status on the map, and return a cptr to the clone.
int entity_Clone(lua_State* ls)
{
	PRINT("entity_Clone");
	if (!luaCountArgs(ls, 1)) return 0;
	
	Entity* e = _getReferencedEntity(ls);
	if (!e) return 0;
	
	string s;
	
	Entity* e2 = e->Clone();
	if (!e2)
	{
		s = "[entity_Clone] Failed to clone: " + e->mId;
		lua_pushstring( ls, s.c_str() );
		lua_error( ls );
		return 0;
	}

	//It's necessary to manually add it the map after a successful cloning
	e2->mMap->AddEntity( e2, e2->GetLayer() );
	
	lua_pushlightuserdata(ls, e2);
	return 1;
}

/*************************************************************************************
*	ACTOR AND ACTOR-INHERITED ENTITIES
/************************************************************************************/

// .IsIdle(actor)
int actor_IsIdle(lua_State* ls)
{
	PRINT("actor_IsIdle");
	if (!luaCountArgs(ls, 1)) return 0;

	Actor* a = _getReferencedActor(ls);
	if (!a) return 0;

	lua_pushnumber( ls, !a->IsMoving() );
	return 1;
}

// .IsJumping(actor)
int actor_IsJumping(lua_State* ls)
{
	PRINT("actor_IsJumping");
	if (!luaCountArgs(ls, 1)) return 0;

	Actor* a = _getReferencedActor(ls);
	if (!a) return 0;

	lua_pushnumber( ls, !a->IsJumping() );
	return 1;
}

// .Emote(actor, type)	
int actor_Emote(lua_State* ls)
{
	PRINT("actor_Emote");
	if (!luaCountArgs(ls, 2)) return 0;

	Actor* a = _getReferencedActor(ls);
	if (!a) return 0;

	a->Emote( lua_tonumber(ls, 2) );
	return 0;
}

// .Jump(actor, type) - Type being: 0: Standing Jump, 1: Walking Jump, 2: Running Jump
int actor_Jump(lua_State* ls)
{
	PRINT("actor_Jump");
	if (!luaCountArgs(ls, 2)) return 0;

	Actor* a = _getReferencedActor(ls);
	if (!a) return 0;

	a->Jump( lua_tonumber(ls, 2) );
	return 0;
}

// .GetDestination(actor) Returns x, y. (Use as:  x, y = Entity.GetDestination(myActor); )
int actor_GetDestination(lua_State* ls) 
{
	PRINT("actor_GetDestination");
	if (!luaCountArgs(ls, 1)) return 0;

	Actor* a = _getReferencedActor(ls);
	if (!a) return 0;

	point2d p = a->GetDestination();
	lua_pushnumber(ls, p.x);
	lua_pushnumber(ls, p.y);
	
	return 2;	
}

// .CanMove(actor, direction, distance) - Direction corrosponding with a keypad number.
int actor_CanMove(lua_State* ls)
{
	PRINT("actor_CanMove");
	if (!luaCountArgs(ls, 3)) return 0;

	Actor* a = _getReferencedActor(ls);
	if (!a) return 0;

	lua_pushnumber( ls, a->CanMove(lua_tonumber(ls, 2), lua_tonumber(ls, 3)) );
	return 1;
}

// .MoveTo(actor, x, y, speed) - Will try to move to x, y. (Will use pathfinding, and return 1 if it's possible, when implemented)
int actor_MoveTo(lua_State* ls)
{
	PRINT("actor_MoveTo");
	if (!luaCountArgs(ls, 4)) return 0;

	Actor* a = _getReferencedActor(ls);
	if (!a) return 0;

	point2d p(lua_tonumber(ls, 2), lua_tonumber(ls, 3));
	a->MoveTo(p, lua_tonumber(ls, 4));
	
	return 0;
}

// .Move(actor, direction, distance, speed)
int actor_Move(lua_State* ls)
{
	PRINT("actor_Move");
	if (!luaCountArgs(ls, 4)) return 0;

	Actor* a = _getReferencedActor(ls);
	if (!a) return 0;

	a->Move( lua_tonumber(ls, 2), lua_tonumber(ls, 3), lua_tonumber(ls, 4) );
	
	return 0;
}

// .AddToBuffer(actor, "bufferdata")
int actor_AddToBuffer(lua_State* ls)
{
	PRINT("actor_AddToBuffer");
	if (!luaCountArgs(ls, 2)) return 0;

	Actor* a = _getReferencedActor(ls);
	if (!a) return 0;

	a->AddToActionBuffer( lua_tostring(ls, 2) );
	
	return 0;
}

// .LoadAvatar(actor, "url", "pass", w, h, delay, loopstand, loopsit) Returns 1 on success, 0 otherwise.
int actor_LoadAvatar(lua_State* ls)
{
	PRINT("actor_LoadAvatar");
	if (!luaCountArgs(ls, 8)) return 0;

	Actor* a = _getReferencedActor(ls);
	if (!a) return 0;

	int result;
	result = a->LoadAvatar( lua_tostring(ls, 2), lua_tostring(ls, 3),
							lua_tonumber(ls, 4), lua_tonumber(ls, 5),
							lua_tonumber(ls, 6), lua_tonumber(ls, 7),
							lua_tonumber(ls, 8)
						);
						
	lua_pushnumber(ls, result);
	return 1;
}

