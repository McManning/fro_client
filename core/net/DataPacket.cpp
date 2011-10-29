
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


#include "DataPacket.h"
#include "../io/Crypt.h"
#include "IrcNet2.h"

DataPacket::DataPacket(string id)
{
	mId = id;
	mReadIndex = 0;
}

string DataPacket::ToString() const
{
	string result = mId;
	
	for (int i = 0; i < mData.size(); i++)
		result += SEPERATOR + mData.at(i);

	return encryptString( result, mKey );
}

bool DataPacket::FromString(string& s)
{
	mData.clear();
	
	string decrypted = decryptString( s, mKey );

	explode(&mData, &decrypted, SEPERATOR, true);
	
	if (mData.empty()) //malformed
		return false;

	mId = mData.at(0);
	mData.erase(mData.begin());
	
	return true;
}

bool DataPacket::End() const
{
	return mReadIndex == mData.size();
}

void DataPacket::WriteString(string data)
{
	mData.push_back(data);
}

void DataPacket::WriteInt(int data)
{
	mData.push_back(its(data));
}

void DataPacket::WriteChar(char data)
{
	mData.push_back(its(data));
}

string DataPacket::ReadString()
{
	if (End()) return "";
	
	string s = mData.at(mReadIndex);
	++mReadIndex;
	return s;
}

int DataPacket::ReadInt()
{
	if (End()) return 0;
		
	string s = mData.at(mReadIndex);
	++mReadIndex;
	return sti(s);
}

char DataPacket::ReadChar()
{
	if (End()) return 0;
		
	string s = mData.at(mReadIndex);
	++mReadIndex;
	return (char)sti(s); //lolol bad temp coding
}


