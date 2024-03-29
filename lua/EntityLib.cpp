
/*
 * Copyright (c) 2011 Chase McManning
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <lua.hpp>
#include "EntityLib.h"
#include "LuaCommon.h"
#include "../entity/Actor.h"
#include "../entity/TextObject.h"
#include "../entity/ExplodingEntity.h"
#include "../entity/DamageIcon.h"
#include "../entity/ChatBubble.h"
//#include "../entity/Lunem.h"
#include "../game/GameManager.h"
#include "../map/Map.h"

// Returns true if the entity is valid. (TODO: Make sure it's on the map?)
bool _verifyEntity(Entity* e)
{
	return (e != NULL);
}

// Referenced entity is always the first parameter of the state.
// This should always return a valid entity pointer. If it's invalid, there will be a longjmp from within lua.
Entity* _getReferencedEntity(lua_State* ls, int index = 1)
{

	Entity* e = NULL;

	if (lua_istable(ls, index))
	{
		// try to pull the pointer from the table
		lua_pushstring(ls, "__centity");
		lua_gettable(ls, index);
		e = (Entity*)(lua_touserdata(ls, -1));
		lua_pop(ls, 1);
	}
	else // assume userdata
	{
		e = (Entity*)(lua_touserdata(ls, index));
		if (!_verifyEntity(e))
		{
			string err = "index " + its(index) + " not a valid entity pointer.";
			luaError(ls, "", err);
		}
	}

	return e;
}

// .Exists(entity) - Returns true if the entity pointer is valid and on the map, false otherwise.
// This is preferred over the checking-for-valid-entity-every-time-it's-accessed approach due to it being MUCH faster.
int entity_Exists(lua_State* ls)
{
	DEBUGOUT("entity_Exists");
	luaCountArgs(ls, 1);

	Entity* e = (Entity*)lua_touserdata(ls, 1);

	bool result = game->mMap->FindEntity(e);

	// also check whether or not they're dead
	if (result)
		result = !e->mDead;

	lua_pushboolean( ls, result );
	return 1;
}

// .FindById("id") returns cptr to entity, nil if it doesn't exist.
int entity_FindById(lua_State* ls)
{
	DEBUGOUT("entity_FindById");
	luaCountArgs(ls, 1);

	ASSERT(game->mMap);

	Entity* e = game->mMap->FindEntityById( lua_tostring(ls, 1) );
	if (!e)
		lua_pushnil(ls);
	else
		lua_pushlightuserdata(ls, e);

	return 1;
}

// .FindAllById("id", 0) - returns a table with indexes 1->N containing all entities on the map with matching id.
// If the second parameter is supplied, it's a bool indicating whether to use wildcard matching or not
int entity_FindAllById(lua_State* ls)
{
	DEBUGOUT("entity_FindAllById");
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
	for (int i = 0; i < game->mMap->mEntities.size(); ++i)
	{
		e = game->mMap->mEntities.at(i);

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

	return 1;
}

// .FindByName("name") returns cptr to entity, nil if it doesn't exist.
int entity_FindByName(lua_State* ls)
{
	DEBUGOUT("entity_FindByName");
	luaCountArgs(ls, 1);

	ASSERT(game->mMap);

	Entity* e = game->mMap->FindEntityByName( lua_tostring(ls, 1) );
	if (!e)
		lua_pushnil(ls);
	else
		lua_pushlightuserdata(ls, e);

	return 1;
}

// .FindAllByName("name", 0) - returns a table with indexes 1->N containing all entities on the map with matching names.
// If the second parameter is supplied, it's a bool indicating whether to use wildcard matching or not
int entity_FindAllByName(lua_State* ls)
{
	DEBUGOUT("entity_FindAllByName");
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
	for (int i = 0; i < game->mMap->mEntities.size(); ++i)
	{
		e = game->mMap->mEntities.at(i);

		//if it matches the search, add to table
		if ( (useWildmatch && wildmatch(name.c_str(), e->GetName().c_str()))
			|| (!useWildmatch && e->GetName() == name) )
		{
			lua_pushnumber(ls, index);
			lua_pushlightuserdata(ls, e);
			lua_settable(ls, top);
			++index;
		}
	}

	return 1;
}

// x, y = .GetPosition(entity)
int entity_GetPosition(lua_State* ls)
{
	DEBUGOUT("entity_GetPosition");
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
	DEBUGOUT("entity_SetPosition");
	luaCountArgs(ls, 3);

	Entity* e = _getReferencedEntity(ls);

	point2d p((sShort)lua_tonumber(ls, 2), (sShort)lua_tonumber(ls, 3));
	e->SetPosition(p);

	return 0;
}

int entity_GetOrigin(lua_State* ls)
{
	luaCountArgs(ls, 1);

	Entity* e = _getReferencedEntity(ls);

	lua_pushnumber(ls, e->mOrigin.x);
	lua_pushnumber(ls, e->mOrigin.y);

	return 2;
}

int entity_SetOrigin(lua_State* ls)
{
	luaCountArgs(ls, 3);

	Entity* e = _getReferencedEntity(ls);

	point2d p((sShort)lua_tonumber(ls, 2), (sShort)lua_tonumber(ls, 3));
	e->mOrigin = p;

	return 0;
}

// x, y, w, h = .GetRect(entity) - Where x, y are coordinates of the top left our our avatar on the map. w, h are avatar dimensions.
int entity_GetRect(lua_State* ls)
{
	DEBUGOUT("entity_GetRect");
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
	DEBUGOUT("entity_GetProp");
	luaCountArgs(ls, 2);

	Entity* e = _getReferencedEntity(ls);

	string prop = lowercase(lua_tostring(ls, 2));

	int result = e->LuaGetProp(ls, prop);

	if (!result)
		console->AddMessage("Entity.GetProp() '" + prop + "' Unknown");

	return result;
}

// .SetProp(entity, "property", value) Sets the property to the specified value. Value can be num, string, ptr, depends on the property.
int entity_SetProp(lua_State* ls)
{
	DEBUGOUT("entity_SetProp");
	luaCountArgs(ls, 3);

	Entity* e = _getReferencedEntity(ls);

	string prop = lowercase(lua_tostring(ls, 2));

	int result = e->LuaSetProp(ls, prop, 3);

	if (!result)
		console->AddMessage("Entity.SetProp() '" + prop + "' Unknown");

	return 0;
}

// .IsTouching(ent, ent) - Returns true if the two entities are intersecting collision rects, false otherwise.
//Also note, if the second ent is a bad pointer, it'll return false also.
// ALSO ALSO, can do .IsTouching(ent, x, y) for a point test!
int entity_IsTouching(lua_State* ls)
{
	DEBUGOUT("entity_IsTouching");
	luaCountArgs(ls, 2);
	int numArgs = lua_gettop(ls);

	Entity* e = _getReferencedEntity(ls);

	if (numArgs > 2 && lua_isnumber(ls, 2))
	{
		// test for point
		lua_pushboolean(ls, e->CollidesWith(
						rect((int)lua_tonumber(ls, 2), (int)lua_tonumber(ls, 3),
							1, 1))
						);
	}
	else // test for entity
	{
		Entity* e2 = (Entity*)lua_touserdata(ls, 2);
		lua_pushboolean(ls, _verifyEntity(e2) && e->IsCollidingWithEntity(e2));
	}

	return 1;
}

// .GetDistance(ent, ent) - Returns distance between GetPosition() points of both entities. Error if either entity is invalid.
int entity_GetDistance(lua_State* ls)
{
	DEBUGOUT("entity_GetDistance");
	luaCountArgs(ls, 2);

	Entity* e = _getReferencedEntity(ls);
	Entity* e2 = _getReferencedEntity(ls, 2);

	lua_pushnumber( ls, getDistance(e->GetPosition(), e2->GetPosition()) );

	return 1;
}

// .Say(entity, "msg", showbubble<true>, showinchat<true>) - Say the message. Last two parameters are optional, and default to 1.
int entity_Say(lua_State* ls)
{
	DEBUGOUT("entity_Say");
	luaCountArgs(ls, 2);

	Entity* e = _getReferencedEntity(ls);

	int numArgs = lua_gettop(ls);

	string msg = lua_tostring(ls, 2);
	bool showbubble = (numArgs > 2) ? lua_toboolean(ls, 3) : true;
	bool showinchat = (numArgs > 3) ? lua_toboolean(ls, 4) : true;

	e->Say(msg, showbubble, showinchat);

	//Dispatch a say message
	MessageData md("ENTITY_SAY");
	md.WriteUserdata("entity", e);
	md.WriteString("message", msg);
	messenger.Dispatch(md, NULL);

	return 0;
}

//	.Remove(entity) - Remove the specified entity from the map. Returns true on success, false otherwise.
int entity_Remove(lua_State* ls)
{
	luaCountArgs(ls, 1);

	bool result;
	if (lua_isnil(ls, 1))
	{
		result = false;
	}
	else
	{
		Entity* e = (Entity*)lua_touserdata(ls, 1);

		if (e)
		{
            if (e->mType == ENTITY_REMOTEACTOR || e->mType == ENTITY_LOCALACTOR)
            {
                luaError(ls, "Entity.Remove", "Tried to remove Remote/Local Actor: " + e->mName);
            }

            result = game->mMap->RemoveEntity(e);
		}
	}

	lua_pushboolean(ls, result);
	return 1;
}

//	.RemoveAllById("id") - Removes all entities with the specified ID. Returns true if at least one was removed, false otherwise
int entity_RemoveAllById(lua_State* ls)
{
	DEBUGOUT("entity_Remove");
	luaCountArgs(ls, 1);

	string id = lua_tostring(ls, 1);
	bool result = game->mMap->RemoveAllEntitiesById(id);

	lua_pushboolean(ls, result);
	return 1;
}

/*	Read in t.Collision = { 1, 2, 3, 4, ... } array of rects into Entity collisions list
	Returns 0 if malformed, 1 otherwise.
*/
int _parseEntityCollision(lua_State* ls, Entity* e)
{
	rect r;
	if (!lua_istable(ls, -1))
		return 0;

	e->mCollisionRects.clear(); //clear up anything that may still be around

	lua_pushnil(ls);
	while (lua_next(ls, -2) != 0)
	{
		//Run through 4 items manually to read in a single rect
		r.x = (int)lua_tonumber(ls, -1);
		lua_pop(ls, 1);

		if (lua_next(ls, -2)) { r.y = (int)lua_tonumber(ls, -1); lua_pop(ls, 1); } else return 0;
		if (lua_next(ls, -2)) { r.w = (int)lua_tonumber(ls, -1); lua_pop(ls, 1); } else return 0;
		if (lua_next(ls, -2)) { r.h = (int)lua_tonumber(ls, -1); lua_pop(ls, 1); } else return 0;

		e->mCollisionRects.push_back(r);
	}

	return 1;
}

