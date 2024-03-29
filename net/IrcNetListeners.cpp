
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


#include <time.h>
#include "IrcNetListeners.h"
#include "IrcNetSenders.h"
#include "../game/GameManager.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/widgets/Multiline.h"
#include "../interface/UserList.h"
#include "../interface/LoginDialog.h"
#include "../interface/OptionsDialog.h"
#include "../interface/WorldViewer.h"
#include "../map/Map.h"
#include "../core/net/IrcNet2.h"
#include "../entity/LocalActor.h"
#include "../entity/ExplodingEntity.h"
#include "../entity/TextObject.h"
#include "../core/io/FileIO.h"
#include "../core/net/DataPacket.h"
#include "../entity/RemoteActor.h"
#include "../avatar/Avatar.h"

/*	Utility */

void printMessage(string& msg)
{
	if (!game->GetChat())
		console->AddMessage(msg);
	else
		game->GetChat()->AddMessage(msg);

	if (loginDialog)
		loginDialog->mText->AddMessage(msg);
}

string decompressActionBuffer(string buffer)
{
	string s;
	s = decompressString(buffer);

	//if decompression worked (it was compressed) return that.
	if (!s.empty())
		return s;
	else //Otherwise, return original.
		return buffer;
}

//TODO: Move to Map?
RemoteActor* _addRemoteActor(string& nick, string& address, DataPacket& data)
{
	char c;
	point2d p;

	ASSERT(game->mMap);

	RemoteActor* ra = new RemoteActor();
	ra->mHostmask = address;
	ra->mMap = game->mMap;
	ra->SetLayer(EntityManager::LAYER_USER);
	ra->SetName(nick);
	if (userlist)
		userlist->AddNick(nick);

	p.x = data.ReadInt();
	p.y = data.ReadInt();
	ra->SetPosition(p);

	c = data.ReadChar();
	ra->SetDirection(c);

	c = data.ReadChar();
	ra->SetAction(c);

	if (!data.End()) //if we have avatar info (at least url/w/h), read it
	{
		ra->ReadAvatarFromPacket(data);
	}

	game->mMap->AddEntity(ra);
	return ra;
}

void _removeRemoteActor(RemoteActor* a)
{
	if (!a) return;

	/*Image* img;
	img = a->GetImage();
	if (img)
	{
		rect r = a->GetBoundingRect();
		new ExplodingEntity(a->mMap, img, point2d(r.x, r.y));
	}
	*/

	if (userlist)
		userlist->RemoveNick(a->GetName());

	a->mMap->RemoveEntity(a);
}

uShort timer_destroyMapStamp(timer* t, uLong ms)
{
	Entity* e = (Entity*)t->userData;

	if (game->mMap)
		game->mMap->RemoveEntity(e);

	return TIMER_DESTROY;
}

//TODO: Move to Map?
void stampMapText(int x, int y, int rotation, string& text)
{
	if (!game->mMap)
		return;

	TextObject* to = new TextObject();
		to->mId = "stamp";
		to->mMap = game->mMap;
		to->SetLayer(EntityManager::LAYER_GROUND);
		to->mMap->AddEntity(to);
		to->SetAA(true);
		to->SetFont("", 10);
		to->SetText(text, color());
		to->Rotozoom(rotation, 1.0);
		to->SetPosition( point2d(x - to->GetImage()->Width() / 2, y - to->GetImage()->Height() / 2) );

	// 5 minutes until the stamp cleans itself up
	timers->Add("stamp", 5*60*1000, false, timer_destroyMapStamp, NULL, to);
}

/*	This user sent a valid message, but we don't have them on the map. Request for add */
void _handleUnknownUser(string& nick)
{
	/*	TODO: Make sure they're actually in our channel! What if they sent
		a message from somewhere outside our channel? */
	if ( timers->Find("pushNm") )
		return;

	if (game->mPlayer->GetName() != nick)
		netSendState(nick, "sup");

	//make sure we don't send this TOO often. Only once every couple seconds
	timers->Add("pushNm", 2*1000, false, NULL, NULL);
}

