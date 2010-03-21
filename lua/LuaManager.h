
#ifndef _LUAMANAGER_H_
#define _LUAMANAGER_H_

#include "../core/Core.h"

class lua_State;
class LuaManager
{
  public:
	LuaManager();
	~LuaManager();
	
	/*	Returns true if it successfully loads the lua file. 
		False otherwise 
	*/
	bool Load(string file);

	/*	Calls OnLoad()/OnUnload() function in the lua file. If not found, returns -1 and sets mError.
		Else, returns the return value of that function.
	*/
	int OnLoad();
	int OnUnload();
	
	/*	Sets up this LuaManager based on information in the map xml */
	int ReadXml(XmlFile* xf, TiXmlElement* e, bool online);
	
	string GetError() const { return mError; };
	
	lua_State* mState;
	string mError;
	string mFilename;

};

#endif //_LUAMANAGER_H_
