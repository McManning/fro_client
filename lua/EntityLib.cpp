
#include <lua.hpp>
#include "EntityLib.h"
#include "LuaCommon.h"
#include "../entity/Entity.h"
#include "../entity/TextObject.h"
#include "../game/GameManager.h"
#include "../map/CollectionMap.h"

// Returns true if the entity is valid. (TODO: Make sure it's on the map) 
bool _verifyEntity(Entity* e)
{
	return (e != NULL);
}

// Referenced entity is always the first parameter of the state.
// This should always return a valid entity pointer. If it's invalid, there will be a longjmp from within lua.
Entity* _getReferencedEntity(lua_State* ls, int index = 1)
{
	Entity* e = (Entity*)lua_touserdata(ls, index);
	if (!_verifyEntity(e))
	{
		string err = "index " + its(index) + " not a valid entity pointer.";
		lua_pushstring( ls, err.c_str() );
		lua_error( ls );
	}
	
	return e;
}

// .Exists(entity) - Returns 1 if the entity pointer is valid and on the map, 0 otherwise. 
// This is preferred over the checking-for-valid-entity-every-time-it's-accessed approach due to it being MUCH faster.
int entity_Exists(lua_State* ls)
{
	PRINT("entity_Exists");
	luaCountArgs(ls, 1);
	
	Entity* e = (Entity*)lua_touserdata(ls, 1);
	
	int result = game->mMap->EntityExists(e);

	lua_pushnumber( ls, result );
	return 1;
}

// .FindById("id") returns cptr to entity, 0 if it doesn't exist.
int entity_FindById(lua_State* ls) 
{
	PRINT("entity_FindById");
	luaCountArgs(ls, 1);

	ASSERT(game->mMap);

	Entity* e = game->mMap->FindEntityById( lua_tostring(ls, 1) );
	if (!e) 
		return luaError(ls, "Entity.FindById", string(lua_tostring(ls, 1)) + " not found");
	
	lua_pushlightuserdata(ls, e);
	return 1;
}

// .FindAllById("id", 0) - returns a table with indexes 1->N containing all entities on the map with matching id.
// If the second parameter is supplied, it's a bool indicating whether to use wildcard matching or not
int entity_FindAllById(lua_State* ls)
{
	PRINT("entity_FindAllById");
	luaCountArgs(ls, 1);

	ASSERT(game->mMap);
	
	string id = lua_tostring(ls, 1);

	bool useWildmatch = false;
	int numArgs = lua_gettop(ls);
	if (numArgs > 1)
		useWildmatch = lua_tonumber(ls, 2);
	
	//Construct a table to store all entities
	lua_newtable(ls);
	int top = lua_gettop(ls);

	Entity* e;
	int index = 1; //Table keys will be index numbers, starting at 1 (common for lua arrays)
	for (int i = 0; i < ENTITYLEVEL_COUNT; ++i)
	{
		for (int ii = 0; ii < game->mMap->entityLevel[i].size(); ++ii)
		{
			e = game->mMap->entityLevel[i].at(ii);
			
			//if it matches the search, add to table
			if ( (useWildmatch && wildmatch(id.c_str(), e->mId.c_str())) 
				|| (!useWildmatch && e->mId == id) )
			{
				lua_pushnumber(ls, index);
				lua_pushlightuserdata(ls, e);
				lua_settable(ls, top);
				++index;
			}
		}
	}
	
	return 1;
}

// .FindByName("name") returns cptr to entity, 0 if it doesn't exist.
int entity_FindByName(lua_State* ls) 
{
	PRINT("entity_FindByName");
	luaCountArgs(ls, 1);

	ASSERT(game->mMap);

	Entity* e = game->mMap->FindEntityByName( lua_tostring(ls, 1) );
	if (!e) 
		return luaError(ls, "Entity.FindByName", string(lua_tostring(ls, 1)) + " not found");
	
	lua_pushlightuserdata(ls, e);
	return 1;
}

// .FindAllByName("name", 0) - returns a table with indexes 1->N containing all entities on the map with matching names.
// If the second parameter is supplied, it's a bool indicating whether to use wildcard matching or not
int entity_FindAllByName(lua_State* ls)
{
	PRINT("entity_FindAllByName");
	luaCountArgs(ls, 1);

	ASSERT(game->mMap);
	
	string name = lua_tostring(ls, 1);

	bool useWildmatch = false;
	int numArgs = lua_gettop(ls);
	if (numArgs > 1)
		useWildmatch = lua_tonumber(ls, 2);
	
	//Construct a table to store all entities
	lua_newtable(ls);
	int top = lua_gettop(ls);

	Entity* e;
	int index = 1; //Table keys will be index numbers, starting at 1 (common for lua arrays)
	for (int i = 0; i < ENTITYLEVEL_COUNT; ++i)
	{
		for (int ii = 0; ii < game->mMap->entityLevel[i].size(); ++ii)
		{
			e = game->mMap->entityLevel[i].at(ii);
			
			//if it matches the search, add to table
			if ( (useWildmatch && wildmatch(name.c_str(), e->mName.c_str())) 
				|| (!useWildmatch && e->mName == name) )
			{
				lua_pushnumber(ls, index);
				lua_pushlightuserdata(ls, e);
				lua_settable(ls, top);
				++index;
			}
		}
	}
	
	return 1;
}