void _handleNetMessage_RequestAvatar(string& nick, DataPacket& data) //reqAvy
{
	netSendAvatar(game->mPlayer->GetAvatar(), nick);
}

void _handleNetMessage_Say(string& nick, DataPacket& data) // say msg
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	if (!ra) { _handleUnknownUser(nick); return; }

	//if they're too far, or blocked, ignore
	//if ( ra->IsBlocked() || !ra->IsVisibleInCamera() )
	//	return;

	string msg = data.ReadString();
	MessageData md;

	if (msg.at(0) != '/')
	{
		md.SetId("NET_SAY");
	}
	else
	{
		md.SetId("NET_CMD");
	}

	md.WriteUserdata("entity", ra);
	md.WriteString("message", msg);
	messenger.Dispatch(md, ra);
}

void _handleNetMessage_Stamp(string& nick, DataPacket& data) // stamp x y angle msg
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	if (!ra) { _handleUnknownUser(nick); return; }

	if (ra->IsBlocked())
		return;

	int x = data.ReadInt();
	int y = data.ReadInt();
	int rot = data.ReadInt();
	string msg = data.ReadString();

	stampMapText(x, y, rot, msg);
}

void _handleNetMessage_Act(string& nick, DataPacket& data)
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	if (!ra) { _handleUnknownUser(nick); return; }

	//if they're too far, or blocked, ignore
	if ( ra->IsBlocked() || !ra->IsVisibleInCamera() )
		return;

	string formatted;

	char mode = data.ReadChar();
	string msg = data.ReadString();
	string msg2;

	switch (mode)
	{
		case 1: //music command: act 1 song
			formatted = "\\c099* " + stripCodes(nick)
						+ " is listening to \\c059"
						+ stripCodes( msg ) + " \\c099*";
			break;
		case 2: //beat command: act 2 target item
			msg2 = data.ReadString();
			formatted = "\\c484 * " + stripCodes(nick) + " beats "
						+ stripCodes( msg ) + " with a \\c784"
						+ stripCodes( msg2 ) + "\\c484 *";
			break;
		default: // act # msg
			formatted = "\\c909* " + stripCodes(nick)
						+ " " + stripCodes( msg ) + " *";
			break;
	}

	printMessage(formatted);
}

void _handleNetMessage_Avy(string& nick, DataPacket& data)
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	if (!ra) { _handleUnknownUser(nick); return; }

	if (ra->IsBlocked())
		return;

	ra->ReadAvatarFromPacket(data);
}

void _handleNetMessage_Sup(string& nick, string& address, DataPacket& data)
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	/*if (ra)
	{
		console->AddMessage("Double 'sup' from " + nick);
		return;
	}*/

	// make sure this sup is coming from our channel only
	if (data.ReadString() != game->mNet->GetChannel()->mId)
	{
		console->AddMessage("Illegal 'sup' from " + nick + " (chan)");
		return;
	}

	//Make sure someone didn't try to clone someone elses 'sup' message to mimic them
	if (data.ReadString() != nick)
	{
		console->AddMessage("Illegal 'sup' from " + nick + " (nick)");
		return;
	}

	/*	Add them to our map w/ their state data. Then reply
		with a "nothing much" edition of our state
	*/

	if (!ra)
	{
		ra = _addRemoteActor(nick, address, data);
	}

	if (ra)
	{
		netSendState("", "nm");
		DEBUGOUT("\\c090" + ra->GetName() + " has been added to the map");
	}
	else
	{
		console->AddMessage("Malformed 'sup' from " + nick);
	}
}

void _handleNetMessage_Nm(string& nick, string& address, DataPacket& data)
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	if (ra)
	{
		console->AddMessage("Double 'nm' from " + nick);
		return;
	}

	if (data.ReadString() != game->mNet->GetChannel()->mId)
	{
		console->AddMessage("Illegal 'nm' from " + nick + " (chan)");
		return;
	}

	if (data.ReadString() != nick)
	{
		console->AddMessage("Illegal 'nm' from " + nick + " (nick)");
		return;
	}

	/*	Add them to our map w/ their state data.
		Don't reply.
	*/
	ra = _addRemoteActor(nick, address, data);
	if (ra)
	{
		DEBUGOUT("\\c090" + ra->GetName() + " has been added to the map");
	}
	else
	{
		console->AddMessage("Malformed 'nm' from " + nick);
	}
}

