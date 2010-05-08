
#include <SDL/SDL.h> //for SDL_Thread
#include "IrcNet2.h"
#include "../io/Crypt.h"

//Utility function. Should go into common...
string getWord(string text, int n)
{
	int pos = 0;
	for (int i = 1; i < n; i++) 
	{
		pos = text.find (' ', pos) + 1;
	}
	return text.substr(pos, text.find (' ', pos) - pos);
}

uShort timer_netProcess(timer* t, uLong ms)
{
	IrcNet* net = (IrcNet*)t->userData;
	
	if (!net)
		return TIMER_DESTROY;
		
	net->Process();
	return TIMER_CONTINUE;
}

uShort timer_netTimeout(timer* t, uLong ms)
{
	IrcNet* net = (IrcNet*)t->userData;
	
	if (net)
	{
		MessageData md("NET_TIMEOUT");
		messenger.Dispatch(md, net);
		
		net->Disconnect();
		net->TryNextServer();
	}
	
	return TIMER_DESTROY;
}

uShort timer_netPing(timer* t, uLong ms)
{
	IrcNet* net = (IrcNet*)t->userData;
	
	if (!net)
		return TIMER_DESTROY;

	net->PingServer();
	
	return TIMER_CONTINUE;
}

/*	Does simplistic socket connecting outside the main thread so we don't get locked up.
	Processing server messages will be done via the main.
	TODO: This could fuck up if we try to connect again while attempting to connect
		(ie: spawn a second thread that calls ircNet->Connect() @ the same time)
*/
int thread_serverConnect(void* param)
{
	IrcNet* in = (IrcNet*)param;
	in->Connect();
	return 0;
}

IrcNet::IrcNet() 
{
	mChannel = NULL;
	mMaxMessageSize = 512; //irc max size
	mServerListIndex = 0;
	mState = DISCONNECTED;
	mConnectThread = NULL;
	mWaitingForPong = false;
	
	timers->AddProcess("netProcess", timer_netProcess, NULL, this);
//	timers->Add("netPing", PING_INTERVAL_MINUTES*60*1000, false, timer_netPing, NULL, this);
}

IrcNet::~IrcNet() 
{
	SAFEDELETE(mChannel);
	
	if (IsConnected())
	{
		Quit();
		//TODO: I don't think they get the quit if the socket closes right after..
		Disconnect();
	}
	
	if (mConnectThread)
	{
		WARNING("Forcing a kill on thread " + pts(mConnectThread));
		SDL_KillThread(mConnectThread);
		mConnectThread = NULL;
	}

	timers->RemoveMatchingUserData(this);
}

void IrcNet::Connect() //NOTE: This will be ran from a thread!
{
	TextSocketConnection::Connect();
	
	//TODO: Probably mutex locking to make sure we don't access these when in use. 
	//Can't do from a thread. _setState( (IsConnected()) ? CONNECTED : COULDNOTCONNECT );
	mState = (IsConnected()) ? CONNECTED : COULDNOTCONNECT;

	mConnectThread = NULL;
}

bool IrcNet::ConnectToServer(string address) 
{
	mServerListIndex = mServerList.size();
	mServerList.push_back(address);
	
	return _connectServer(address);
}

bool IrcNet::TryNextServer()
{
	if (mServerList.empty())
		return false;

	if (mServerListIndex >= mServerList.size())
		mServerListIndex = 0;

	_setState(CONNECTING);
	
	bool result = _connectServer(mServerList.at(mServerListIndex));
	
	mServerListIndex++;
	
	return result;
}

//Returns true if already connected. False if the address is invalid or it's trying to connect in another thread.
bool IrcNet::_connectServer(string address)
{
	int port;
	int i;
	
	i = address.find(":");
	if (i == string::npos)
	{
		console->AddMessage("\\c900 * Invalid Server Format: " + address);
		_setState(COULDNOTCONNECT);
		return false;
	}

	port = sti( address.substr(i + 1) );
	address.erase(i);

	if (address == mHost && IsConnected())
	{
		console->AddMessage("\\c090 * Server Alreadly Connected!");	
		return true;
	}

	SetDestination(address, port);
	//_setState(CONNECTING);

	if (mConnectThread)
	{
		WARNING("Forcing a kill on thread " + pts(mConnectThread));
		SDL_KillThread(mConnectThread);
	}
	
	//Run it in a seperate thread so we don't lock up the client
	mConnectThread = SDL_CreateThread(thread_serverConnect, this);
				
	//add our reconnect timer to ensure we stay on a server while the client is running
//	timers->Add("reconnect", sti(config.GetParamInt("communication", "reconnect")) * 60000, 
//					false, timer_reconnect);

	return false;
}

