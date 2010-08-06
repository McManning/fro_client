
#ifndef _LUNEM_H_
#define _LUNEM_H_

#include "Actor.h"

class Lunem : public Actor
{
  public:
	Lunem();
	~Lunem();

	/*	Will set mSpecies and attempt to (down)load a new image associated
		to the specific species
	*/
	void SetSpecies(string s);
	
	/*	index - Index of the stack where our new value for the property should be */
	int LuaSetProp(lua_State* ls, string& prop, int index);
	int LuaGetProp(lua_State* ls, string& prop);

	void ReadFromFile(FILE* f);
	void WriteToFile(FILE* f) const;

	string m_sSpecies;
};

#endif //_LUNEM_H_