void _handleNetMessage_Afk(string& nick, DataPacket& data)
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	if (!ra) { _handleUnknownUser(nick); return; }

	ra->SetAfk(true);
}

void _handleNetMessage_Back(string& nick, DataPacket& data)
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	if (!ra) { _handleUnknownUser(nick); return; }

	ra->SetAfk(false);
}

void _handleNetMessage_Mov(string& nick, DataPacket& data)
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	if (!ra) { _handleUnknownUser(nick); return; }

	/*	write our target coords into the buffer along with it. FORMAT: cXXXXX.YYYYYY.
		Reason we queue up the check in the buffer instead of writing it raw is that our computer
		may take longer to process than it takes another computer to send. And if we jumped coordinates
		every mov recv, we'd get jerky movement.
	*/

	int dx = data.ReadInt();
	int dy = data.ReadInt();
	string buffer = data.ReadString();

	ra->AddToActionBuffer( 'c' + its(dx) + "."
							+ its(dy) + "."
							+ decompressActionBuffer( buffer ) );

}

void _handleNetMessage_Emo(string& nick, DataPacket& data) // emo #emote
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	if (!ra) { _handleUnknownUser(nick); return; }

	if (ra->IsBlocked())
		return;

	char emote = data.ReadChar();

	ra->Emote( emote );
}

void _handleNetMessage_Lua(string& nick, DataPacket& data) // lua $id $msg
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	if (!ra) { _handleUnknownUser(nick); return; }

	string id = data.ReadString();
	string msg = data.ReadString();

	//Dispatch it and let lua listeners pick it up
	MessageData md;
	md.SetId("NET_LUA");
	md.WriteString("id", id);
	md.WriteUserdata("entity", ra);
	md.WriteString("message", msg);

	messenger.Dispatch(md, ra);
}

void _handleNetMessage_Mod(string& nick, DataPacket& data) // mod #type
{
	if (!game->mMap) return;

	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	if (!ra) { _handleUnknownUser(nick); return; }

	if (!ra->GetAvatar())
	{
		console->AddMessage("'mod' without Avatar from " + ra->GetName());
		return;
	}

	char type = data.ReadChar();

	ra->SetAvatarModifier( type );
}

void _handleNetMessage_Private(string& nick, string& msg)
{
	//if we don't accept private messages, send an auto respond
	if ( sti(game->mUserData.GetValue("MapSettings", "PrivMsg")) == 0 )
	{
		game->mNet->Privmsg( nick, "\\c900* I have whispers disabled *" );
	}
	else
	{
		//If they exist on the map, and are blocked, ignore.
		//TODO: Address blocking.
		RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
		if (ra && ra->IsBlocked())
		{
			game->mNet->Privmsg( nick, "\\c900* You are blocked *");
		}
		else
		{
			Console* c = game->GetPrivateChat(nick);
			c->AddMessage(nick + ": " + msg);
			gui->GetUserAttention();
		}
	}
}

