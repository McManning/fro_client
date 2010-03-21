
#ifndef _XMLFILE_H_
#define _XMLFILE_H_

#include "../Common.h"
#include "tinyxml/tinyxml.h"

enum
{
	XMLPARSE_SUCCESS = 0,
	XMLPARSE_CANCEL
};

//Wrapper for TinyXML make certain often-used-functions easier
class XmlFile 
{
  public:
	XmlFile();
	~XmlFile();

	bool LoadFromFile(string fileName);
	bool LoadFromMemory(const char* buff, int size);
	bool SaveToFile(string filename = "");

	int Parse(void* userData);

	//Returns whichever error tinyXML throws
	string GetError() { return mErrorText; };

	//set a function that parses the individual elements
	//(allows this class to be used for nearly anything)
	void SetParser(int (*func)(XmlFile*, TiXmlElement*, void* userData)) { mParseCallback = func; };

	//Starts parsing the file from this entry point. If "", starts @ topmost element.
	//Returns false if the designated entry point is not found, true otherwise (also true if blank)
	bool SetEntryPoint(string element = "");

	int ParseSiblingsOnly(TiXmlElement* element, void* userData);

	//Calls the parser callback on the current element
	int ParseElement(TiXmlElement* element, void* userData);

	/*Runs through all children of element:
		<element>
			<child></child>
			<child />
		</element>
	*/
	int ParseElementChildren(TiXmlElement* element, void* userData);

	//call to ignore children after current element is processed
	void SkipChildren() { mSkipChildren = true; };

	//Gets attribute/param data: <element param="Returns This"/>
	string GetParamString(TiXmlElement* element, string param);
	int GetParamInt(TiXmlElement* element, string param);
	double GetParamDouble(TiXmlElement* element, string param);
	string GetText(TiXmlElement* element); //<element>Returns This</element>

	void SetParamString(TiXmlElement* element, string param, string val);
	void SetParamInt(TiXmlElement* element, string param, int val);
	void SetParamDouble(TiXmlElement* element, string param, double val);
	void SetText(TiXmlElement* element, string text);

	//Shorthand functions to manipulate shit easier at the current level
	void SetParamString(string element, string param, string val);
	void SetParamInt(string element, string param, int val);
	void SetParamDouble(string element, string param, double val);
	void SetText(string element, string text);

	string GetParamString(string element, string param);
	int GetParamInt(string element, string param);
	double GetParamDouble(string element, string param);
	string GetText(string element);

	//Add in children objects
	TiXmlElement* AddChildElement(TiXmlElement* element, string id);
	TiXmlText* AddChildText(TiXmlElement* element, string msg);

	//Return first child with the specified id (element)
	TiXmlElement* GetChild(TiXmlElement* element, string id);
	void RemoveChild(TiXmlElement* element);

	//Special Read/write functions~
	void SetParamColor(TiXmlElement* element, string param, color val);
	color GetParamColor(TiXmlElement* element, string param);

	void SetParamRect(TiXmlElement* element, string param, rect val);
	rect GetParamRect(TiXmlElement* element, string param);

	void SetParamPoint2d(TiXmlElement* element, string param, point2d val);
	point2d GetParamPoint2d(TiXmlElement* element, string param);

	void SetParamDirection(TiXmlElement* element, string param, direction val);
	direction GetParamDirection(TiXmlElement* element, string param);

	//Test if the parameter exists
	bool ParamExists(TiXmlElement* element, string param);

	TiXmlDocument mDoc; //loaded document
	TiXmlElement* mXmlPos; //Where we currently are in the xml tree. Set by parse_element

	bool mAutoSave; //if true, will save the xml file to disk when unloaded.

private:
	string mErrorText;
	bool mSkipChildren;
	string mEntryPoint;

	int (*mParseCallback)(XmlFile*, TiXmlElement*, void* userData);
};

extern XmlFile config;

#endif //_XMLFILE_H_
