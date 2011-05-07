
#ifdef WIN32
#include <windows.h> //for Winamp support
#endif

#include "GameManager.h"
#include "../core/net/IrcNet2.h"
#include "../avatar/Avatar.h"
#include "../entity/LocalActor.h"
#include "../net/IrcNetSenders.h"
#include "../map/Map.h"

void callback_chatNoCommand(Console* c, string s)
{
	if (s == "/mini")
	{
		game->mPlayer->SetAvatarModifier(Avatar::MOD_MINI);
		netSendAvatarMod();
	}
	else if (s == "/normal")
	{
		game->mPlayer->SetAvatarModifier(Avatar::MOD_NONE);
		netSendAvatarMod();
	}
	else if (s == "/ghost")
	{
		game->mPlayer->SetAvatarModifier(Avatar::MOD_GHOST);
		netSendAvatarMod();
	}
#ifdef DEBUG
	else if (s == "/giant")
	{
		game->mPlayer->SetAvatarModifier(Avatar::MOD_GIANT);
		netSendAvatarMod();
	}
#endif
	else
	{
		if (!game->mMap)
			c->AddMessage(s);
		else
			netSendSay(s);
	}
}

void callback_chatCommandMe(Console* c, string s)
{
	if (s.length() < 5)
		return;

	netSendMe(s.substr(4));
}

void callback_chatCommandStamp(Console* c, string s)
{
	if (s.length() < 8)
		return;

	netSendStamp(s.substr(7));
}

void callback_chatCommandEmote(Console* c, string s)
{
	if (s.length() < 6)
		return;

	netSendEmote(sti(s.substr(5)));
}

void callback_chatCommandPos(Console* c, string s)
{
	c->AddMessage("Player: " + its(game->mPlayer->mPosition.x) + ", " + its(game->mPlayer->mPosition.y));
	c->AddMessage("Cursor (Real): " + its(gui->GetMouseX()) + ", " + its(gui->GetMouseY()));
	
	rect r = game->mMap->ToCameraPosition(gui->GetMouseRect());
	c->AddMessage("Cursor (Map): " + its(r.x) + ", " + its(r.y));
}

#ifdef WIN32
BOOL CALLBACK callback_findWinamp(HWND hwnd, LPARAM lParam) //for the below code
{
	char buffer[255];
	 
	GetWindowText(hwnd, buffer, sizeof(buffer));
    string title = buffer;
	
	int pos = title.find(" - Winamp", 0);
	if (pos != string::npos)
	{
		//found it, clean it and send it
		title.erase(pos, title.size() - pos);
		netSendMusic(title);
		return FALSE;
	}

	return TRUE; //keep searching
}
#endif //WIN32

void callback_chatCommandListeningTo(Console* c, string s)
{
	//they manually specified a song
	if (s.length() > 7)
	{
		netSendMusic(s.substr(7));
		return;
	}
	
	//otherwise, let's try to find Winamp!
#ifdef WIN32
	if (EnumWindows(callback_findWinamp, 0) == TRUE) //didn't find it
	{
		c->AddMessage("\\c900 * Can't find Winamp! Do /music <title> instead!");
	}
#else
	c->AddMessage("\\c900 * Only supported in Windows! Do /music <title> instead!");
#endif

}

void callback_chatCommandCoolface(Console* c, string s) { netSendEmote(1); }
void callback_chatCommandBrofist(Console* c, string s) { netSendEmote(2); }
void callback_chatCommandSpoilereyes(Console* c, string s) { netSendEmote(3); }
void callback_chatCommandBaww(Console* c, string s) { netSendEmote(4); }
void callback_chatCommandDerp(Console* c, string s) { netSendEmote(5); }
void callback_chatCommandHappy(Console* c, string s) { netSendEmote(6); }
void callback_chatCommandOmg(Console* c, string s) { netSendEmote(7); }
void callback_chatCommandFFFUUU(Console* c, string s) { netSendEmote(8); }
void callback_chatCommandHeart(Console* c, string s) { netSendEmote(9); }
void callback_chatCommandAwesome(Console* c, string s) { netSendEmote(10); }
void callback_chatCommandWtf(Console* c, string s) { netSendEmote(12); }
void callback_chatCommandTroll(Console* c, string s) { netSendEmote(13); }
void callback_chatCommandFacepalm(Console* c, string s) { netSendEmote(14); }
void callback_chatCommandPedo(Console* c, string s) { netSendEmote(15); }
void callback_chatCommandDatAss(Console* c, string s) { netSendEmote(16); }
void callback_chatCommandRage(Console* c, string s) { netSendEmote(17); }