void listener_NetPrivmsg(MessageListener* ml, MessageData& md, void* sender)
{
	if (!game->mNet->GetChannel())
	{
		DEBUGOUT("Privmsg with no channel");
		return;
	}

	string nick = md.ReadString("sender");
	string address = md.ReadString("address");
	string target = md.ReadString("target");
	string msg = md.ReadString("message");

	DEBUGOUT("[" + nick + "] " + msg);

	//Convert our message to a data packet.
	DataPacket data;
	data.SetKey( game->mNet->GetEncryptionKey() );

	//Couldn't decrypt. Assume plaintext and privmsg to us
	if (!data.FromString(msg) && target == game->mPlayer->GetName())
	{
		_handleNetMessage_Private(nick, msg);
		return;
	}

	string id = data.GetID();

	//Send our message to functions built for it
	if (id == "say")
		_handleNetMessage_Say(nick, data);
	else if (id == "act")
		_handleNetMessage_Act(nick, data);
	else if (id == "avy")
		_handleNetMessage_Avy(nick, data);
	else if (id == "sup")
		_handleNetMessage_Sup(nick, address, data);
	else if (id == "nm")
		_handleNetMessage_Nm(nick, address, data);
	else if (id == "mov")
		_handleNetMessage_Mov(nick, data);
	else if (id == "emo")
		_handleNetMessage_Emo(nick, data);
	else if (id == "stp")
		_handleNetMessage_Stamp(nick, data);
	else if (id == "lua")
		_handleNetMessage_Lua(nick, data);
	else if (id == "mod")
		_handleNetMessage_Mod(nick, data);
	else if (id == "afk")
		_handleNetMessage_Afk(nick, data);
	else if (id == "back")
		_handleNetMessage_Back(nick, data);
	else if (id == "reqAvy")
		_handleNetMessage_RequestAvatar(nick, data);
	else if (target == game->mPlayer->GetName())
		_handleNetMessage_Private(nick, msg);

	return;
}

void listener_NetCmdError(MessageListener* ml, MessageData& md, void* sender)
{
	int cmd = sti(md.ReadString("command"));
	string msg;
	switch (cmd)
	{
		case 442: //You're not on that channel! - Ignore.
			break;
		case 473: //+i invite only
		case 474: //+b you're banned
		case 475: //+k channel has key
			msg = "\\c900" + stripCodes(md.ReadString("message"));
			printMessage(msg);
			game->UnloadMap();
			break;
		default:
			msg = "\\c900 * [" + md.ReadString("command") + "] " + stripCodes(md.ReadString("message"));
			printMessage(msg);
			break;
	}
}

void listener_NetError(MessageListener* ml, MessageData& md, void* sender)
{
	string msg = "\\c900 * " + stripCodes(md.ReadString("message"));
	printMessage(msg);
}

void listener_NetTimeout(MessageListener* ml, MessageData& md, void* sender)
{
	string msg = "\\c900 * Connection Timed Out";
	printMessage(msg);
}

void listener_NetOnChannel(MessageListener* ml, MessageData& md, void* sender)
{
	string msg = "\\c090 * On Channel " + md.ReadString("channel");
	printMessage(msg);
}

void listener_NetChannelJoin(MessageListener* ml, MessageData& md, void* sender)
{
	string nick = md.ReadString("nick");
	if (game->mShowJoinParts)
	{
		string msg;
		msg = "\\c090 * " + nick + "\\c090";

		if (game->mShowAddresses)
			msg += " (" + md.ReadString("address") + ")";

		msg += " joins";

		printMessage(msg);
	}

	//If it's not us, send our data to them
	if (game->mPlayer->GetName() != nick)
	{
		netSendState(nick, "sup");
	}

	//This is here, because anywhere else it would play constantly when we join a new map filled with people.
//	if (sound)
//		sound->Play("warpin");
}

void listener_NetChannelKick(MessageListener* ml, MessageData& md, void* sender)
{
	string msg;
	string nick = md.ReadString("nick");
	string kicker = md.ReadString("kicker");
	string reason = md.ReadString("reason");

	if (game->mShowJoinParts)
	{
		msg = "\\c139 * " + nick + "\\c139 has been kicked by "
			+ kicker + "\\c139 (" + reason + ")";

		printMessage(msg);
	}

	if (nick == game->mPlayer->GetName()) //I was kicked. Will rejoin immediately
	{
		//console->AddMessage("\\c900 * That son of a bitch. Rejoining");
		//SAFEDELETE( ((IrcNet*)sender)->mChannel );
		//fast rejoin code. Ignores all that map loading stuff.
		//I COULD trigger a full re-download on a kick, but that's pointless.
		//JoinChannel(mChannel->mId, mChannel->mPassword);
		msg = "\\c099 You have been kicked!";
		game->UnloadMap();

		printMessage(msg);
	}
	else if (game->mMap)
	{
		//Remove them from our map if they were on it
		Entity* e = game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
		_removeRemoteActor((RemoteActor*)e);
	}
}