// .GetPosition(entity)  Returns 2 values, x and y
int entity_GetPosition(lua_State* ls) 
{
	PRINT("entity_GetPosition");
	luaCountArgs(ls, 1);

	Entity* e = _getReferencedEntity(ls);

	point2d p = e->GetPosition();
	lua_pushnumber(ls, p.x);
	lua_pushnumber(ls, p.y);
	
	return 2;	
}

// .SetPosition(entity, x, y)
int entity_SetPosition(lua_State* ls) 
{
	PRINT("entity_SetPosition");
	luaCountArgs(ls, 3);
	
	Entity* e = _getReferencedEntity(ls);

	point2d p((sShort)lua_tonumber(ls, 2), (sShort)lua_tonumber(ls, 3));
	e->SetPosition(p);

	return 0;	
}

// x, y, w, h = .GetRect(entity) - Where x, y are coordinates of the top left our our avatar on the map. w, h are avatar dimensions.
int entity_GetRect(lua_State* ls) 
{
	PRINT("entity_GetRect");
	luaCountArgs(ls, 1);

	Entity* e = _getReferencedEntity(ls);

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
	luaCountArgs(ls, 2);
	
	Entity* e = _getReferencedEntity(ls);

	string prop = lua_tostring(ls, 2);
	
	//TODO: Improve this somehow! (How did Valve do it?)
	if (prop == "id") lua_pushstring( ls, e->mId.c_str() );
	else if (prop == "name") lua_pushstring( ls, e->mName.c_str() );
	else if (prop == "visible") lua_pushnumber( ls, e->IsVisible() );
	else if (prop == "solid") lua_pushnumber( ls, e->IsSolid() );
	else if (prop == "shadow") lua_pushnumber( ls, e->mShadow );
	else if (prop == "layer") lua_pushnumber( ls, e->GetLayer() );
	else if (prop == "type") lua_pushstring( ls, e->GetTypeName().c_str() );
	else return luaError(ls, "Entity.GetProp", prop + " unknown");

	return 1;
}

// .SetProp(entity, "property", value) Sets the property to the specified value. Value can be num, string, ptr, depends on the property.
int entity_SetProp(lua_State* ls)
{
	PRINT("entity_SetProp");
	luaCountArgs(ls, 3);

	Entity* e = _getReferencedEntity(ls);

	string prop = lua_tostring(ls, 2);
	
	//TODO: Improve this somehow! (How did Valve do it?)
	if (prop == "id") e->mId = lua_tostring(ls, 3);
	else if (prop == "name") e->mName = lua_tostring(ls, 3);
	else if (prop == "visible") e->SetVisible( lua_tonumber(ls, 3) );
	else if (prop == "solid") e->SetSolid( lua_tonumber(ls, 3) );
	else if (prop == "shadow") e->mShadow = lua_tonumber(ls, 3);
	else if (prop == "layer") e->SetLayer( (byte)lua_tonumber(ls, 3) );
	else return luaError(ls, "Entity.SetProp", prop + " unknown");

	return 0;
}

// .GetFlag(entity, "key") Returns string of value. Empty string if key does not exist.
int entity_GetFlag(lua_State* ls) 
{
	PRINT("entity_GetFlag");
	luaCountArgs(ls, 2);

	Entity* e = _getReferencedEntity(ls);

	string s = e->GetFlag( lua_tostring(ls, 2) );
	lua_pushstring(ls, s.c_str());
	
	return 1;
}

// .SetFlag(entity, "key", "value")
int entity_SetFlag(lua_State* ls) 
{
	PRINT("entity_SetFlag");
	luaCountArgs(ls, 3);

	Entity* e = _getReferencedEntity(ls);

	e->SetFlag(lua_tostring(ls, 2), lua_tostring(ls, 3));

	return 0;	
}

// .IsTouching(ent, ent) - Returns 1 if the two entities are intersecting collision rects, 0 otherwise. 
//Also note, if the second ent is a bad pointer, it'll return 0 also.
int entity_IsTouching(lua_State* ls)
{
	PRINT("entity_IsTouching");
	luaCountArgs(ls, 2);
	
	Entity* e = _getReferencedEntity(ls);

	Entity* e2 = (Entity*)lua_touserdata(ls, 2);
	
	//Return 0 if e2 is invalid, or they're not touching
	if ( !_verifyEntity(e2) || !e->IsCollidingWithEntity(e2) )
		lua_pushnumber(ls, 0);
	else
		lua_pushnumber(ls, 1);

	return 1;
}

