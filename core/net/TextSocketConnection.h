
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

