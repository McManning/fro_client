
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
	
	string GetID() const { return mId; };
	
	/*	Construct a resulting encrypted and compressed message from the data to be sent out */
	string ToString() const;
	
	/*	Returns true if decrypted properly, false if the input string wasn't a valid message */
	bool FromString(string& s);
	
	/*	Returns true if we're at the end of the packet (nothing else to read) */
	bool End() const;
	
	void WriteString(string data);
	void WriteInt(int data);
	void WriteChar(char data);
	
	/*	Read data in and increase the current index */
	string ReadString();
	int ReadInt();
	char ReadChar();
	
	/*	Encryption stuff */
	void SetKey(string s) { mKey = s; };
	string GetKey() const { return mKey; };
	
	string mId;
	string mKey;
	vString mData;
	
  private:
	int mReadIndex; // Current index to read from
};

#endif //_DATAPACKET_H_
