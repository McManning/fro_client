
#include "XmlFile.h"

XmlFile config;

XmlFile::XmlFile()
{
	mParseCallback = NULL;
	mSkipChildren = false;
	mAutoSave = false;
}

XmlFile::~XmlFile() 
{
	if (mAutoSave)
		SaveToFile();
}

bool XmlFile::LoadFromFile(string fileName)
{
	if(mDoc.LoadFile(fileName.c_str()) == false)
	{
		mErrorText = mDoc.ErrorDesc();
		return false;
	}
	return true;
}

bool XmlFile::LoadFromMemory(const char* buff, int size)
{
	if (!buff || size < 1)
		return false;
	
	string hack(buff, size);
	printf(hack.c_str());
	// ^ hack for the below todo, lolol

	//TODO: Currently, buff must be null terminated and size isn't used.
	//need to add a check to ensure null is @ the end.
	if(mDoc.Parse(hack.c_str()) == NULL)
	{
		mErrorText = mDoc.ErrorDesc();
		return false;
	}
	return true;
}

bool XmlFile::SaveToFile(string filename)
{
	if (filename.empty())
		return mDoc.SaveFile();
	else
		return mDoc.SaveFile(filename.c_str());
}

int XmlFile::Parse(void* userData)
{
	if (mEntryPoint.empty())
		mXmlPos = mDoc.FirstChildElement();
	else
		mXmlPos = mDoc.FirstChildElement(mEntryPoint.c_str());

	return ParseElement(mXmlPos, userData);
}

bool XmlFile::SetEntryPoint(string element)
{
	mEntryPoint = element; 
	if (!element.empty())
		return (mDoc.FirstChildElement(element.c_str()) != NULL);
	else
		return true;
}

//Only parse element siblings @ the current level
int XmlFile::ParseSiblingsOnly(TiXmlElement* element, void* userData)
{
	if (!element) return XMLPARSE_CANCEL;

	while (element)
	{
		if (mParseCallback(this, element, userData) == XMLPARSE_CANCEL)
			return XMLPARSE_CANCEL;
		element = element->NextSiblingElement(); //go to next element
	}
	return XMLPARSE_SUCCESS;
}

//TODO: less recursive version of these two to prevent stack overflow
int XmlFile::ParseElement(TiXmlElement* element, void* userData)
{
	if (!element) return XMLPARSE_CANCEL;

	mXmlPos = element;

	//call the handler for this document if it exists
	if (mParseCallback)
	{
		if (mParseCallback(this, element, userData) == XMLPARSE_CANCEL)
			return XMLPARSE_CANCEL;
	}

	if (!mSkipChildren)
		return ParseElementChildren(element, userData); //recurse through children
	else
		mSkipChildren = false; //reset for the next sibling

	return XMLPARSE_SUCCESS;
}

int XmlFile::ParseElementChildren(TiXmlElement* element, void* userData)
{
	if (!element) return XMLPARSE_CANCEL;

	TiXmlElement* childElement = element->FirstChildElement();
	while (childElement)
	{
		if (ParseElement(childElement, userData) == XMLPARSE_CANCEL)
			return XMLPARSE_CANCEL;
		childElement = childElement->NextSiblingElement(); //go to next element
	}
	return XMLPARSE_SUCCESS;
}

string XmlFile::GetText(TiXmlElement* element)
{
	if (!element) return "";

	//Combines all text type children and returns. Ignores the rest.
	TiXmlNode* child = element->FirstChild();
	string s;
	while (child)
	{
		if (child->Type() == TiXmlNode::TEXT)
		{
			s += child->Value();
		}
		child = child->NextSibling();
	}
	return s;
}

string XmlFile::GetParamString(TiXmlElement* element, string param)
{
	if (!element) return "";

	const char* s = element->Attribute(param.c_str());

	return (s) ? s : "";
}

int XmlFile::GetParamInt(TiXmlElement* element, string param)
{
	if (!element) return 0;

	const char* s = element->Attribute(param.c_str());

	return (s) ? readFormattedNumber(s) : 0;
}

double XmlFile::GetParamDouble(TiXmlElement* element, string param)
{
	if (!element) return 0.0;
	
	return stod(GetParamString(element, param));
}

void XmlFile::SetParamString(TiXmlElement* element, string param, string val)
{
	if (!element) return;
	element->SetAttribute(param.c_str(), val.c_str());
}

void XmlFile::SetParamInt(TiXmlElement* element, string param, int val)
{
	if (!element) return;
	element->SetAttribute(param.c_str(), val);
}

void XmlFile::SetParamDouble(TiXmlElement* element, string param, double val)
{
	SetParamString(element, param, dts(val));	
}

void XmlFile::SetText(TiXmlElement* element, string text)
{
	if (!element) return;
	TiXmlNode* child = element->FirstChild();

	//TODO: Add if it doesn't exist?
	if (!child || child->Type() != TiXmlNode::TEXT)
		AddChildText(element, text);
	else
		child->SetValue(text.c_str());
}