void listener_NetChannelPart(MessageListener* ml, MessageData& md, void* sender)
{
	if (!game->mMap)
		return;

	string nick = md.ReadString("nick");

	if (game->mShowJoinParts)
	{
		string msg;
		msg = "\\c139 * " + nick + "\\c139";

		if (game->mShowAddresses)
			msg += " (" + md.ReadString("address") + ")";

		msg += " leaves";

		printMessage(msg);
	}

	//Remove them from our map if they were on it
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	_removeRemoteActor(ra);
}

void listener_NetChannelQuit(MessageListener* ml, MessageData& md, void* sender)
{
	string msg;
	string reason;
	string nick = md.ReadString("nick");

	if (game->mShowJoinParts)
	{
		msg = "\\c139 * " + nick + "\\c139";

		if (game->mShowAddresses)
			msg += " (" + md.ReadString("address") + ")";

		msg += " quit";

		reason = md.ReadString("reason");
		if (!reason.empty())
			msg += " (" + reason + ")";

		printMessage(msg);
	}

	if (nick == game->mPlayer->GetName())
	{
		msg = "User Quit";
		printMessage(msg);
	}
	else if (game->mMap)
	{
		//Remove them from our map if they were on it
		RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
		_removeRemoteActor(ra);
	}
}

//nicks (seperated by commas)
void listener_NetUserlist(MessageListener* ml, MessageData& md, void* sender)
{
	string msg = "\\c099Online Users: " + stripCodes(md.ReadString("list"));

	//Pad the seperation between nicks with color tags
	replace(&msg, ",", "\\c099,");

	printMessage(msg);
}

//(Note: This will be sent multiple times for extremely long motds, with different command ids)
void listener_NetMotd(MessageListener* ml, MessageData& md, void* sender)
{
	//TODO: Maybe dump into a seperate output?
	string msg = "\\c940" + md.ReadString("message");
	printMessage(msg);
}

void listener_NetCouldNotConnect(MessageListener* ml, MessageData& md, void* sender)
{
	IrcNet* net = (IrcNet*)sender;

	string msg;
	msg = "\\c900 * Could not connect to " + net->mHost + ":" + its(net->mPort);
	printMessage(msg);

	net->TryNextServer();
}

void listener_NetNotice(MessageListener* ml, MessageData& md, void* sender)
{
	string msg;
	msg = "\\c900-" + md.ReadString("nick") + "\\c900- " + stripCodes(md.ReadString("message"));

	printMessage(msg);
}

// http://www.mirc.net/raws/
void listener_NetUnknown(MessageListener* ml, MessageData& md, void* sender)
{
	int cmd = sti(md.ReadString("command"));

	//skip commands
	if (cmd == 333)
		return;

	vString v;
	string line = md.ReadString("line");
	explode(&v, &line, " ");

	/*Console* c;   Idle lies. When someone joins, you send a privmsg to them and it destroys idle state.
	if (cmd == "317") //RPL_WHOISIDLE "<nick> <integer> :seconds idle"
	{
		c = game->GetPrivateChat( v.at(3) );
		if (c)
			c->AddMessage(v.at(3) + "\\c099 has been idle for " + v.at(4) + " seconds");
	}*/

	string msg;
	msg = "\\c900 * (" + md.ReadString("command") + ") " + stripCodes(md.ReadString("line"));

	printMessage(msg);
}

//channel, nick who sent it (or blank if server), new topic
void listener_NetTopic(MessageListener* ml, MessageData& md, void* sender)
{
	string msg;
	string nick = md.ReadString("nick");
	string topic = md.ReadString("message");

	if (nick.empty())
		msg = "\\c090 * Topic: " + topic;
	else
		msg = "\\c090 * " + nick + "\\c090 sets topic: " + topic;

	printMessage(msg);
}

