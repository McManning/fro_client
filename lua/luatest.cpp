
#include <string>
#include <map>
#include <lua.hpp> //deals with all the extern "C" crap for us
using namespace std;


class Entity
{
  public:
	Entity() { };
	~Entity() { };	
	
	map<string, string> mFlags;
};

Entity ent;

int luaEntity_GetFlag(lua_State* ls)
{
	printf("GetFlag\n");
	int numArgs = lua_gettop( ls );
	
	if (numArgs < 2)
		return 0;

	Entity* e = (Entity*)lua_touserdata(ls, 1);
	if (!e)
	{
		lua_pushstring( ls, "[entity.getflag] First Parameter not an entity!" );
		lua_error( ls );
		return 0;
	}

	string s;
	s = e->mFlags[lua_tostring(ls, 2)];
	if (s.empty())
	{
		printf("Flag No Existo\n");	
		lua_pushstring(ls, "");
	}
	else
	{
		printf("Returning flag: %s\n", s.c_str());
		lua_pushstring(ls, s.c_str());
	}

	return 1;
}

int luaEntity_Find(lua_State* ls)
{
	lua_pushlightuserdata(ls, (void*)&ent);	
	return 1;
}

int luaEntity_PrintFlags(lua_State* ls)
{
	Entity* e = (Entity*)lua_touserdata(ls, 1);
	if (!e)
	{
		lua_pushstring( ls, "[entity.printflags] First Parameter not an entity!" );
		lua_error( ls );
		return 0;
	}
	
	map<string, string>::iterator iter;
	for ( iter = e->mFlags.begin(); iter != e->mFlags.end(); ++iter ) 
	{
		printf("[%s]->%s\n", (*iter).first.c_str(), (*iter).second.c_str());
	}
	
	return 0;
}

int luaEntity_SetFlag(lua_State* ls)
{
	printf("SetFlag\n");
	int numArgs = lua_gettop( ls );
	
	if (numArgs < 3)
	{
		lua_pushstring( ls, "[entity.setflag] Invalid Parameters" );
		lua_error( ls );
		return 0;
	}

	Entity* e = (Entity*)lua_touserdata(ls, 1);
	if (!e)
	{
		lua_pushstring( ls, "[entity.setflag] First Parameter not an entity!" );
		lua_error( ls );
		return 0;
	}

	char* c = (char*)lua_tostring(ls, 2);
	e->mFlags[c] = lua_tostring(ls, 3);
	printf("Set flag: %s to %s!\n", c, e->mFlags[c].c_str());

	return 0;	
}

static const luaL_Reg entitylib[] = {
  {"getflag", luaEntity_GetFlag},
  {"setflag", luaEntity_SetFlag},
  {"find", luaEntity_Find},
  {"printflags", luaEntity_PrintFlags},
  {NULL, NULL}
};

int luaFoo( lua_State *ls )
{
	int numArgs = lua_gettop( ls ); //# of arguments passed	

	if (numArgs < 2)
	{
		lua_pushstring( ls, "[foo] Needs two arguments!" );
		lua_error( ls );
	}
	
	double d = lua_tonumber( ls, 1 ); //argument 1
	double dd = lua_tonumber( ls, 2 ); //argument 2 
	
	lua_pushnumber( ls, d * dd ); //push a result of the function onto the stack
	return 1; // Return the number of results
}

void callLuaFunction( lua_State* ls, string id, string msg )
{
	lua_getglobal(ls, id.c_str()); //get function name

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(ls, -1)) 
	{
		printf("Could not find function %s\n", id.c_str());
		return;	
	}

	lua_pushstring(ls, msg.c_str()); //first argument

	lua_call(ls, 1, 0); //lua_state, # params passed, # params to return

	//int result = (int)lua_tointeger(ls, -1); //get the result
	//lua_pop(ls, 1); //get rid of result from stack

	//return result;
}

int main()
{
	printf("\tSup\n\n");
	
	lua_State *ls = luaL_newstate();
	luaL_openlibs( ls );  

	lua_register( ls, "foo", luaFoo );
	luaL_register( ls, "entity", entitylib );
	
	// Run a test script to exercise our new functions
	if (luaL_dofile( ls, "test.lua" ) != 0) //luaL_loadfile( ls, "test.lua") to not auto run code
		printf("\n\tERROR [%s]\n", lua_tostring( ls, -1 )); //output the error message @ the top of the stack
	else
		printf("\n\tDid file!\n");

	callLuaFunction( ls, "callmefromcpp", "Sup Lua" ); //lua is case sensitive. Will fail~
	callLuaFunction( ls, "callMeFromCpp", "Sup Lua" );
	
	lua_close( ls ); //close the state	
	
	printf("\tEnter to exit (ironnyyy)\n");
	getchar();
	return 0;	
}
