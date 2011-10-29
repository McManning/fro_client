
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


#ifdef WIN32
#include <windows.h> //for Winamp support
#endif

#include "GameManager.h"
#include "../core/net/IrcNet2.h"
#include "../avatar/Avatar.h"
#include "../entity/LocalActor.h"
#include "../net/IrcNetSenders.h"
#include "../map/Map.h"

#include "../interface/UserList.h"

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
*/

void callback_chatCommandNick(Console* c, string s) // /nick nickname
{
	if (s.length() < 7)
		return;

	s = s.substr(6);

	if (s.find(" ", 0) != string::npos)
		s.erase(s.find(" ", 0));

	replace(&s, "\\n", "");
	replace(&s, "\\t", "");
	replace(&s, "\n", "");
	replace(&s, "\t", "");

	if (!stripCodes(s).empty())
	{
		if (game->mNet->IsConnected())
		{
			game->mNet->ChangeNick(s);
		}
		else
		{
			if (userlist)
				userlist->ChangeNick(game->mPlayer->GetName(), s);
			game->mPlayer->SetName(s);

			game->mUserData.SetValue("MapSettings", "Nick", s);
		}
	}
	else
	{
		game->GetChat()->AddMessage("\\c900 * Invalid nickname, ignoring.");
	}
}

/*
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
	c->HookCommand("/nick", callback_chatCommandNick);
//	c->HookCommand("/worlds", callback_chatCommandWorldsViewer);
	c->HookCommand("/msg", callback_chatCommandMsg);
	c->HookCommand("/pos", callback_chatCommandPos);

	c->HookCommand("/commands", callback_chatCommandListCommands);
}