/*		Read in	t.CollisionFile = "Filename"
*/
int _parseEntityCollisionFile(lua_State* ls, Entity* e)
{
	if (lua_isstring(ls, -1))
		return e->LoadCollisionFile(game->mMap->mWorkingDir + lua_tostring(ls, -1));

	return 0;
}
/*		Read in	t.Image = { File = "Something", Width = #, Delay = #, Clip = {x, y, w, h} }
			OR 	t.Image = "Filename"
*/
int _parseEntityImage(lua_State* ls, StaticObject* so, int virtualIndex = -1)
{
	//Virtual index can NOT be an offset from the top, must be an absolute position.
	// This is because the lua_next() will screw up unless we specify an absolute.
	if (virtualIndex < 0)
		virtualIndex = lua_gettop(ls) + virtualIndex + 1;

	//printf("vi: %i top:%i\n", virtualIndex, lua_gettop(ls));
	//luaStackdump(ls);

	string file;

	if (lua_isstring(ls, virtualIndex))
	{
		file = game->mMap->mWorkingDir + lua_tostring(ls, virtualIndex);
		if (!so->LoadImage(file))
		{
			return luaError(ls, "", "Could not load entity image: " + file);
		}
		return 1;
	}

	if (!lua_istable(ls, virtualIndex))
		return 0;

	string key;
	int width = 0, delay = 1000;

	lua_pushnil(ls); /* first key */
	while (lua_next(ls, virtualIndex) != 0)
	{
		/* key = index -2, value = index -1 */
		if (lua_isstring(ls, -2))
		{
			key = lua_tostring(ls, -2);
			if (key == "File")
			{
				file = game->mMap->mWorkingDir + lua_tostring(ls, -1);
				if (!so->LoadImage(file))
				{
					return luaError(ls, "", "Could not load entity image: " + file);
				}
			}
			else if (key == "Width")
				width = (int)lua_tonumber(ls, -1);
			else if (key == "Delay")
				delay = (int)lua_tonumber(ls, -1);
		}
		lua_pop(ls, 1); //pop value, keep key for next iteration
	}

	// TODO: A better way to handle this?
	if (width > 0)
	{
		so->ToHorizontalAnimation(width, delay);
	}

	return 1;
}