// .GetDistance(ent, ent) - Returns distance between GetPosition() points of both entities. Error if either entity is invalid.
int entity_GetDistance(lua_State* ls)
{
	PRINT("entity_GetDistance");
	luaCountArgs(ls, 2);
	
	Entity* e = _getReferencedEntity(ls);
	Entity* e2 = _getReferencedEntity(ls, 2);

	lua_pushnumber( ls, getDistance(e->GetPosition(), e2->GetPosition()) );

	return 1;
}

// .Say(entity, "msg", showbubble, showinchat) - Say the message. Last two parameters are optional, and default to 1.
int entity_Say(lua_State* ls)
{
	PRINT("entity_Say");
	luaCountArgs(ls, 2);
	
	Entity* e = _getReferencedEntity(ls);

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

//	ent = .Add("id") - Create a replica of the loaded entity matching id and return a cptr to it. You must manually position it and such.
int entity_Add(lua_State* ls)
{
	PRINT("entity_Add");
	luaCountArgs(ls, 1);
	
	ASSERT(game->mMap);

	if (!lua_isstring(ls, 1))
		return luaError(ls, "Entity.Add", "Invalid Param");
	
	string id = lua_tostring(ls, 1);

	Entity* e = game->mMap->AddEntityFromResources( id, point2d() );
	if (!e)
		return luaError(ls, "Entity.Add", id + " not a loaded resource");

	lua_pushlightuserdata(ls, e);
	return 1;
}

//	.Remove(entity) - Remove the specified entity from the map. Returns 1 on success, 0 otherwise. 
int entity_Remove(lua_State* ls)
{
	PRINT("entity_Remove");
	luaCountArgs(ls, 1);
	
	Entity* e = (Entity*)lua_touserdata(ls, 1);
	
	bool result = game->mMap->RemoveEntity(e);
	
	lua_pushnumber(ls, result);
	return 1;
}

//	.RemoveAllById("id") - Removes all entities with the specified ID. Returns 1 if at least one was removed, 0 otherwise
int entity_RemoveAllById(lua_State* ls)
{
	PRINT("entity_Remove");
	luaCountArgs(ls, 1);
	
	string id = lua_tostring(ls, 1);
	bool result = game->mMap->RemoveAllEntitiesById(id);
	
	lua_pushnumber(ls, result);
	return 1;
}

// entity_ptr Entity.NewTextObject(text, x, y, size<default_font_size>, layer<1>, rotation<0.0>)
int entity_NewTextObject(lua_State* ls)
{
	PRINT("entity_NewTextObject");
	luaCountArgs(ls, 3);

	int numArgs = lua_gettop(ls);

	//Read in our buttload of properties, and check for defaults
	string text = lua_tostring(ls, 1);
	point2d pos( (sShort)lua_tonumber(ls, 2), (sShort)lua_tonumber(ls, 3) );
	int size = (numArgs > 3) ? (int)lua_tonumber(ls, 4) : 0;
	byte layer = (numArgs > 4) ? (byte)lua_tonumber(ls, 5) : 0;
	double rot = (numArgs > 5) ? lua_tonumber(ls, 6) : 0.0;

	//Actually create it, and add it
	TextObject* e = new TextObject();
	e->mMap = game->mMap;
	e->mMap->AddEntity( e, layer );
	e->SetText(text, size, rot);
	e->SetPosition(pos);
	
	lua_pushlightuserdata(ls, e);
	return 1;
}


static const luaL_Reg functions[] = {
	{"Exists", entity_Exists},
	{"FindById", entity_FindById},
	{"FindAllById", entity_FindAllById},
	{"FindByName", entity_FindByName},
	{"FindAllByName", entity_FindAllByName},
	{"GetPosition", entity_GetPosition},
	{"SetPosition", entity_SetPosition},
	{"GetRect", entity_GetRect},
	{"GetProp", entity_GetProp},
	{"SetProp", entity_SetProp},
	{"GetFlag", entity_GetFlag},
	{"SetFlag", entity_SetFlag},
	{"IsTouching", entity_IsTouching},
	{"GetDistance", entity_GetDistance},
	{"Say", entity_Say},
	{"Add", entity_Add},
	{"Remove", entity_Remove},
	{"RemoveAllById", entity_RemoveAllById},
	{"NewTextObject", entity_NewTextObject},
	{NULL, NULL}
};

void RegisterEntityLib(lua_State* ls)
{
	luaL_register( ls, "Entity", functions );
}
