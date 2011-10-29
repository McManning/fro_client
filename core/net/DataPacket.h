
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