int _parseEntityAvatar(lua_State* ls, Actor* a, int virtualIndex = -1)
{
	//Virtual index can NOT be an offset from the top, must be an absolute position.
	// This is because the lua_next() will screw up unless we specify an absolute.
	if (virtualIndex < 0)
		virtualIndex = lua_gettop(ls) + virtualIndex + 1;

	if (!lua_istable(ls, virtualIndex))
		return 0;

	string key, file;
	int width = 0, height = 0, delay = 1000, flags = 0;

	lua_pushnil(ls);
	while (lua_next(ls, virtualIndex) != 0)
	{
		if (lua_isstring(ls, -2))
		{
			key = lua_tostring(ls, -2);
			if (key == "File")
				file = lua_tostring(ls, -1);
			else if (key == "Width")
				width = (int)lua_tonumber(ls, -1);
			else if (key == "Height")
				height = (int)lua_tonumber(ls, -1);
			else if (key == "Flags")
				flags = (int)lua_tonumber(ls, -1);
			else if (key == "Delay")
				delay = (int)lua_tonumber(ls, -1);
		}
		lua_pop(ls, 1); //pop value
	}

	if (!file.empty())
	{
		// don't allow them out of our cache unless they're using an avy:// construct
		if (file.find("avy://") == string::npos)
			file = game->mMap->mWorkingDir + file;

		a->LoadAvatar(file, "", width, height, delay, flags);
	}
	return 1;
}