//called once the sockets connect.
void IrcNet::OnConnect()
{
	string msg;
	
	if (!mServerPassword.empty()) 
	{
		msg = "PASS " + mServerPassword + "\r\n";
		SendLine(msg);
	}
		
	//ChangeNick(mRealname);
	
	msg = "USER FroUser 0 * :" + mRealname + "\r\n";
	SendLine(msg); 
}

void IrcNet::PingServer()
{	
	string s;
	if (IsConnected())
	{
		if (mWaitingForPong) //the previous ping timed out
		{
			Disconnect();
		}
		else
		{
			mWaitingForPong = true;
			s = "PING " + mRealServerAddress + "\r\n";
			SendLine(s);
		}
	}
}

IrcChannel* IrcNet::CreateChannel(string chan, string pass)
{
	IrcChannel* c = new IrcChannel();
	c->mId = lowercase(chan);
	c->mPassword = pass;
	c->mEncryptionKey = makePassword(c->mId, SECONDARY_PACKET_PASS);
	return c;
}

void IrcNet::JoinChannel(IrcChannel* chan)
{
	if (!chan || !IsConnected()) return;
	
	PRINT("#####JOINING");

	PartChannel(mChannel); 
	mChannel = chan;

	MessageData md("NET_JOINING");
	md.WriteString("channel", mChannel->mId);
	messenger.Dispatch(md, this);
	
	string msg = "JOIN " + mChannel->mId + " " + mChannel->mPassword + "\r\n";
	SendLine(msg);

	PRINT("#####JOINDONE");
}

void IrcNet::JoinChannel(string chan, string pass) 
{
	JoinChannel(CreateChannel(chan, pass));
}

void IrcNet::Privmsg(string receiver, string message) 
{
	string msg = "PRIVMSG " + receiver + " :" + message + "\r\n";
	SendLine(msg);
}

void IrcNet::Notice(string receiver, string message) 
{
	string msg = "NOTICE " + receiver + " :" + message + "\r\n";
	SendLine(msg);
}

void IrcNet::Whois(string lookupnick) 
{
	string msg = "WHOIS " + lookupnick + "\r\n";
	SendLine(msg);
}

void IrcNet::PartChannel(IrcChannel* chan) 
{
	if (!mChannel) return;
	
	DEBUGOUT("\\c900Sending Part: " + mChannel->mId);
	string msg = "PART " + mChannel->mId + "\r\n";
	SendLine(msg);
	SAFEDELETE(mChannel);
}

void IrcNet::SetTopic(IrcChannel* chan, string topic) 
{
	if (!chan) return;
	string msg = "TOPIC " + chan->mId + " :" + topic + "\r\n";
	SendLine(msg);
}

void IrcNet::ChangeNick(string newNick) 
{
	string msg = "NICK " + newNick + "\r\n";
	mNickname = newNick;
	SendLine(msg);
}

/*
void IrcNet::setMode(string receiver, string mode) {
	SendLine("MODE " + receiver + " " + mode + "\r\n");
}

void IrcNet::invite(string nick) {
	SendLine("INVITE " + nick + " " + mChannel + "\r\n");
}

void IrcNet::kick(string nick, string reason) {
	SendLine("KICK " + mChannel + " " + nick + " :" + reason + "\r\n");
}
*/

void IrcNet::Rawmsg(string raw) 
{
	raw += "\r\n";
	SendLine(raw);
}

void IrcNet::Quit(string text)
{
	string msg = "QUIT :" + text + "\r\n";
	SendLine(msg);
	
	//If we're not on a channel, forcefully disconnect.
	if (!mChannel)
		Disconnect();
		
	if (mConnectThread)
	{
		SDL_KillThread(mConnectThread);
		mConnectThread = NULL;
	}
}

void IrcNet::MessageToChannel(string msg) 
{
	if (mChannel && mChannel->mSuccess)
		Privmsg(mChannel->mId, msg);	
}

