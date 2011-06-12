
#include "IrcNetSenders.h"
#include "IrcNetListeners.h"
#include "../core/net/IrcNet2.h"
#include "../core/net/DataPacket.h"
#include "../core/io/FileIO.h"
#include "../game/GameManager.h"
#include "../entity/LocalActor.h"
#include "../avatar/Avatar.h"

const int MAX_STAMP_TEXT_LENGTH = 30;


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

void netSendSay(string text) //say $message
{
	// disable the usage of special shit in say
	replace(&text, "\\n", "");
	replace(&text, "\\t", "");
	replace(&text, "\n", "");
	replace(&text, "\t", "");
	
	if (game->IsMapLoading() || text.empty()) return;

	//do a few modifications
	if (text.find_last_of(">") == 0 && text.find("<", 0) == string::npos)
		text.insert(0, "\\c561");

	MessageData md;
	if (text.at(0) != '/')
	{
		md.SetId("NET_SAY");
	}
	else
	{	
		md.SetId("NET_CMD");
	}
	
	md.WriteUserdata("entity", game->mPlayer);
	md.WriteString("message", text);
	messenger.Dispatch(md, game->mPlayer);
	
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("say");
		data.SetKey( game->mNet->GetEncryptionKey() );

		data.WriteString(text);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendAchievement(string title) // ern $title (earn, get it, get it!?)
{
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("ern");
		data.SetKey( game->mNet->GetEncryptionKey() );

		data.WriteString(title);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendStamp(string text) //stp x y rotation color $text
{
	if (!game->mMap) return;
	
	replace(&text, "\\n", "");
	replace(&text, "\\t", "");
	replace(&text, "\n", "");
	replace(&text, "\t", "");

	//text = stripCodes(text);
	point2d p = game->mPlayer->GetPosition();
	sShort rotation = rnd2(-15, 15);
	
	if (text.length() > MAX_STAMP_TEXT_LENGTH)
		text.erase(MAX_STAMP_TEXT_LENGTH);
	
	stampMapText(p.x, p.y, rotation, text);

	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("stp");
		data.SetKey( game->mNet->GetEncryptionKey() );

		data.WriteInt(p.x);
		data.WriteInt(p.y);
		data.WriteInt(rotation);
		data.WriteString(text);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendMe(string text) //act #mode $text
{
	if (!game->mMap) return;

	text = stripCodes(text);

	game->GetChat()->AddMessage("\\c909* " + stripCodes(game->mPlayer->GetName()) 
							+ " " + text + " *");

	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("act");
		data.SetKey( game->mNet->GetEncryptionKey() );

		data.WriteChar(0);
		data.WriteString(text);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendMusic(string song) //act 1 song
{
	if (!game->mMap) return;
	
	song = stripCodes(song);
	
	game->GetChat()->AddMessage(
					"\\c099* " + stripCodes(game->mPlayer->GetName())
					+ " is listening to \\c059" 
					+ song + " \\c099*" 
			);
		
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("act");
		data.SetKey( game->mNet->GetEncryptionKey() );

		data.WriteChar(1);
		data.WriteString(song);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendBeat(Entity* target, string item) //act 2 target item
{
	if (!game->mMap || !target) return;

	string targetName = stripCodes(target->GetName());
	item = stripCodes(item);

	game->GetChat()->AddMessage(
					"\\c484 * " + stripCodes(game->mPlayer->GetName()) + " beats "
					+ stripCodes(targetName) + " with a \\c784"
					+ item + "\\c484 *"
				);

	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("act");
		data.SetKey( game->mNet->GetEncryptionKey() );

		data.WriteChar(2);
		data.WriteString(targetName);
		data.WriteString(item);
		
		game->mNet->MessageToChannel( data.ToString() );
	}	
}

void netSendAvatar(Avatar* a, string nick) //avy $url #w #h ...
{
	if (!a || !game || !game->mNet || game->mNet->GetState() != ONCHANNEL)
		return;
		
	DataPacket data("avy");
	data.SetKey( game->mNet->GetEncryptionKey() );
	
	a->Serialize(data);

	if (nick.empty())
		game->mNet->MessageToChannel( data.ToString() );
	else
		game->mNet->Privmsg( nick, data.ToString() );
}

void netSendEmote(uShort num) //emo num
{
	if (!game->mMap) return;

	timer* t = timers->Find("emowait");
	if (t)
	{
		game->GetChat()->AddMessage("\\c900 * Spam emotes less, jerk.");
		return;
	}
	else
	{
		timers->Add("emowait", 1000, false, NULL, NULL, NULL);
	}

	game->mPlayer->Emote(num);
		
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("emo");
		data.SetKey( game->mNet->GetEncryptionKey() );

		data.WriteChar(num);

		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendRequestAvatar(string nick)
{
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("reqAvy");
		data.SetKey( game->mNet->GetEncryptionKey() );

		game->mNet->Privmsg(nick, data.ToString() );
	}
}

const int NMREPLY_RESET_MS = 3*1000;

void netSendState(string targetNick, string header) //header VERSION $id #x #y #dir #action Avatar Stuff
{
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		LocalActor* a = game->mPlayer;
		
    	if (header == "nm")
    	{
    		if ( timers->Find("resetNm") )
    			return;
    
    		timers->Add("resetNm", NMREPLY_RESET_MS, false, NULL, NULL);
    	}
    	
    	DataPacket data(header);
    	data.SetKey( game->mNet->GetEncryptionKey() );
    	
    	data.WriteString(game->mNet->GetChannel()->mId);
    	data.WriteString(a->GetName());
    	data.WriteInt(a->mPosition.x);
    	data.WriteInt(a->mPosition.y);
    	data.WriteChar(a->mDirection);
    	data.WriteChar(a->mAction);
    
    	//if our avatar loaded but has yet to swap, swap it to send properly.
    	a->CheckLoadingAvatar();
    
    	//Add our avatar to the outbound message
    	if (a->mAvatar)
    		a->mAvatar->Serialize(data);
    	else if (a->mLoadingAvatar)
    		a->mLoadingAvatar->Serialize(data);
    
    	if (targetNick.empty())
    		game->mNet->MessageToChannel( data.ToString() );
    	else
    		game->mNet->Privmsg( targetNick, data.ToString() );
    }
}

void netSendActionBuffer() //mov #x #y $buffer
{
	LocalActor* a = game->mPlayer;

	DEBUGOUT("Sending: (" + its(a->mLastSavedPosition.x) 
				+ "," + its(a->mLastSavedPosition.y)
				+ ") " + a->mOutputActionBuffer);
			
	// add position correction to the end of the buffer
	a->mOutputActionBuffer += 'c' + its(a->mPosition.x) + '.' + its(a->mPosition.y) + '.';
	
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("mov");
		data.SetKey( game->mNet->GetEncryptionKey() );
	
		data.WriteInt(a->mLastSavedPosition.x);
		data.WriteInt(a->mLastSavedPosition.y);
		
		//Appends speed in the beginning in order to keep speed synced more or less
		data.WriteString( compressActionBuffer( ((a->mSpeed == SPEED_WALK) ? 'w' : 'r') + a->mOutputActionBuffer ) );
	
		game->mNet->MessageToChannel( data.ToString() );
	}
}

void netSendAvatarMod()
{
	LocalActor* a = game->mPlayer;
	
	if (a->GetAvatar() && game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("mod");
		data.SetKey( game->mNet->GetEncryptionKey() );
		
		data.WriteChar(a->GetAvatar()->mModifier);
		
		game->mNet->MessageToChannel( data.ToString() );
	}
}