/*	Read in t.Origin = { 0, 0 } two ints, (x, y). Returns 1 if both are read in, 0 otherwise */
int _parseEntityOrigin(lua_State* ls, Entity* e)
{
	point2d p;
	if (!lua_istable(ls, -1))
		return 0;

	lua_pushnil(ls);
	if (!lua_next(ls, -2))
		return 0;

	p.x = (int)lua_tonumber(ls, -1);
	lua_pop(ls, 1);

	if (!lua_next(ls, -2))
		return 0;

	p.y = (int)lua_tonumber(ls, -1);
	lua_pop(ls, 1);

	e->mOrigin = p;

	//Have to manually pop off the table because we're not running lua_next() to completion
	lua_pop(ls, 1);

	return 1;
}

/**	Table at the top of the stack will be in the form { R, G, B, A }
	@return 0 if it didn't manage to read in the entire RGBA, 1 otherwise.
*/
int _getColorTable(lua_State* ls, color& c)
{
	if (!lua_istable(ls, -1))
		return 0;

	lua_pushnil(ls);

	if (!lua_next(ls, -2)) return 0;
	c.r = (int)lua_tonumber(ls, -1);
	lua_pop(ls, 1);

	if (!lua_next(ls, -2)) return 0;
	c.g = (int)lua_tonumber(ls, -1);
	lua_pop(ls, 1);

	if (!lua_next(ls, -2)) return 0;
	c.b = (int)lua_tonumber(ls, -1);
	lua_pop(ls, 1);

	if (!lua_next(ls, -2)) return 0;
	c.a = (int)lua_tonumber(ls, -1);
	lua_pop(ls, 1);

	//Have to manually pop off the table because we're not running lua_next() to completion
	lua_pop(ls, 1);

	return 1;
}


/* 	key is index -2, value is index -1
	@return 0 on error, 1 otherwise
*/
int _parseSingleEntityProperty(lua_State* ls, string key, Entity* e)
{
	color c;
	if (key == "ID" || (e->mType == ENTITY_DAMAGEICON && key == "Amount"))
	{
		e->mId = lua_tostring(ls, -1);
	}
	else if (key == "Name")
	{
		e->SetName( lua_tostring(ls, -1) );
	}
	/*else if (key == "Species" && e->mType == ENTITY_LUNEM)
	{
		((Lunem*)e)->SetSpecies( lua_tostring(ls, -1) );
	}*/
	else if (key == "Visible")
	{
		e->SetVisible( lua_toboolean(ls, -1) );
	}
	else if (key == "Solid")
	{
		e->SetSolid( lua_toboolean(ls, -1) );
	}
	else if (key == "Shadow")
	{
		e->mShadow = lua_toboolean(ls, -1);
	}
	else if (key == "Clickable")
	{
		e->mClickRange = (int)lua_tonumber(ls, -1);
	}
	else if (key == "Layer")
	{
		e->SetLayer( (int)lua_tonumber(ls, -1) );
	}
	else if (key == "Collision") //have to deal with an array of ints
	{
		return _parseEntityCollision(ls, e);
	}
	else if (key == "CollisionFile")
	{
		return _parseEntityCollisionFile(ls, e);
	}
	else if (key == "Origin")
	{
		return _parseEntityOrigin(ls, e); //don't really care if it fails
	}
	else if (key == "Image" && e->mType == ENTITY_STATICOBJECT)
	{
		return _parseEntityImage(ls, (StaticObject*)e);
	}
	else if (key == "Avatar" && e->mType >= ENTITY_ACTOR && e->mType < ENTITY_END_ACTORS)
	{
		return _parseEntityAvatar(ls, (Actor*)e);
	}
	else if (key == "BgColor" && e->mType == ENTITY_DAMAGEICON)
	{
		_getColorTable(ls, c);
		((DamageIcon*)e)->mImage->ColorizeGreyscale(c);
	}
	else if (key == "FontColor" && e->mType == ENTITY_DAMAGEICON)
	{
		_getColorTable(ls, c);
		((DamageIcon*)e)->mFontImage->ColorizeGreyscale(c);
	}

	return 1;
}

