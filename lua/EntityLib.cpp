
#include <lua.hpp>
#include "EntityLib.h"
#include "LuaCommon.h"
#include "../entity/Actor.h"
#include "../entity/TextObject.h"
#include "../entity/ExplodingEntity.h"
#include "../game/GameManager.h"
#include "../map/BasicMap.h"

// Returns true if the entity is valid. (TODO: Make sure it's on the map) 
bool _verifyEntity(Entity* e)
{
	return (e != NULL);
}

// Referenced entity is always the first parameter of the state.
// This should always return a valid entity pointer. If it's invalid, there will be a longjmp from within lua.
Entity* _getReferencedEntity(lua_State* ls, int index = 1)
{
	Entity* e = (Entity*)(lua_touserdata(ls, index));
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
	
	int result = (game->mMap->FindEntity(e) == -1) ? 0 : 1;

	lua_pushnumber( ls, result );
	return 1;
}

// .FindById("id") returns cptr to entity, nil if it doesn't exist.
int entity_FindById(lua_State* ls) 
{
	PRINT("entity_FindById");
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

// .FindByName("name") returns cptr to entity, 0 if it doesn't exist.
int entity_FindByName(lua_State* ls) 
{
	PRINT("entity_FindByName");
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
	for (int i = 0; i < game->mMap->mEntities.size(); ++i)
	{
		e = game->mMap->mEntities.at(i);
		
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
	
	return 1;
}

// x, y = .GetPosition(entity)
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

	int result = e->LuaGetProp(ls, prop);

	if (!result)
		console->AddMessage("Entity.GetProp() '" + prop + "' Unknown");
	
	return result;
}

// .SetProp(entity, "property", value) Sets the property to the specified value. Value can be num, string, ptr, depends on the property.
int entity_SetProp(lua_State* ls)
{
	PRINT("entity_SetProp");
	luaCountArgs(ls, 3);

	Entity* e = _getReferencedEntity(ls);

	string prop = lua_tostring(ls, 2);

	int result = e->LuaSetProp(ls, prop, 3);

	if (!result)
		console->AddMessage("Entity.SetProp() '" + prop + "' Unknown");

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
	int layer = (numArgs > 4) ? (int)lua_tonumber(ls, 5) : 0;
	double rot = (numArgs > 5) ? lua_tonumber(ls, 6) : 0.0;

	//Actually create it, and add it
	TextObject* e = new TextObject();
	e->mMap = game->mMap;
	e->SetLayer(layer);
	e->mMap->AddEntity(e);
	e->SetText(text, size, rot);
	e->SetPosition(pos);
	
	lua_pushlightuserdata(ls, e);
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
/*		Read in	t.Image = { File = "Something", Width = #, Delay = # }	
			OR 	t.Image = "Filename"
*/
int _parseEntityImage(lua_State* ls, StaticObject* so, int virtualIndex = -1)
{
	//Virtual index can NOT be an offset from the top, must be an absolute position.
	// This is because the lua_next() will screw up unless we specify an absolute.
	if (virtualIndex < 0)
		virtualIndex = lua_gettop(ls) + virtualIndex + 1;
	
	printf("vi: %i top:%i\n", virtualIndex, lua_gettop(ls));
	luaStackdump(ls);
	
	if (lua_isstring(ls, virtualIndex))
	{
		so->LoadImage(game->mMap->mWorkingDir + lua_tostring(ls, virtualIndex));
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
				so->LoadImage(game->mMap->mWorkingDir + lua_tostring(ls, -1));
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
		if (so->mImage)
			so->mImage->ConvertToHorizontalAnimation(rect(0, 0, width, so->mImage->Height()), delay);
		if (so->mOriginalImage)
			so->mOriginalImage->ConvertToHorizontalAnimation(rect(0, 0, width, so->mOriginalImage->Height()), delay);
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
	int width = 0, height = 0, delay = 1000;
	bool loopsit = false, loopstand = false;
	
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
			else if (key == "LoopSit")
				loopsit = lua_toboolean(ls, -1);
			else if (key == "LoopStand")
				loopstand = lua_toboolean(ls, -1);
			else if (key == "Delay")
				delay = (int)lua_tonumber(ls, -1);
		}
		lua_pop(ls, 1); //pop value	
	}
	
	if (!file.empty())
	{
		a->LoadAvatar(game->mMap->mWorkingDir + file, "", width, height, delay, loopstand, loopsit);
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
	
/* 	key is index -2, value is index -1 
	@return 0 on error, 1 otherwise
*/
int _parseSingleEntityProperty(lua_State* ls, string key, Entity* e)
{
	if (key == "ID")
	{
		e->mId = lua_tostring(ls, -1);
	}
	else if (key == "Name")
	{
		e->mName = lua_tostring(ls, -1);
	}
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
		e->mCanClick = lua_toboolean(ls, -1);	
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
		default:
			break;
	}
	return e;
}
	
/* entity = Entity.Create(entityInfoTable, x, y);
	Create a new entity instance and place it on the map at (x, y)
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

	//Finally, add it to the map and return a reference to it
	e->mMap = game->mMap;
	game->mMap->AddEntity(e);
	
	p.x = (int)lua_tonumber(ls, 2);
	p.y = (int)lua_tonumber(ls, 3);
	
	e->SetPosition(p);
	
	lua_pushlightuserdata(ls, e);
	return 1;
}

//	Set the image of the specified static object entity. Can take either string or table form.
//	.SetImage(entity, "entities/world/file.png");
//	.SetImage(entity, {...});
int entity_SetImage(lua_State* ls)
{
	PRINT("entity_SetImage");
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
	PRINT("entity_SetAvatar");
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
	{"Remove", entity_Remove},
	{"RemoveAllById", entity_RemoveAllById},
	{"NewTextObject", entity_NewTextObject},
	{"Create", entity_Create},
	{"SetImage", entity_SetImage},
	{"SetAvatar", entity_SetAvatar},
	{"Explode", entity_Explode},
	{NULL, NULL}
};

void RegisterEntityLib(lua_State* ls)
{
	luaL_register( ls, "Entity", functions );
}
