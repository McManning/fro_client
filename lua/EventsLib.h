
#ifndef _LUA_EVENTSLIB_H_
#define _LUA_EVENTSLIB_H_

class lua_State;
void RegisterEventsLib(lua_State*);

void unregisterAllEventListeners(lua_State* ls);

#endif //_LUA_EVENTSLIB_H_
