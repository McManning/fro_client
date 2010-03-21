
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