/*	Iterates through all members of the entity table,
	handling each property uniquely
*/
int _parseEntityProperties(lua_State* ls, Entity* e, int tableIndex)
{
	lua_pushnil(ls);

	string key;
	while (lua_next(ls, tableIndex) != 0)
	{
		if (lua_isstring(ls, -2)) //make sure they key is a string before screwing with it
		{
			key = lua_tostring(ls, -2);
			_parseSingleEntityProperty(ls, key, e);
		}
		else
		{
			console->AddMessage("[_parseEntityProperties] key != string");
		}
		lua_pop(ls, 1); //pop value

	} //while lua_next != 0

	return 1;
}

Entity* _createEntity(int type)
{
	Entity* e = NULL;
	switch (type)
	{
		case ENTITY_STATICOBJECT:
			e = new StaticObject;
			break;
		case ENTITY_ACTOR:
			e = new Actor;
			break;
		case ENTITY_TEXT:
			e = new TextObject;
			break;
		case ENTITY_DAMAGEICON:
			e = new DamageIcon;
			break;
		//case ENTITY_LUNEM:
		//	e = new Lunem;
			break;
		default:
			break;
	}
	return e;
}

/* entity = Entity.Create(entityInfoTable, x<nil>, y<nil>);
	Create a new entity instance and place it on the map at (x, y)
	If (x, y) are nil, it will create a new entity and return it, but NOT
	add it to the map. It's our responsibility to do Entity.Add(ent, x, y)
	later, or to do something else special with it (Such as adding to the
	players party)
*/
int entity_Create(lua_State* ls)
{
	Entity* e;
	point2d p;

	// Make sure they passed in a table as the first parameter
	if (!lua_istable(ls, 1))
	{
		return luaError(ls, "Entity.Create", "First param must be a table");
	}

	// Grab t.Type and create an entity associated with that type
	lua_pushstring(ls, "Type");
	lua_gettable(ls, 1);
	int type = (int)lua_tonumber(ls, -1);
	lua_pop(ls, 1);

	// Create the entity class based on the type provided

	e = _createEntity(type);
	if (!e)
	{
		return luaError(ls, "Entity.Create", "Invalid type: " + its(type));
	}

	// Configure the class via all the properties the class defines
	if (!_parseEntityProperties(ls, e, 1))
	{
		return luaError(ls, "Entity.Create", "Error parsing properties");
	}

	int count = lua_gettop(ls);

	if (count > 1)
	{
		//Finally, add it to the map and return a reference to it
		e->mMap = game->mMap;
		game->mMap->AddEntity(e);

		p.x = (int)lua_tonumber(ls, 2);
		p.y = (int)lua_tonumber(ls, 3);

		e->SetPosition(p);
	}

	lua_pushlightuserdata(ls, e);
	return 1;
}

/*	.Add(ent, x, y) - Will add the specified entity pointer to the map at (x,y) Only really useful for when
		Entities are created through Entity.Create(ent) where no map coordinates are specified
		Returns true if the entity was added, false otherwise
*/
int entity_Add(lua_State* ls)
{
	luaCountArgs(ls, 3);

	point2d p;
	Entity* e = (Entity*)(lua_touserdata(ls, 1));

	if (e && e->mMap == NULL)
	{
		e->mMap = game->mMap;
		game->mMap->AddEntity(e);

		p.x = (int)lua_tonumber(ls, 2);
		p.y = (int)lua_tonumber(ls, 3);

		e->SetPosition(p);

		lua_pushboolean(ls, true);
	}
	else
	{
		lua_pushboolean(ls, false);
	}

	return 1;
}