//write all possible information about the network
void IrcNet::StateToString(string& s) const
{
	s += "NET\\nState: ";
	switch (mState)
	{
		case CONNECTED: s += "CONNECTED"; break;
		case COULDNOTCONNECT: s += "COULD NOT CONNECT"; break;
		case DISCONNECTED: s += "DISCONNECTED"; break;
		case AWAITINGSERVERVERIFY: s += "AWAITING VERIFY"; break;
		case ONCHANNEL: s += "ON CHANNEL"; break;
		case ONSERVER: s += "ON SERVER"; break;
		case CONNECTING: s += "CONNECTING"; break;
		default: break;
	}
	if (mChannel)
		s += "\\nChannel: " + mChannel->mId + " [" + mChannel->mPassword + "]"; 
}

void IrcNet::_setState(connectionState newState)
{
	//add timeout timers
	if (newState == AWAITINGSERVERVERIFY || newState == CONNECTING)
	{
		timers->Add("netTimeout", NET_TIMEOUT_SECONDS*1000, false, timer_netTimeout, NULL, this);
	}
	else //make sure timeout timers don't activate
	{
		timers->Remove("netTimeout", this);
	}
	
	mState = newState;

	MessageData md("NET_NEWSTATE");
	md.WriteInt("state", newState);
	
	messenger.Dispatch(md, this);
}

//Act upon various state changes. Returns false if we should not process anything else
bool IrcNet::_checkState()
{
	bool result = true;
	MessageData md;
	switch (GetState())
	{
		case CONNECTED:
			_setState(CONNECTED);
			OnConnect(); //send our login information
			_setState(AWAITINGSERVERVERIFY);
			break;
		case COULDNOTCONNECT:
			_setState(COULDNOTCONNECT);

			md.SetId("NET_FAILED");
			md.Clear();
			
			messenger.Dispatch(md, this);

			//_setState(DISCONNECTED);
			result = false;

			break;
		case DISCONNECTED:
			//don't run below.
			result = false;
			break;
		case AWAITINGSERVERVERIFY:
		case VERIFYING:
			break;
		case ONCHANNEL: //Our channel died somehow but we never updated the state change.
			if (!mChannel)
			{
				DEBUGOUT("ONCHANNEL->ONSERVER");
				_setState(ONSERVER);
			}
			break;
		case ONSERVER: //disconnected somewhere in here
			break;
		case CONNECTING: //thread is still working
			result = false;
			break;
		default: break;
	}
	return result;
}