/*
void callback_chatCommandJoin(Console* c, string s)
{
	if (s.length() < 7)
		return;

	if (!game->mNet->IsConnected())
	{
		c->AddMessage("\\c900* No server connection! Could not jump worlds!");
		return;
	}

	if (game->mLoader->m_state != WorldLoader::WORLD_ACTIVE
		&& game->mLoader->m_state != WorldLoader::FAILED)
	{
		c->AddMessage("\\c900* Already loading a world!");
		return;	
	}
	
#ifndef DEBUG
	timer* t = timers->Find("joinwait");
	if (t)
	{
		int seconds = t->lastMs + t->interval - gui->GetTick();
		if (seconds < 0)
			seconds = 0;
		else
			seconds /= 1000;

		c->AddMessage("\\c900 * You must wait " + its(seconds+1) + " seconds.");
		return;
	}
#endif

	s = stripCodes(s.substr(6));
	c->AddMessage("\\c090* Loading " + s);

	game->LoadOnlineWorld(s);

#ifndef DEBUG
	//limit the number of times they can change channels
	timers->Add("joinwait", JOIN_INTERVAL_MS, false, NULL, NULL, NULL);
#endif
}


void callback_chatCommandNick(Console* c, string s) // /nick nickname
{	
	if (s.length() < 7)
		return;
		
	s = s.substr(6);
	
	replace(&s, "\\n", ""); //filter out \n 
	
	if (stripCodes(s).empty())
		return;
	
	if (game->mNet->IsConnected())
		game->mNet->ChangeNick(s);
	else
		game->mPlayer->mName = s;
	
	TiXmlElement* e = game->mPlayerData.mDoc.FirstChildElement("data")->FirstChildElement("user");
	game->mPlayerData.SetParamString(e, "nick", s);
	game->SavePlayerData();
}

void callback_chatCommandWorldsViewer(Console* c, string s)
{	
	//if (!gui->Get("WorldViewer"))
	//	new WorldsViewer();
}
*/
void callback_chatCommandMsg(Console* c, string s) // /msg nick message
{
	vString v;
	explode(&v, &s, " ");
	if (v.size() < 3)
	{
		c->AddMessage("Syntax: /msg nick message");
		return;
	}
	
	if (game->mNet)
		game->mNet->Privmsg(v.at(1), s.substr( s.find(v.at(2))) );
}

void callback_chatCommandListEmotes(Console* c, string s)
{
	c->AddMessage("\\c990Emotes:\\n  /facepalm, /datass, /rage, /troll, /coolface, /brofist, /spoilereyes, /sad, /derp, /happy, /omg, /fff, /heart, /awesome, /wtf, /pedo");
}

void callback_chatCommandListCommands(Console* c, string s)
{
	c->AddMessage("\\c990Commands:\\n  /emotes - Display emotes list\\n  /save - Save chat log to an html file\\n"
							"  /clear - Clear the chatbox\\n  /exit - Close the program\\n  /ss - Save a screenshot (same as PRTSCRN)\\n"
							"  /stamp <text> - Stamp the text onto the map\\n  /nick <name> - Change your nickname\\n"
							"  /music <text> - Tell everyone the song you're listening to\\n"
							"  /me <text> - Tell everyone what you're currently doing");
}

void hookChatCommands(Console* c)
{
	c->HookCommand("", callback_chatNoCommand);
	c->HookCommand("/me", callback_chatCommandMe);
	c->HookCommand("/stamp", callback_chatCommandStamp);
	c->HookCommand("/music", callback_chatCommandListeningTo);
//	c->HookCommand("/join", callback_chatCommandJoin);
//	c->HookCommand("/nick", callback_chatCommandNick);
//	c->HookCommand("/worlds", callback_chatCommandWorldsViewer);
	c->HookCommand("/msg", callback_chatCommandMsg);
	c->HookCommand("/emo", callback_chatCommandEmote);
	c->HookCommand("/pos", callback_chatCommandPos);

	//Emotes
	c->HookCommand("/coolface", callback_chatCommandCoolface);
	c->HookCommand("/troll", callback_chatCommandTroll);
	c->HookCommand("/brofist", callback_chatCommandBrofist);
	c->HookCommand("/spoilereyes", callback_chatCommandSpoilereyes);
	c->HookCommand("/sad", callback_chatCommandBaww);
	c->HookCommand("/derp", callback_chatCommandDerp);
	c->HookCommand("/happy", callback_chatCommandHappy);
	c->HookCommand("/omg", callback_chatCommandOmg);
	c->HookCommand("/wtf", callback_chatCommandWtf);
	c->HookCommand("/fff", callback_chatCommandFFFUUU);
	c->HookCommand("/heart", callback_chatCommandHeart);
	c->HookCommand("/awesome", callback_chatCommandAwesome);
	c->HookCommand("/facepalm", callback_chatCommandFacepalm);
	c->HookCommand("/pedo", callback_chatCommandPedo);
	c->HookCommand("/datass", callback_chatCommandDatAss);
	c->HookCommand("/rage", callback_chatCommandRage);
	
	c->HookCommand("/emotes", callback_chatCommandListEmotes);
	c->HookCommand("/commands", callback_chatCommandListCommands);
}





