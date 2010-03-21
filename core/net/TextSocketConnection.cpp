
#include "TextSocketConnection.h"

TextSocketConnection::TextSocketConnection() 
{
	mConnected = false;
	mRecvRest = "";
	mServerAddrIn = new sockaddr_in;
	mClientSocket = new SOCKET;
	mMaxMessageSize = 0;
}

TextSocketConnection::~TextSocketConnection() 
{
	Disconnect();

	delete mClientSocket;
	delete mServerAddrIn;

	for (int i = 0; i < mRecvLineList.size(); i++)
		delete mRecvLineList.at(i);
	mRecvLineList.clear();
}

void TextSocketConnection::SetDestination(string s, int p) 
{
	mHost = s;
	mPort = p;
}

void TextSocketConnection::Connect() 
{
	Disconnect();

	//TODO: Do I need to erase the hostname? 
	hostent* hostname = gethostbyname(mHost.c_str());
	if (hostname == NULL) {
		mConnected = false;
		return;
	}
	mServerAddrIn->sin_family = PF_INET;
	mServerAddrIn->sin_port = htons (mPort);
	mServerAddrIn->sin_addr = *(struct in_addr*)hostname->h_addr;

	*mClientSocket = socket(PF_INET, SOCK_STREAM, 0);

	mConnected = (::connect(*mClientSocket, (sockaddr*)mServerAddrIn, 
								sizeof(*mServerAddrIn)) != SOCKET_ERROR);
}

void TextSocketConnection::Disconnect() 
{
#ifdef WIN32
	shutdown(*mClientSocket, SD_BOTH); //stops send/recv
	closesocket(*mClientSocket); //closes socket itself
#else
	close(*mClientSocket);
#endif
	mConnected = false;
	
	//clean up messages so they aren't processed next time
	for (int i = 0; i < mRecvLineList.size(); i++)
		delete mRecvLineList.at(i);

	mRecvLineList.clear();
}

bool TextSocketConnection::_hasNewData() 
{
	timeval t;
	t.tv_sec = 0;
	t.tv_usec = 100;

	fd_set rfds;
	FD_ZERO (&rfds);
	FD_SET (*mClientSocket, &rfds);

	if ( select(*mClientSocket + 1, &rfds, NULL, NULL, &t) == -1 )
	{
		WARNING("Socket Ret -1");
		return false;
	}
	return FD_ISSET(*mClientSocket, &rfds);
}

void TextSocketConnection::_recvData() 
{
	if (!_hasNewData()) return;
	
	int bytes;
	char buffer[SOCKET_BUFFER_SIZE];

	string data = mRecvRest;
	
	//keep receiving until we got nothing new or the server terminates
	do {
		bytes = recv(*mClientSocket, buffer, sizeof(buffer) - 1, 0);
		
		buffer[bytes] = '\0'; //So we don't add trash data from the buffer to the string. 
		
		if (bytes == -1 || bytes == 0)  //server terminated connection while receiving
		{
			mConnected = false;
			break;
		}
		
		data += buffer;
	} while (_hasNewData());

	//Split our data up by lines
	int i = 0;
	while (data.find ('\n', i) != string::npos)
	{
		string* line = new string();
		*line = data.substr(i, data.find('\n', i) - i - 1); //no \r\n
		mRecvLineList.push_back(line);
		i = data.find('\n', i) + 1;
	}
	
	//leave incomplete lines in the buffer
	mRecvRest = data.substr(i);
}

bool TextSocketConnection::CanReadLine() 
{
	_recvData();
	return !mRecvLineList.empty(); //if we got shit~
}

string TextSocketConnection::GetLine() 
{
	if (CanReadLine()) 
	{
		string line = *(mRecvLineList.at(0));
		
		delete mRecvLineList.at(0);
		mRecvLineList.erase(mRecvLineList.begin());
		
		return line; 
	} 
	else 
	{
		return mRecvRest;
	}
}

bool TextSocketConnection::SendLine(string& data) 
{
	if (!IsConnected()) return false;
	
	//Can't have more than the defined max
	if (data.size() > mMaxMessageSize && mMaxMessageSize > 0) 
	{
		WARNING("Clipping Outbound Message: " + data);
		data.erase(data.begin() + mMaxMessageSize);
	}
	
	return ( send(*mClientSocket, data.c_str(), data.size(), 0) != -1 );
}

