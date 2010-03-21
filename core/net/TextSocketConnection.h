
/*	TextSocketConnection class:
		Simplified string-based communication protocol. Each packet is ascii, seperated by \n or \r\n.
		Does NOT initialize or shut down Winsocks, that must be done externally.

*/
#ifndef _TEXTSOCKETCONNECTION_H_
#define _TEXTSOCKETCONNECTION_H_

#include "../../core/Core.h"

#ifdef WIN32
	#include <winsock2.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/select.h>
	typedef int SOCKET;
	#define SOCKET_ERROR (-1)
	#define INVALID_SOCKET (-1)
#endif

const int SOCKET_BUFFER_SIZE = (1024*10);

struct sockaddr_in;

class TextSocketConnection
{
  public:
	TextSocketConnection();
	virtual ~TextSocketConnection();

	virtual void Connect();
	virtual void Disconnect();
	
	//TODO: Rewrite this to query the socket state itself
	bool IsConnected() { return mConnected; };

	bool CanReadLine();
	string GetLine();

	//Return false if we're not connected or an error occured
	bool SendLine(string& data);
	void SetDestination(string s, int p);

	string mHost;
	int mPort;

	int mMaxMessageSize;

  private:
	void _recvData();
	bool _hasNewData();

	bool mConnected;
	std::vector<string*> mRecvLineList;
	string mRecvRest;
	sockaddr_in* mServerAddrIn;

	SOCKET* mClientSocket;
};

#endif //_TEXTSOCKETCONNECTION_H_

