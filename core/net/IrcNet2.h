/*	Messages sent by IrcNet to MessageManager (messenger)

	ID			Tokens (seperated by commas)

	NET_NEWSTATE	New connectionState
	NET_FAILED		(Could not connect to server)
	NET_ACTION		sender nick, sender address, target, message
	NET_PRIVMSG		sender nick, sender address, target, message
	NET_JOIN		joiner nick, joiner address
	NET_KICK			kicked nick, kicker nick, reason
	NET_NICK			old nick, new nick
	NET_PART			parter nick, parter address
	NET_MODE		server address or nick, mode set
	NET_TOPIC		channel, nick who sent it (or blank if server), new topic
	NET_QUIT		parter nick, parter address, reason
	NET_NOTICE		sender nick, notice message
	NET_NICKINUSE	message ( Nickname already in use / Erronous Nickname )
	NET_VERIFIED    (Sent when server sends 001 message)
	NET_USERLIST		nicks (seperated by commas)
	NET_ONCHANNEL	channel id
	NET_JOINING		channel id (sent when we're attempting to join a channel, but not necessarily on it)
	NET_MOTD		message
	NET_CMDERROR	command, error message
	NET_UNKNOWN	command, full packet string
	NET_ERROR		error message
	NET_PING			server that sent the ping
	NET_TIMEOUT 	(Sent when CONNECTING or AWAITINGVERIFY states time out)
	
	
	Reference Material
		http://www.mirc.com/help/rfc2812.txt
*/

#ifndef _IRCNET2_H_
#define _IRCNET2_H_

#include "../Core.h"
#include "TextSocketConnection.h"

const char* const SEPERATOR = "\x01";
const char* const SECONDARY_PACKET_PASS = "DivineRightToRule";

const int NET_TIMEOUT_SECONDS = 60;
const int PING_INTERVAL_SECONDS = 60;

/* Channels were initially a container that held lists of members
	and all those fancy user control functions. However,
	since the game manages users w/ its entity manager,
	there's no reason for it. So we've downgraded
*/
class IrcChannel
{
  public:
	IrcChannel()
	{
		mSuccess = false;
	};
	
	string mId;
	string mTopic;
	string mPassword;
	string mEncryptionKey;
	
	bool mSuccess;
};	

enum connectionState 
{
	DISCONNECTED = 0, 
	COULDNOTCONNECT, //error occured while connecting
	CONNECTING, //if our thread is currently negotiating a connection
	CONNECTED, //transfers to AWAITINGSERVERVERIFY once OnConnect is called.
	AWAITINGSERVERVERIFY, //Here until we receive 001 welcome
	VERIFYING, //Currently in the middle of negotiating a verified connection, this usually takes a few seconds
	ONSERVER, //Post 001 welcome, or were kicked from a channel
	ONCHANNEL //Once we get on a channel
};

class SDL_Thread;
class IrcNet : public TextSocketConnection
{
  public:
	IrcNet();
	~IrcNet();

	//Returns false if we're not connected, true otherwise.
	bool Process();

	void OnConnect();

	//Returns true if already connected. 
	//False if the address is invalid or it's trying to connect in another thread.
	bool ConnectToServer(string address);
	bool TryNextServer();
	
	//Ran from the connection thread.
	void Connect();

	connectionState GetState() const { return mState; };
		
	void PingServer();
		
	//CHANNEL RELATED
	IrcChannel* CreateChannel(string chan, string pass = "");
	void JoinChannel(IrcChannel* chan);
	void JoinChannel(string chan, string pass = "");
	IrcChannel* GetChannel() const { return mChannel; };
	void PartChannel(IrcChannel* chan = NULL);
	void SetTopic(IrcChannel* chan, string newTopic = "");
	
	//OUTBOUND MESSAGE HANDLERS
	void Privmsg(string receiver, string message);
	void Notice(string receiver, string message);
	void Whois(string lookupnick);
	
	void ChangeNick(string newNick);
	string GetNick() const { return mNickname; };
	
	void Rawmsg(string raw); //send a raw message to the server. 
	
	void Quit(string text = "");
	void MessageToChannel(string msg); 

	//OTHER FUNCTIONS
	void StateToString(string& s) const;
	
	string GetEncryptionKey() const; 

 	string mRealname;
	string mServerPassword;

	vString mServerList; //list of servers to try to connect to
	int mServerListIndex;
	
	// When IRC hubs bounce us, this will be different than the connect address
	string mRealServerAddress;
	
  private:
	void _setState(connectionState newState);
	bool _connectServer(string address);
	
	//Act upon various state changes. Returns false if we should not process anything else
	bool _checkState();
	
	connectionState mState;
	SDL_Thread* mConnectThread;
	IrcChannel* mChannel;
	
	string mNickname;
	
	bool mWaitingForPong; 
};

#endif //_IRCNET2_H_

