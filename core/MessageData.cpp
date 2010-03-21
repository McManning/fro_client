
#include "MessageData.h"

MessageData::MessageData(string id)
{
	mId = id;
}

MessageData::~MessageData()
{

}

void MessageData::WriteString(string key, string value)
{
	messageItem d;
	d.type = STRING;
	d.s = value;
	mData[key] = d;
}

void MessageData::WriteInt(string key, int value)
{
	messageItem d;
	d.type = INTEGER;
	d.i = value;
	mData[key] = d;
}

void MessageData::WriteUserdata(string key, void* value)
{
	messageItem d;
	d.type = USERDATA;
	d.u = value;
	mData[key] = d;
}

string MessageData::ReadString(string key)
{
	messageItem* d = &mData[key];
	if (d->type != STRING)
		FATAL("MessageData " + mId + " member " + key + " not string");
	
	return d->s;
}

int MessageData::ReadInt(string key)
{
	messageItem* d = &mData[key];
	if (d->type != INTEGER)
		FATAL("MessageData " + mId + " member " + key + " not int");
	
	return d->i;
}

void* MessageData::ReadUserdata(string key)
{
	messageItem* d = &mData[key];
	if (d->type != USERDATA)
		FATAL("MessageData " + mId + " member " + key + " not userdata");
	
	return d->u;
}
