
#ifndef _LUA_TIMERSLIB_H_
#define _LUA_TIMERSLIB_H_

class lua_State;
void RegisterTimersLib(lua_State*);

/*	Unload all timers with matching lua_State from TimerManager */
void unregisterAllLuaTimers(lua_State* ls);

#endif //_LUA_TIMERSLIB_H_
