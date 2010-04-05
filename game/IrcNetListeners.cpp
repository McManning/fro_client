
#include "IrcNetListeners.h"
#include "GameManager.h"
#include "Achievements.h"
#include "../core/widgets/MessagePopup.h"
#include "../interface/UserList.h"
#include "../interface/LoginDialog.h"
#include "../interface/ItemTrade.h"
#include "../map/Map.h"
#include "../core/net/IrcNet2.h"
#include "../entity/LocalActor.h"
#include "../entity/ExplodingEntity.h"
#include "../entity/TextObject.h"
#include "../core/io/FileIO.h"
#include "../core/widgets/Button.h"
#include "../core/net/IrcNet2.h"
#include "../core/net/DataPacket.h"
#include "../core/io/Crypt.h"
#include "../entity/RemoteActor.h"
#include "../entity/Avatar.h"
#include "../core/sound/SoundManager.h"

/*	Utility */

void printMessage(string& msg)
{
	if (!game->mChat) 
		console->AddMessage(msg);
	else
		game->mChat->AddMessage(msg);
}

string compressActionBuffer(string buffer)
{
	return buffer;
	string s;
	s = compressString(buffer);

	//if our compressed version is smaller than our raw, return that.
	if (!s.empty() && s.length() < buffer.length())
		return s;
	else //Otherwise, return original.
		return buffer;
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

//Move to Map?
RemoteActor* _addRemoteActor(string& nick, DataPacket& data)
{
	ASSERT(game->mMap);

	if (data.Size() < 6)
		return NULL;

	RemoteActor* ra = new RemoteActor();
	ra->mMap = game->mMap;
	ra->SetLayer(EntityManager::LAYER_USER);
	ra->mName = nick;
	if (userlist)
		userlist->AddNick(nick);
	
	ra->SetPosition( point2d(data.ReadInt(2), data.ReadInt(3)) );
	ra->SetDirection( data.ReadInt(4) );
	ra->SetAction( data.ReadInt(5) );
	
	if (data.Size() > 8) //if we have avatar info (at least url/w/h), read it
	{
		ra->ReadAvatarFromPacket(data, 6);
	}
	
	
	game->mMap->AddEntity(ra);

	return ra;
}

void _removeRemoteActor(RemoteActor* a)
{
	if (!a) return;

	Image* img;
	if (a->GetAvatar())
	{
		img = a->GetAvatar()->GetImage();
		rect r = a->GetBoundingRect();
		new ExplodingEntity(a->mMap, img, point2d(r.x, r.y));
	}
	
	if (userlist)
		userlist->RemoveNick(a->mName);
		
	a->mMap->RemoveEntity(a);
}

uShort timer_destroyMapStamp(timer* t, uLong ms)
{
	Entity* e = (Entity*)t->userData;
	
	if (game->mMap)
		game->mMap->RemoveEntity(e);
	
	return TIMER_DESTROY;	
}

void _stampMapText(sShort x, sShort y, sShort rotation, string text)
{
	if (!game->mMap)
		return;
		
	text = text.substr(0, 40);
	
	color c;
	
	TextObject* to = new TextObject();
		to->mId = "stamp";
		to->mMap = game->mMap;
		to->SetLayer(EntityManager::LAYER_GROUND);
		to->mMap->AddEntity(to);
		to->SetText(text, 20, rotation);	
		to->SetPosition( point2d(x, y) );
	
	// 5 minutes until the stamp cleans itself up
	timers->Add("stampDie", 5*60*1000, false, timer_destroyMapStamp, NULL, to);
}

/*	Net Senders */

void netSendSay(string text) //say $message
{
	if (game->IsMapLoading() || text.empty()) return;

	//do a few modifications
	if (text.find_last_of(">") == 0 && text.find("<", 0) == string::npos)
		text.insert(0, "\\c090");

	if (text.at(0) != '/') //Ignore slash commands 
	{
		game->mChat->AddMessage(game->mPlayer->mName + ": " + text);
		game->mMap->mBubbles.CreateBubble(game->mPlayer, text);
		
		achievement_NeedASpamBlocker();
		
		gui->GetUserAttention(); //in case they still want sound while they send a msg
		gui->mGetUserAttention = false; //hack to stop the title from flashing when we talk
	}
	else
	{	
		//Dispatch a command message
		MessageData md("ENTITY_CMD");
		md.WriteUserdata("entity", game->mPlayer);
		md.WriteString("message", text);
		messenger.Dispatch(md, game->mPlayer);
	}
	
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("say");
		data.SetKey( game->mNet->GetChannel()->mEncryptionKey );

		data.WriteString(text);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendAchievement(string title) // ern $title (earn, get it, get it!?)
{
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("ern");
		data.SetKey( game->mNet->GetChannel()->mEncryptionKey );

		data.WriteString(title);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendStamp(string text) //stp x y rotation color $text
{
	if (!game->mMap) return;

	//text = stripCodes(text);
	point2d p = game->mPlayer->GetPosition();
	sShort rotation = rnd2(-15, 15);
	
	_stampMapText(p.x, p.y, rotation, text);

	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("stp");
		data.SetKey( game->mNet->GetChannel()->mEncryptionKey );

		data.WriteInt(p.x);
		data.WriteInt(p.y);
		data.WriteInt(rotation);
		data.WriteString(text);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendMe(string text) //act $text
{
	if (!game->mMap) return;

	text = stripCodes(text);

	game->mChat->AddMessage("\\c909* " + stripCodes(game->mPlayer->mName) 
							+ " " + text + " *");

	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("act");
		data.SetKey( game->mNet->GetChannel()->mEncryptionKey );

		data.WriteString(text);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendMusic(string song) //act 1 song
{
	if (!game->mMap) return;
	
	song = stripCodes(song);

	game->mChat->AddMessage(
					"\\c099* " + stripCodes(game->mPlayer->mName)
					+ " is listening to \\c059" 
					+ song + " \\c099*" 
			);
		
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("act");
		data.SetKey( game->mNet->GetChannel()->mEncryptionKey );

		data.WriteInt(1);
		data.WriteString(song);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendBeat(Entity* target, string item) //act 2 target item
{
	if (!game->mMap || !target) return;

	string targetName = stripCodes(target->mName);
	item = stripCodes(item);
	
	game->mChat->AddMessage(
					"\\c484 * " + stripCodes(game->mPlayer->mName) + " beats "
					+ stripCodes(targetName) + " with a \\c784"
					+ item + "\\c484 *"
				);

	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("act");
		data.SetKey( game->mNet->GetChannel()->mEncryptionKey );

		data.WriteInt(2);
		data.WriteString(targetName);
		data.WriteString(item);
		
		game->mNet->MessageToChannel( data.ToString() );
	}	
}

void netSendAvatar(Avatar* a) //avy $url #w #h ...
{
	if (!a || !game || !game->mNet || game->mNet->GetState() != ONCHANNEL)
		return;
		
	DataPacket data("avy");
	data.SetKey( game->mNet->GetChannel()->mEncryptionKey );
	
	a->Serialize(data);

	game->mNet->MessageToChannel( data.ToString() );
}

void netSendEmote(uShort num) //emo num
{
	if (!game->mMap) return;

	game->mPlayer->Emote(num);
		
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("emo");
		data.SetKey( game->mNet->GetChannel()->mEncryptionKey );

		data.WriteInt(num);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

/*	This user sent a valid message, but we don't have them on the map. Request for add */
void _handleUnknownUser(string& nick)
{
	/*	TODO: Make sure they're actually in our channel! What if they sent
		a message from somewhere outside our channel? */
	if ( timers->Find("pushNm") )
		return;
			
	if (game->mPlayer->mName != nick)
		game->mPlayer->NetSendState(nick, "sup");

	//make sure we don't send this TOO often. Only once every couple seconds
	timers->Add("pushNm", 2*1000, false, NULL, NULL);
}

/*	Net Message Listeners */

void _handleNetMessage_TradeDeny(string& nick, DataPacket& data) //trDNY reason 
{
	if (data.Size() < 1)
	{
		console->AddMessage("Malformed 'trDNY' from " + nick);
		return;
	}
	
	string msg = "\\c900 * Trade with " + nick + " has closed (Reason: " + data.ReadString(0) + ")";
	printMessage(msg);
	
	//If we were trading with this person and they cancelled it, close everything on our side.
	ItemTrade* i = (ItemTrade*)gui->Get("ItemTrade");
	if (i && i->mTrader == nick)
	{
		i->Die();
		inventory->mTrade->SetVisible(false);
	}
}

void _handleNetMessage_TradeOkay(string& nick, DataPacket& data) //trOK
{
	//make sure we haven't gotten in a trade with someone else before the acceptance
	if (gui->Get("ItemTrade"))
	{
		//send auto-deny if so
		if (game->mNet && game->mNet->GetState() == ONCHANNEL)
		{
			DataPacket data("trDNY");
			data.SetKey( game->mNet->GetChannel()->mEncryptionKey );
			data.WriteString("Already in a trade");
			game->mNet->Privmsg( nick, data.ToString() );
		}
		return;
	}
	
	string msg = "\\c090 * Trade with " + nick + " has been accepted!";
	printMessage(msg);
	
	new ItemTrade(nick);
}

void _handleNetMessage_TradeReady(string& nick, DataPacket& data) //trRDY
{
	ItemTrade* trade = (ItemTrade*)gui->Get("ItemTrade");
	if (trade && trade->mTrader == nick)
		trade->RemoteReady();
}

void _handleNetMessage_TradeItem(string& nick, DataPacket& data) //trITM id description amount cost
{
	ItemTrade* trade = (ItemTrade*)gui->Get("ItemTrade");

	if (trade && trade->mTrader == nick)
	{
		trade->SetRemoteItem(data.ReadString(0), data.ReadString(1), data.ReadInt(2), data.ReadInt(3));
	}
}

void _handleNetMessage_TradeRequest(string& nick, DataPacket& data) //trREQ
{
	handleInboundTradeRequest(nick);
}

void _handleNetMessage_Say(string& nick, DataPacket& data)
{
	if (!game->mMap || data.Size() < 1) return;
	
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	if (!ra) { _handleUnknownUser(nick); return; }

	//if they're too far, ignore
	if ( !ra->IsVisibleInCamera() )
		return;
		
	string msg = data.ReadString(0);
	
	
	if (msg.at(0) != '/') //Ignore slash commands 
	{
		game->mChat->AddMessage(nick + ": " + msg);
		game->mMap->mBubbles.CreateBubble(ra, msg);
	}
	else
	{	
		//Dispatch a command message
		MessageData md("ENTITY_CMD");
		md.WriteUserdata("entity", ra);
		md.WriteString("message", msg);
		messenger.Dispatch(md, ra);
	}

	gui->GetUserAttention();
}

void _handleNetMessage_Stamp(string& nick, DataPacket& data)
{
	if (!game->mMap || data.Size() < 4) return;
	
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	if (!ra) { _handleUnknownUser(nick); return; }

	_stampMapText(data.ReadInt(0), data.ReadInt(1), data.ReadInt(2), data.ReadString(3));
}

void _handleNetMessage_Act(string& nick, DataPacket& data)
{
	if (!game->mMap || data.Size() < 1) return;
	
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	if (!ra) { _handleUnknownUser(nick); return; }

	//if they're too far, ignore
	if ( !ra->IsVisibleInCamera() )
		return;
		
	string msg;
	
	if (data.Size() < 2) //check for basic /me: act 0 msg OR act msg
	{
		msg = "\\c909* " + stripCodes(nick) 
			+ " " + stripCodes(data.ReadString(0)) + " *";
	}
	else //Check for variants
	{
		switch (data.ReadInt(0))
		{
			case 1: //music command: act 1 song
				msg = "\\c099* " + stripCodes(nick)
					+ " is listening to \\c059" 
					+ stripCodes( data.ReadString(1) ) + " \\c099*";
				break;
			case 2: //beat command: act 2 target item
				msg = "\\c484 * " + stripCodes(nick) + " beats "
					+ stripCodes( data.ReadString(1) ) + " with a \\c784"
					+ stripCodes( data.ReadString(2) ) + "\\c484 *";
				break;
			default: break;
		}
	}
	
	printMessage(msg);
}

void _handleNetMessage_Avy(string& nick, DataPacket& data)
{
	if (!game->mMap) return;
	
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	if (!ra) { _handleUnknownUser(nick); return; }
	
	ra->ReadAvatarFromPacket(data, 0);
}

void _handleNetMessage_Sup(string& nick, DataPacket& data)
{
	if (!game->mMap) return;
	
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	if (ra)
	{
		console->AddMessage("Double 'sup' from " + ra->mName);
		return;
	}
	
	/*	Add them to our map w/ their state data. Then reply
		with a "nothing much" edition of our state
	*/
	ra = _addRemoteActor(nick, data);
	if (ra)
	{
		game->mPlayer->NetSendState("", "nm");
		DEBUGOUT("\\c090" + ra->mName + " has been added to the map");
	}
	else
	{
		console->AddMessage("Malformed 'sup' from " + nick);	
	}
}

void _handleNetMessage_Nm(string& nick, DataPacket& data)
{
	if (!game->mMap) return;
	
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);

	if (ra)
	{
		console->AddMessage("Double 'nm' from " + ra->mId);
		return;
	}

	/*	Add them to our map w/ their state data.
		Don't reply.
	*/
	ra = _addRemoteActor(nick, data);
	if (ra)
	{
		DEBUGOUT("\\c090" + ra->mName + " has been added to the map");
	}
	else
	{
		console->AddMessage("Malformed 'nm' from " + nick);	
	}
}

void _handleNetMessage_Ach(string& nick, DataPacket& data) // ach $id $desc #max
{
	if (data.Size() != 4)
	{
		console->AddMessage("Malformed 'ach' from " + nick);
		return;
	}
	
	game->EarnAchievement(data.ReadString(0), data.ReadString(1), data.ReadInt(2), data.ReadString(3));
}

void _handleNetMessage_PlayerEarnedAchievement(string& nick, DataPacket& data) // ern $title
{
	if (!game->mMap) return;
		
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	if (!ra) { _handleUnknownUser(nick); return; }
	
	if (data.Size() != 1)
	{
		console->AddMessage("Malformed 'ern' from " + nick);
		return;
	}

	game->mChat->AddMessage("\\c139 * " + nick + "\\c999 achieved: \\c080 " + data.ReadString(1));
	ra->Emote(11);
}

void _handleNetMessage_Mov(string& nick, DataPacket& data)
{
	if (!game->mMap) return;
	
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	if (!ra) { _handleUnknownUser(nick); return; }

	if (data.Size() != 3)
	{
		console->AddMessage("Malformed 'mov' from " + ra->mName);
		return;
	}

	/*	write our target coords into the buffer along with it. FORMAT: cXXXXX.YYYYYY.
		Reason we queue up the check in the buffer instead of writing it raw is that our computer
		may take longer to process than it takes another computer to send. And if we jumped coordinates
		every mov recv, we'd get jerky movement.
	*/
	ra->AddToActionBuffer( 'c' + its(data.ReadInt(0)) + "."
							+ its(data.ReadInt(1)) + "."
							+ decompressActionBuffer( data.ReadString(2) ) );
	
}

void _handleNetMessage_Emo(string& nick, DataPacket& data)
{
	if (!game->mMap) return;
	
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	if (!ra) { _handleUnknownUser(nick); return; }

	if (data.Size() != 1)
	{
		console->AddMessage("Malformed 'emo' from " + ra->mName);
		return;
	}

	ra->Emote( data.ReadInt(0) );
}

void _handleNetMessage_Lua(string& nick, DataPacket& data)
{
	if (!game->mMap) return;
	
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	if (!ra) { _handleUnknownUser(nick); return; }
	
	if (data.Size() != 2)
	{
		console->AddMessage("Malformed 'lua' from " + ra->mName);
		return;
	}
	
	string id = data.ReadString(0);
	string msg = data.ReadString(1);
	
	//Dispatch it and let lua listeners pick it up
	MessageData md;
	md.SetId("NET_LUA");
	md.WriteString("id", id);
	md.WriteString("message", msg);
	
	messenger.Dispatch(md, ra);
}

void _handleNetMessage_Mod(string& nick, DataPacket& data) //avatar mod: mod #
{
	if (!game->mMap) return;
	
	RemoteActor* ra = (RemoteActor*)game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	if (!ra) { _handleUnknownUser(nick); return; }

	if (!ra->GetAvatar())
	{
		console->AddMessage("'mod' without Avatar from " + ra->mName);
		return;
	}

	if (data.Size() != 1)
	{
		console->AddMessage("Malformed 'mod' from " + ra->mName);
		return;
	}

	ra->GetAvatar()->Modify( data.ReadInt(0) );
}

void _handleNetMessage_Private(string& nick, string& msg)
{
	//if we don't accept private messages, send an auto respond
	if ( game->mPlayerData.GetParamInt("map", "privmsg") != 1 )
	{
		game->mNet->Privmsg( nick, "\\c900I currently have private messages blocked" );
	}
	else
	{
		Console* c = game->GetPrivateChat(nick);
		c->AddMessage(nick + ": " + msg);
		gui->GetUserAttention();
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
	data.SetKey( game->mNet->GetChannel()->mEncryptionKey );
	
	//Couldn't decrypt. Assume plaintext and privmsg
	if (!data.FromString(msg))
	{
		_handleNetMessage_Private(nick, msg);
		return;
	}

	string id = data.mId;
	
#ifdef DEBUG
	//Output our analyzed packet data
	string out = "[" + nick + "] {" + data.mId + "} ";
	for (int i = 0; i < data.Size(); i++)
		out += data.ReadString(i) + " & ";
	console->AddMessage(out);
#endif

	//Send our message to functions built for it
	if (id == "say")
		_handleNetMessage_Say(nick, data);
	else if (id == "act")
		_handleNetMessage_Act(nick, data);
	else if (id == "avy")
		_handleNetMessage_Avy(nick, data);
	else if (id == "sup")
		_handleNetMessage_Sup(nick, data);
	else if (id == "nm")
		_handleNetMessage_Nm(nick, data);
	else if (id == "mov")
		_handleNetMessage_Mov(nick, data);
	else if (id == "emo")
		_handleNetMessage_Emo(nick, data);
	else if (id == "stp")
		_handleNetMessage_Stamp(nick, data);
	else if (id == "lua")
		_handleNetMessage_Lua(nick, data);
	else if (id == "ern")
		_handleNetMessage_PlayerEarnedAchievement(nick, data);
	else if (id == "mod")
		_handleNetMessage_Mod(nick, data);
	else if (id == "ach")
		_handleNetMessage_Ach(nick, data);
	else if (id == "trDNY")
		_handleNetMessage_TradeDeny(nick, data);
	else if (id == "trOK") 
		_handleNetMessage_TradeOkay(nick, data);
	else if (id == "trRDY")
		_handleNetMessage_TradeReady(nick, data);
	else if (id == "trITM")
		_handleNetMessage_TradeItem(nick, data);
	else if (id == "trREQ")
		_handleNetMessage_TradeRequest(nick, data);
	else if (target == game->mPlayer->mName)
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
			msg = "\\c900" + md.ReadString("message");
			printMessage(msg);
			game->UnloadMap();
			break;
		default:
			msg = "\\c900 * [" + md.ReadString("command") + "] " + md.ReadString("message");
			printMessage(msg);
			break;	
	}
}

void listener_NetError(MessageListener* ml, MessageData& md, void* sender)
{
	string msg = "\\c900 * " + md.ReadString("message");
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
			
		msg += " has joined.";
		
		printMessage(msg);
	}
	
	//If it's not us, send our data to them
	if (game->mPlayer->mName != nick)
	{
		game->mPlayer->NetSendState(nick, "sup");
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
	
	if (nick == game->mPlayer->mName) //I was kicked. Will rejoin immediately
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
			
		msg += " leaves.";
		
		printMessage(msg);
	}
	
	//Remove them from our map if they were on it
	Entity* e = game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
	_removeRemoteActor((RemoteActor*)e);
}

void listener_NetChannelQuit(MessageListener* ml, MessageData& md, void* sender)
{
	string msg;
	
	string nick = md.ReadString("nick");
	
	if (game->mShowJoinParts)
	{
		msg = "\\c139 * " + nick + "\\c139";
		
		if (game->mShowAddresses)
			msg += " (" + md.ReadString("address") + ")";	
			
		msg += " Quit (" + md.ReadString("reason") + ")";
		
		printMessage(msg);
	}
	
	if (nick == game->mPlayer->mName)
	{
		msg = "User Quit";
		printMessage(msg);
		//SAFEDELETE(mChannel);
		//Disconnect();
	}
	else if (game->mMap)
	{
		//Remove them from our map if they were on it
		Entity* e = game->mMap->FindEntityByName(nick, ENTITY_REMOTEACTOR);
		_removeRemoteActor((RemoteActor*)e);
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
	msg = "\\c900 * Could not connect to " + net->mHost + ": " + its(net->mPort);
	printMessage(msg);

	net->TryNextServer();
}

void listener_NetNotice(MessageListener* ml, MessageData& md, void* sender)
{
	string msg;
	msg = "\\c900-" + md.ReadString("nick") + "\\c900- " + md.ReadString("message");
	
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
	msg = "\\c900 * (" + md.ReadString("command") + ") " + md.ReadString("line");
	
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
	string oldn = md.ReadString("oldnick");
	string newn = md.ReadString("newnick");

	Entity* e;
	if (oldn == game->mPlayer->mName)
		e = game->mPlayer;
	else
		e = game->mMap->FindEntityByName(oldn, ENTITY_REMOTEACTOR);
	
	if (!e)
		return;
		
	e->mName = newn;
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

	switch (state)
	{
		case CONNECTING:
			game->mChat->AddMessage("\\c090* Connecting to server...");
			break;
		case AWAITINGSERVERVERIFY: //Connected, awaiting verify. Send nick.
			game->mChat->AddMessage("\\c139* Waiting for verification");
			net->ChangeNick(game->mPlayer->mName);
			break;
		case ONSERVER:
			break;
		case ONCHANNEL:
			DEBUGOUT("ONCHANNEL");
			if (game->mLoader)
				game->mLoader->SetState(WorldLoader::WORLD_READY);
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
	
	string msg = "\\c900" + md.ReadString("message");
	printMessage(msg);
	
	string newnick;
	newnick = "fro_" + generateWord(6);
	
	if (userlist)
		userlist->ChangeNick(game->mPlayer->mName, newnick);

	game->mPlayer->mName = newnick;
	game->mPlayerData.SetParamString("user", "nick", newnick);
	net->ChangeNick(newnick);
}

// Nothing in message
void listener_NetVerified(MessageListener* ml, MessageData& md, void* sender)
{
	game->LoadOnlineWorld(game->mStartingWorldId);
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
}