//	Set the image of the specified static object entity. Can take either string or table form.
//	.SetImage(entity, "entities/world/file.png");
//	.SetImage(entity, {...});
int entity_SetImage(lua_State* ls)
{
	DEBUGOUT("entity_SetImage");
	luaCountArgs(ls, 2);

	Entity* e = _getReferencedEntity(ls);
	if (!e || e->mType != ENTITY_STATICOBJECT)
		return 0;

	return _parseEntityImage(ls, (StaticObject*)e, 2);
}

//	Set the avatar of the actor entity
//	.SetAvatar(entity, {...});
int entity_SetAvatar(lua_State* ls)
{
	DEBUGOUT("entity_SetAvatar");
	luaCountArgs(ls, 2);

	Entity* e = _getReferencedEntity(ls);
	if (!e || !(e->mType >= ENTITY_ACTOR && e->mType < ENTITY_END_ACTORS))
		return 0;

	return _parseEntityAvatar(ls, (Actor*)e, 2);
}

// .Explode(ent) - Will create an explosion entity at ent's position
// 		This will not remove ent from the map!
int entity_Explode(lua_State* ls)
{
	Image* img;
	rect r;
	Entity* e = _getReferencedEntity(ls);

	if (e)
	{
		img = e->GetImage();
		if (img)
		{
			r = e->GetBoundingRect();
			new ExplodingEntity(e->mMap, img, point2d(r.x, r.y));
		}
	}

	return 0;
}

//	.SetText(ent, "text", r, g, b, maxWidth<0>) - Set displayed text of a text object entity
int entity_SetText(lua_State* ls)
{
	luaCountArgs(ls, 2);

	int args = lua_gettop(ls);
	int width = 0;
	color c;
	string text;

	TextObject* o = (TextObject*)_getReferencedEntity(ls);
	if (!o || o->mType != ENTITY_TEXT)
	{
		return luaError(ls, "Entity.SetText", "Invalid Entity type");
	}

	text = lua_tostring(ls, 2);
	c.r = (int)lua_tonumber(ls, 3);
	c.g = (int)lua_tonumber(ls, 4);
	c.b = (int)lua_tonumber(ls, 5);

	if (args > 5)
		width = (int)lua_tonumber(ls, 6);

	o->SetText(text, c, width);
	return 0;
}

//	.SetFont(ent, "face"<nil>, size<nil>, style<nil>) - Set displayed text of a text object entity
//		Any non-defined parameter will use user defaults
int entity_SetFont(lua_State* ls)
{
	luaCountArgs(ls, 1);

	int args = lua_gettop(ls);
	int size = 0, style = 0;
	string face;

	TextObject* o = (TextObject*)_getReferencedEntity(ls);
	if (!o || o->mType != ENTITY_TEXT)
	{
		return luaError(ls, "Entity.SetFont", "Invalid Entity type");
	}

	if (args > 1 && lua_isstring(ls, 2))
		face = lua_tostring(ls, 2);

	if (args > 2 && lua_isnumber(ls, 3))
		size = (int)lua_tonumber(ls, 3);

	if (args > 3 && lua_isnumber(ls, 4))
		style = (int)lua_tonumber(ls, 4);

	o->SetFont(face, size, style);
	return 0;
}

static const luaL_Reg functions[] = {
	{"Exists", entity_Exists},
	{"FindById", entity_FindById},
	{"FindAllById", entity_FindAllById},
	{"FindByName", entity_FindByName},
	{"FindAllByName", entity_FindAllByName},
	{"GetPosition", entity_GetPosition},
	{"SetPosition", entity_SetPosition},
	{"GetOrigin", entity_GetOrigin},
	{"SetOrigin", entity_SetOrigin},
	{"GetRect", entity_GetRect},
	{"GetProp", entity_GetProp},
	{"SetProp", entity_SetProp},
	{"IsTouching", entity_IsTouching},
	{"GetDistance", entity_GetDistance},
	{"Say", entity_Say},
	{"Remove", entity_Remove},
	{"RemoveAllById", entity_RemoveAllById},
	{"Create", entity_Create},
	{"Add", entity_Add},
	{"SetImage", entity_SetImage},
	{"SetAvatar", entity_SetAvatar},
	{"Explode", entity_Explode},
	{"SetText", entity_SetText},
	{"SetFont", entity_SetFont},
	{NULL, NULL}
};

void RegisterEntityLib(lua_State* ls)
{
	luaL_register( ls, "Entity", functions );
}
