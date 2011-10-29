
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


/*	A more flexible message system. Filled by various classes, and dispatched by MessageManager */

#ifndef _MESSAGEDATA_H_
#define _MESSAGEDATA_H_

#include <map>
#include "Common.h"

typedef enum 
{
	STRING = 0,
	INTEGER,
	USERDATA
} messageItemType;

class MessageData
{
  public:
	MessageData(string id = "");
	~MessageData();
	
	void WriteString(string key, string value);
	void WriteInt(string key, int value);
	void WriteUserdata(string key, void* value);

	string ReadString(string key);
	int ReadInt(string key);
	void* ReadUserdata(string key);

	void Clear() { mData.clear(); };
	void SetId(string id) { mId = id; };

	struct messageItem
	{
		messageItemType type;
		void* u;
		int i;
		string s;
	};

	std::map<string, messageItem> mData;
	string mId;
};

#endif //_MESSAGEDATA_H_


