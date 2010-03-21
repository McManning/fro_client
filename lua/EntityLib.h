
#ifndef _LUA_ENTITYLIB_H_
#define _LUA_ENTITYLIB_H_

class lua_State;
void RegisterEntityLib(lua_State*);

class Entity;
bool _verifyEntity(Entity* e);

#endif //_LUA_ENTITYLIB_H_