//server address or nick, mode set
void listener_NetMode(MessageListener* ml, MessageData& md, void* sender)
{
	string msg;
	msg = "\\c090 * " + md.ReadString("sender") + "\\c090 sets mode " + md.ReadString("mode");

	printMessage(msg);
}

//old nick, new nick
void listener_NetNick(MessageListener* ml, MessageData& md, void* sender)
{
	IrcNet* net = (IrcNet*)sender;

	string oldn = md.ReadString("oldnick");
	string newn = md.ReadString("newnick");

	Entity* e;
	if (newn == net->GetNick()) //we've changed our nick
	{
		e = game->mPlayer;

		game->mUserData.SetValue("MapSettings", "Nick", newn);
	}
	else
	{
		e = game->mMap->FindEntityByName(oldn, ENTITY_REMOTEACTOR);
	}

	if (!e)
		return;

	e->SetName(newn);
	if (userlist)
		userlist->ChangeNick(oldn, newn);

	string msg;
	if (game->mShowJoinParts && e->IsVisibleInCamera())
	{
		msg = "\\c090 * " + oldn + "\\c090 is now known as " + newn;
		printMessage(msg);
	}
}

//New connectionState
void listener_NetNewState(MessageListener* ml, MessageData& md, void* sender)
{
	int state = md.ReadInt("state");

	DEBUGOUT("NET STATE: " + its(state));
	IrcNet* net = (IrcNet*)sender;

	game->UpdateAppTitle();

	timers->Remove("resetNm"); //so we can send it again immediately

	string s;
	switch (state)
	{
		case CONNECTING:
			s = "\\c050 * Connecting to server...";
			printMessage(s);
			break;
		case AWAITINGSERVERVERIFY: //Connected, awaiting verify. Send nick.
			s = "\\c139 * Waiting for verification";
			printMessage(s);
			net->ChangeNick(game->mPlayer->GetName());
			break;
		case VERIFYING:
		case ONSERVER:
			break;
		case ONCHANNEL:
			DEBUGOUT("ONCHANNEL");
			if (game->mLoader)
				game->mLoader->SetState(WorldLoader::WORLD_READY);

			// if we have a default nick still, bring up a request to change it
			if (net->GetNick().find("fro_", 0) == 0)
			{
				if (!gui->Get("OptionsDialog"))
				{
					OptionsDialog* o = new OptionsDialog();
					o->DemandFocus(true);
				}
				new MessagePopup("", "Change your nick!",
					"We recommend changing your nick from the default to something new!"
					" When you enter a new nickname, hit the green checkmark to save!");
			}
			break;
		case DISCONNECTED:
			game->UnloadMap();

			if (!loginDialog)
				new LoginDialog();

			new MessagePopup("", "Disconnected", "Disconnected from Server");
			break;
		default: break;
	}
}

//message ( Nickname already in use / Erronous Nickname )
void listener_NetNickInUse(MessageListener* ml, MessageData& md, void* sender)
{
	IrcNet* net = (IrcNet*)sender;

	string msg = "\\c900 * Could not change nickname: " + stripCodes(md.ReadString("message"));
	printMessage(msg);

	if (net->GetState() != ONCHANNEL) // not on a world yet, force change
	{
		net->ChangeNick(net->GetNick() + "_");

		game->mPlayer->SetName(game->mPlayer->GetName() + "_");
		net->ChangeNick(game->mPlayer->GetName());
	}
}

// Nothing in message
void listener_NetVerified(MessageListener* ml, MessageData& md, void* sender)
{
	IrcNet* net = (IrcNet*)sender;

	if (loginDialog)
		loginDialog->SetActive(false);

	// Bug fix, the server could have changed our nick on join (if it was too long, etc)
	// Without giving an Erronous Nickname message.
	game->mPlayer->SetName(net->GetNick());

	//game->LoadOnlineWorld(game->mStartingWorldId);

	WorldViewer* viewer = new WorldViewer();
    viewer->mClose->SetVisible(false);
}

