
#ifndef _LUA_ACTORLIB_H_
#define _LUA_ACTORLIB_H_

class lua_State;
void RegisterActorLib(lua_State*);

class Actor;
Actor* getReferencedActor(lua_State* ls, int index = 1);

#endif //_LUA_ACTORLIB_H_
