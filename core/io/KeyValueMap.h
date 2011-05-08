
#ifndef _KEYVALUEMAP_H_
#define _KEYVALUEMAP_H_

#include "../Common.h"

struct lua_State;
class KeyValueMap
{
  public:
	KeyValueMap();
	~KeyValueMap();

	int Load(string file, bool useEncryption);
	int Save();

	string GetValue(string section, string key);
	int SetValue(string section, string key, string value);
	
	string mFilename;
	lua_State* mLuaState;
	bool mUseEncryption;
};

#endif //_KEYVALUEMAP_H_


