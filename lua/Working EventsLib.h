
#ifndef _LUA_EVENTSLIB_H_
#define _LUA_EVENTSLIB_H_

class lua_State;
void RegisterEventsLib(lua_State*);

/*	Unload all listeners with matching lua_State from messenger */
void unregisterAllEventListeners(lua_State* ls);

#endif //_LUA_EVENTSLIB_H_
