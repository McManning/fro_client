
/*
 * Copyright (c) 2011 Chase McManning
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */


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