bool IrcNet::Process() 
{	
	//Return false if we're not actually connected and ready to receive messages
	if (!_checkState())
		return false;

	if (!IsConnected())
	{
		console->AddMessage("IrcNet::Process DISCONNECTED");
		_setState(DISCONNECTED);
	}

	string line;
	string s1, s2, s3;
	MessageData md;
	
	while (CanReadLine()) 
	{
		line = GetLine();
		PRINTF("%s\n", line.c_str());
		//console->AddMessage(line);

		if (line.at(0) == ':')
		{
			string cmd = lowercase(getWord (line, 2));
			string msg = line.substr(line.find (':',1) + 1);

			if (cmd == "privmsg") 
			{
				s1 = line.substr(1, line.find ('!',0) - 1); //user who sent it
				s2 = line.substr(s1.length() + 2, line.find(' ', s1.length()) - s1.length() - 2); //address
				
				md.Clear();
				md.WriteString("sender", s1); //Sender Nick
				md.WriteString("address", s2); //Sender Address
				md.WriteString("target", getWord(line, 3)); //Target
				md.WriteString("message", msg); //Message	

				string x01 = "\x01";
				if (msg.find(x01 + "VERSION", 0) == 0)
					Notice(s1, x01 + "VERSION Sybolt.com - IrcNet2 " + x01);
				else if (msg.find(x01 + "PING", 0) == 0)
					Notice(s1, x01 + "PING PONG" + x01);
				else if (msg.find(x01 + "TIME", 0) == 0)
					Notice(s1, x01 + "TIME Last Thursday" + x01);
				else if (msg.find(x01 + "FINGER", 0) == 0)
					Notice(s1, x01 + "FINGER Get that out of here." + x01);
				else if (msg.find(x01 + "ACTION", 0) == 0)
				{
					md.SetId("NET_ACTION");
					messenger.Dispatch(md, this);
				}
				else //:Halio!~HalioBot@25C32C5.C816C137.BF0FDDF8.IP PRIVMSG #drm.library :Blah Blah Blah..
				{
					md.SetId("NET_PRIVMSG");
					messenger.Dispatch(md, this);
				}
			}
			else if (cmd == "join") //:Halio!~HalioBot@25C32C5.C816C137.BF0FDDF8.IP JOIN :#drm.library..
			{
				s1 = line.substr(1, line.find ('!',0) - 1); //joiner				
				s2 = line.substr(s1.length() + 2, line.find(' ', s1.length()) - s1.length() - 2);	
				if (s2.find("@", 2) != string::npos) //trash the realname
					s2.erase(2, s2.find("@", 2) - 2);

				md.Clear();
				md.SetId("NET_JOIN");
				md.WriteString("nick", s1); //Joiner Nick
				md.WriteString("address", s2); //Joiner Address

				messenger.Dispatch(md, this);
			}
			else if (cmd == "kick")
			{
				s1 = getWord(line, 4); //kicked
				s2 = line.substr(1, line.find ('!',0) - 1); //nick that kicked them
				
				md.Clear();
				md.SetId("NET_KICK");
				md.WriteString("nick", s1); //Kicked Nick
				md.WriteString("kicker", s2); //Kicker Nick
				md.WriteString("reason", msg); //Reason

				messenger.Dispatch(md, this);
				
				if (s1 == mNickname)
				{
					SAFEDELETE(mChannel);
				}	
			} 
			else if (cmd == "nick") //:Noel!~noel@Rizon-B5D29C05.hsd1.ca.comcast.net NICK :test...
			{
				s1 = line.substr(1, line.find ('!',0) - 1); //old nick
				s2 = line.substr(line.find (':',1)+1); //new nick
		
				md.Clear();
				md.SetId("NET_NICK");
				md.WriteString("oldnick", s1); //Old Nick
				md.WriteString("newnick", s2); //New Nick

				messenger.Dispatch(md, this);
			}
			else if (cmd == "part")
			{
				s1 = line.substr(1, line.find ('!',0) - 1); //nick
				
				s2 = line.substr(s1.length() + 2, line.find(' ', s1.length()) - s1.length() - 2);	
				if (s2.find("@", 2) != string::npos) //trash the realname
					s2.erase(2, s2.find("@", 2) - 2);
				
				md.Clear();
				md.SetId("NET_PART");
				md.WriteString("nick", s1); //Parter Nick
				md.WriteString("address", s2); //Parter Address
				
				messenger.Dispatch(md, this);
			}
			else if (cmd == "mode") 
			{
				if (line.find('!', 0) == string::npos)
					s1 = line.substr(1, line.find (' ',0) - 1); //server
				else
					s1 = line.substr(1, line.find ('!',0) - 1); //nick
				
				s2 = line.substr(line.find("MODE", 0) + 4); //Mode

				md.Clear();
				md.SetId("NET_MODE");
				md.WriteString("sender", s1); //Server Address or Nick
				md.WriteString("mode", s2); //Mode set
				
				messenger.Dispatch(md, this);
			} 
			else if (cmd == "topic")
			{ 
				s1 = getWord(line, 3); //chan
				s2 = line.substr(1, line.find ('!',0) - 1); //nick
				
				if (mChannel && s1 == mChannel->mId)
					mChannel->mTopic = msg;	
					
				md.Clear();
				md.SetId("NET_TOPIC");
				md.WriteString("channel", s1); //channel
				md.WriteString("nick", s2); //nick who set it
				md.WriteString("message", msg); //new topic
				
				messenger.Dispatch(md, this);
			}
			else if (cmd == "quit") //:Halio!~HalioBot@25C32C5.C816C137.BF0FDDF8.IP QUIT :Read error: Connection reset by peer..
			{ 
				s1 = line.substr(1, line.find ('!',0) - 1); //user
				s2 = line.substr(line.find(':',1) + 1); //reason
				s3 = line.substr(s1.length() + 2, line.find(' ', s1.length()) - s1.length() - 2); //address
				
				if (s3.find("@", 2) != string::npos) //trash the realname
					s3.erase(2, s3.find("@", 2) - 2);
				
				md.Clear();
				md.SetId("NET_QUIT");
				md.WriteString("nick", s1); //Parter Nick
				md.WriteString("address", s3); //Parter Address
				md.WriteString("reason", s2); //Reason
				
				messenger.Dispatch(md, this);
			}
			else if (cmd == "notice") //:Noel!~noel@Rizon-1F2DCFAD.hsd1.ca.comcast.net NOTICE Drm_gabide :ffff
			{
				s1 = getWord(line, 1); 
				s1 = s1.substr( 1, s1.find("!", 1) - 1 );

				md.Clear();
				md.SetId("NET_NOTICE");
				md.WriteString("nick", s1); //Sender Nick
				md.WriteString("message", msg); //Notice Message

				messenger.Dispatch(md, this);
			}
			else if (cmd == "pong")
			{
				DEBUGOUT("PONG");
				mWaitingForPong = false; //should probably double check the address, but I don't care.	
			}
			else if (cmd == "332") //channel topic when joining
			{ 
				s1 = getWord (line, 4);
				
				if (mChannel && s1 == mChannel->mId) 
				{
					mChannel->mTopic = msg;
					mChannel->mSuccess = true;
				}
				
				md.Clear();
				md.SetId("NET_TOPIC");
				md.WriteString("channel", s1); //channel
				md.WriteString("nick", ""); //No nick (was sent while joining)
				md.WriteString("message", msg); //topic
				
				messenger.Dispatch(md, this);

			} 
			else if (cmd == "433" || cmd == "432") // Nickname already in use / Erronous Nickname
			{ 
				md.Clear();
				md.SetId("NET_NICKINUSE");
				md.WriteString("message", msg); //message
				
				messenger.Dispatch(md, this);
			} 
			else if (cmd == "001") //successful registration
			{		
				// Get the real host name
				mRealServerAddress = getWord(line, 1);
				mRealServerAddress.erase(0, 1); //erase prefix colon

				_setState(ONSERVER);

				md.Clear();
				md.SetId("NET_VERIFIED");
				
				messenger.Dispatch(md, this);

				return true; //gotta break out so we can stop the doConnect loop
			} 
			else if (cmd == "439") // Please wait while we process your connection
			{
				// Get the real host name
				mRealServerAddress = getWord(line, 1);
				mRealServerAddress.erase(0, 1); //erase prefix colon
			
				_setState(VERIFYING);
			}
			else if (cmd == "353") //userlist
			{	
				md.Clear();
				md.SetId("NET_USERLIST");
				md.WriteString("list", msg); //list
				
				messenger.Dispatch(md, this);
			}
			else if (cmd == "366") // :irc.lunarforums.org 366 ircNet_Nick #drm-testing :End of /NAMES list.
			{ 		
				mChannel->mSuccess = true;
				_setState(ONCHANNEL);			
				
				mChannel->mId = getWord(line, 4);
				
				md.Clear();
				md.SetId("NET_ONCHANNEL");
				md.WriteString("channel", mChannel->mId); //channel
				
				messenger.Dispatch(md, this);
			}
			else if (cmd == "372" || cmd == "375" || cmd == "376") //MOTD from server
			{
				md.Clear();
				md.SetId("NET_MOTD");
				md.WriteString("message", msg); //motd data
				
				messenger.Dispatch(md, this);
			}
			else if (atoi(cmd.c_str()) > 400 && atoi(cmd.c_str()) < 503) //command ERROR reply from IRC server
			{		
				md.Clear();
				md.SetId("NET_CMDERROR");
				md.WriteString("command", cmd); //the command issued
				md.WriteString("message", msg); //the error
				
				messenger.Dispatch(md, this);
			} 
			else //unidentified
			{
				md.Clear();
				md.SetId("NET_UNKNOWN");
				md.WriteString("command", cmd); //the command issued
				md.WriteString("line", line); //the full line of data sent
				
				messenger.Dispatch(md, this);
			}
		}
		else if (getWord(line, 1) == "ERROR") //ERROR :Closing Link: Rizon-1F2DCFAD.hsd1.ca.comcast.net ()
		{
			s1 = line.substr(line.find(':',1) + 1); 
						
			md.Clear();
			md.SetId("NET_ERROR");
			md.WriteString("message", s1); //the error
			
			messenger.Dispatch(md, this);
		}
		else if (getWord(line, 1) == "PING") //  necessary to keep us hooked up to the server
		{
			s1 = line.substr(4);
			s2 = "PONG" + s1 + "\r\n";
			SendLine(s2);
			
			md.Clear();
			md.SetId("NET_PING");
			md.WriteString("sender", s1); //the server
			
			messenger.Dispatch(md, this);
		}
	}
	return true;
}