TiXmlElement* XmlFile::AddChildElement(TiXmlElement* element, string id)
{
	if (!element) return NULL;

	TiXmlElement* child = new TiXmlElement(id.c_str());
	element->LinkEndChild(child); //add it

	return child;
}

TiXmlText* XmlFile::AddChildText(TiXmlElement* element, string msg)
{
	if (!element) return NULL;

	TiXmlText* text = new TiXmlText(msg.c_str());
	element->LinkEndChild(text);

	return text;
}

TiXmlElement* XmlFile::GetChild(TiXmlElement* element, string id)
{
	if (!element && id.empty()) return mDoc.FirstChildElement();
	if (!element) return mDoc.FirstChildElement(id.c_str());
	if (id.empty()) return element->FirstChildElement();
	return element->FirstChildElement(id.c_str());
}

void XmlFile::RemoveChild(TiXmlElement* element)
{
	element->Parent()->RemoveChild(element);
}

//Shorthand functions to manipulate shit easier at the current level
void XmlFile::SetParamString(string element, string param, string val) {
	SetParamString(GetChild(mXmlPos, element), param, val);
}

void XmlFile::SetParamInt(string element, string param, int val) {
	SetParamInt(GetChild(mXmlPos, element), param, val);
}

void XmlFile::SetParamDouble(string element, string param, double val) {
	SetParamDouble(GetChild(mXmlPos, element), param, val);
}

void XmlFile::SetText(string element, string text) {
	SetText(GetChild(mXmlPos, element), text);
}

string XmlFile::GetParamString(string element, string param) {
	return GetParamString(GetChild(mXmlPos, element), param);
}

int XmlFile::GetParamInt(string element, string param) {
	return GetParamInt(GetChild(mXmlPos, element), param);
}

double XmlFile::GetParamDouble(string element, string param) {
	return GetParamDouble(GetChild(mXmlPos, element), param);
}

string XmlFile::GetText(string element) {
	return GetText(GetChild(mXmlPos, element));
}

//Special Read/write functions~
void XmlFile::SetParamColor(TiXmlElement* element, string param, color val) { // x="R,G,B,A"
	string s = its(val.r) + "," + its(val.g) + "," + its(val.b) + "," + its(val.a);
	SetParamString(element, param, s);
}

color XmlFile::GetParamColor(TiXmlElement* element, string param) { // x="R,G,B,A"
	string s = GetParamString(element, param);
	if (s.empty()) return color();

	vString data;
	explode(&data, &s, ",");
	color c;
	c.r = (data.size() > 0) ? sti(data.at(0)) : 255;
	c.g = (data.size() > 1) ? sti(data.at(1)) : 255;
	c.b = (data.size() > 2) ? sti(data.at(2)) : 255;
	c.a = (data.size() > 3) ? sti(data.at(3)) : ALPHA_OPAQUE;
	return c;
}

void XmlFile::SetParamRect(TiXmlElement* element, string param, rect val) // x="X,Y,W,H"
{ 
	string s = its(val.x) + "," + its(val.y) + "," + its(val.w) + "," + its(val.h);
	SetParamString(element, param, s);
}

rect XmlFile::GetParamRect(TiXmlElement* element, string param) // x="X,Y,W,H"
{ 
	string s = GetParamString(element, param);
	if (s.empty()) return rect();

	vString data;
	explode(&data, &s, ",");
	rect r;
	r.x = (data.size() > 0) ? readFormattedNumber(data.at(0)) : 0;
	r.y = (data.size() > 1) ? readFormattedNumber(data.at(1)) : 0;
	r.w = (data.size() > 2) ? readFormattedNumber(data.at(2)) : 0;
	r.h = (data.size() > 3) ? readFormattedNumber(data.at(3)) : 0;
	return r;
}

void XmlFile::SetParamPoint2d(TiXmlElement* element, string param, point2d val) // x="X,Y"
{ 
	string s = its(val.x) + "," + its(val.y);
	SetParamString(element, param, s);
}

point2d XmlFile::GetParamPoint2d(TiXmlElement* element, string param) // x="X,Y"
{ 
	string s = GetParamString(element, param);
	if (s.empty()) return point2d(0, 0);

	vString data;
	explode(&data, &s, ",");
	point2d v;
	v.x = (data.size() > 0) ? readFormattedNumber(data.at(0)) : 0;
	v.y = (data.size() > 1) ? readFormattedNumber(data.at(1)) : 0;
	return v;
}

void XmlFile::SetParamDirection(TiXmlElement* element, string param, direction val) 
{
	string s = directionToString(val);
	SetParamString(element, param, s);
}

direction XmlFile::GetParamDirection(TiXmlElement* element, string param) 
{
	string s = GetParamString(element, param);
	return stringToDirection(s);
}

bool XmlFile::ParamExists(TiXmlElement* element, string param)
{
	if (!element)
		return false;
	else
		return (element->Attribute(param.c_str()) != NULL);
}


