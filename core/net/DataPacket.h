
#ifndef _DATAPACKET_H_
#define _DATAPACKET_H_

#include "../Common.h"

/*	Message that will be sent and recieved over the network 
	(For custom data, NOT IRC messages)
	Turned into a seperate class so that it can be improved upon 
	in the future.
*/
class DataPacket
{
  public:
	DataPacket(string id = "");
	~DataPacket() {};
	
	/*	Construct a resulting encrypted and compressed message from the data to be sent out */
	string ToString() const;
	
	/*	Returns true if decrypted properly, false if the input string wasn't a valid message */
	bool FromString(string& s);
	
	int Size() const { return mData.size(); };
	
	void WriteString(string data);
	void WriteInt(int data);
	
	/*	Read data from index */
	string ReadString(int index) const;
	int ReadInt(int index) const;
	
	/*	Encryption stuff */
	void SetKey(string s) { mKey = s; };
	string GetKey() const { return mKey; };
	
	string mId;
	string mKey;
	vString mData;
};

#endif //_DATAPACKET_H_
