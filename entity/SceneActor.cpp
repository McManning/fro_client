
#include "SceneActor.h"
#include "../game/GameManager.h"

SceneActor::SceneActor()
	: Actor()
{
	mType = ENTITY_SCENEACTOR;
}

SceneActor::~SceneActor()
{
	PRINT("~SceneActor");
}

int SceneActor::ReadXml(XmlFile* xf, TiXmlElement* e, bool online)
{
	string id = e->Value();
	
	PRINT("SceneActor::ReadXml " + id);
	
	string s1, s2;
	bool b1, b2;
	uShort u1, u2, u3;
	
	//<avatar file="blah" md5="hash" pass="$" w="0" h="0" loopstand="0" loopsit="0" delay="1000" />
	if (id == "avatar")
	{
		/*if (online)
		{
			s1 = string(DIR_CACHE) + DIR_ENTITIES + xf->GetParamString(e, "file") 
					+ "." + xf->GetParamString(e, "ver");
		}
		else
		{
			s1 = DIR_ENTITIES + xf->GetParamString(e, "file");
		}*/
		
		if (online)
			s1 = DIR_CACHE;
		else
			s1 = DIR_EDITOR;
		
		s1 += DIR_ENTITIES + xf->GetParamString(e, "file");
		
		s2 = xf->GetParamString(e, "pass");
		u1 = (xf->ParamExists(e, "w")) ? xf->GetParamInt(e, "w") : 0;
		u2 = (xf->ParamExists(e, "h")) ? xf->GetParamInt(e, "h") : 0;
		u3 = (xf->ParamExists(e, "delay")) ? xf->GetParamInt(e, "delay") : 1000;
		b1 = xf->GetParamInt(e, "loopstand");
		b2 = xf->GetParamInt(e, "loopsit");
		
		if (!LoadAvatar(s1, s2, u1, u2, u3, b1, b2))
		{
			return XMLPARSE_CANCEL;
		}
	}
	else //Unidentified, let the base handle it
	{
		return Actor::ReadXml(xf, e, online);
	}
	
	PRINT("SceneActor::ReadXml " + id + " End");
	
	return XMLPARSE_SUCCESS;
}