void listener_NetWhois(MessageListener* ml, MessageData& md, void* sender)
{
	IrcNet* net = (IrcNet*)sender;

	time_t t = (time_t)md.ReadInt("signon");

	string msg = "\\c099[WHOIS] " + md.ReadString("nick") + "\\c099 signed on at ";
		msg += string(asctime(localtime(&t)));
	printMessage(msg);
}

void listener_NetWhois2(MessageListener* ml, MessageData& md, void* sender)
{
	IrcNet* net = (IrcNet*)sender;

	string msg = "\\c099[WHOIS] " + md.ReadString("nick") + "\\c099's hostname: ";
		msg += md.ReadString("address");
	printMessage(msg);

	msg = "\\c099[WHOIS] " + md.ReadString("nick") + "\\c099 is logged in as: ";
		msg += md.ReadString("realname");
	printMessage(msg);
}

void hookNetListeners()
{
	//Hook event listeners
	messenger.AddListener("NET_VERIFIED", listener_NetVerified);
	messenger.AddListener("NET_TOPIC", listener_NetTopic);
	messenger.AddListener("NET_NOTICE", listener_NetNotice);
	messenger.AddListener("NET_USERLIST", listener_NetUserlist);
	messenger.AddListener("NET_QUIT", listener_NetChannelQuit);
	messenger.AddListener("NET_PART", listener_NetChannelPart);
	messenger.AddListener("NET_JOIN", listener_NetChannelJoin);
	messenger.AddListener("NET_KICK", listener_NetChannelKick);
	messenger.AddListener("NET_ONCHANNEL", listener_NetOnChannel);
	messenger.AddListener("NET_ERROR", listener_NetError);
	messenger.AddListener("NET_CMDERROR", listener_NetCmdError);
	messenger.AddListener("NET_NICKINUSE", listener_NetNickInUse);
	messenger.AddListener("NET_NEWSTATE", listener_NetNewState);
	messenger.AddListener("NET_PRIVMSG", listener_NetPrivmsg);
	messenger.AddListener("NET_NICK", listener_NetNick);
	messenger.AddListener("NET_MODE", listener_NetMode);
	messenger.AddListener("NET_UNKNOWN", listener_NetUnknown);
	messenger.AddListener("NET_MOTD", listener_NetMotd);
	messenger.AddListener("NET_FAILED", listener_NetCouldNotConnect);
	messenger.AddListener("NET_TIMEOUT", listener_NetTimeout); //NOT IMPLEMENTED
	messenger.AddListener("NET_WHOIS", listener_NetWhois);
	messenger.AddListener("NET_WHOIS2", listener_NetWhois2);
}

void unhookNetListeners()
{
	messenger.RemoveListener(listener_NetVerified);
	messenger.RemoveListener(listener_NetTopic);
	messenger.RemoveListener(listener_NetNotice);
	messenger.RemoveListener(listener_NetUserlist);
	messenger.RemoveListener(listener_NetChannelQuit);
	messenger.RemoveListener(listener_NetChannelPart);
	messenger.RemoveListener(listener_NetChannelJoin);
	messenger.RemoveListener(listener_NetChannelKick);
	messenger.RemoveListener(listener_NetOnChannel);
	messenger.RemoveListener(listener_NetError);
	messenger.RemoveListener(listener_NetCmdError);
	messenger.RemoveListener(listener_NetNickInUse);
	messenger.RemoveListener(listener_NetNewState);
	messenger.RemoveListener(listener_NetPrivmsg);
	messenger.RemoveListener(listener_NetNick);
	messenger.RemoveListener(listener_NetMode);
	messenger.RemoveListener(listener_NetUnknown);
	messenger.RemoveListener(listener_NetMotd);
	messenger.RemoveListener(listener_NetCouldNotConnect);
	messenger.RemoveListener(listener_NetTimeout);
	messenger.RemoveListener(listener_NetWhois);
	messenger.RemoveListener(listener_NetWhois2);
}
