
#include "DataPacket.h"
#include "../io/Crypt.h"
#include "IrcNet2.h"

DataPacket::DataPacket(string id)
{
	mId = id;
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

void DataPacket::WriteString(string data)
{
	mData.push_back(data);
}

void DataPacket::WriteInt(int data)
{
	mData.push_back(its(data));
}

string DataPacket::ReadString(int index) const
{
	if (mData.size() <= index)
	{
		FATAL(mId + " Invalid Index: " + its(index));
	}
	
	return mData.at(index);
}

int DataPacket::ReadInt(int index) const
{
	if (mData.size() <= index)
	{
		FATAL(mId + " Invalid Index: " + its(index));
	}
	
	return sti(mData.at(index));
}

