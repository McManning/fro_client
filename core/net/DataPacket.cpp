
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


