
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


#include <lua.hpp>
#include "GameLib.h"
#include "ActorLib.h" //for getReferencedActor
#include "LuaCommon.h"
#include "../game/GameManager.h"
#include "../map/Map.h"
#include "../core/net/IrcNet2.h"
#include "../core/net/DataPacket.h"
#include "../entity/RemoteActor.h"
#include "../net/IrcNetSenders.h"
//#include "../interface/ScreenText.h"
//#include "../interface/ActorStats.h"

// .Print("Message")
int game_Print(lua_State* ls)
{
	DEBUGOUT("game_Print");
	luaCountArgs(ls, 1);

	game->GetChat()->AddMessage( lua_tostring(ls, 1) );

	return 0;
}

//	.NetSendToChannel("id", "message")
//		Send a custom packet over the network to all members of the channel
int game_NetSendToChannel(lua_State* ls)
{
	DEBUGOUT("game_NetSendToChannel");
	luaCountArgs(ls, 2);

	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("lua");
		data.SetKey( game->mNet->GetEncryptionKey() );

		data.WriteString( lua_tostring(ls, 1) );
		data.WriteString( lua_tostring(ls, 2) );

		game->mNet->MessageToChannel( data.ToString() );
	}
}

//	.NetSendToPlayer(ent, "id", "message")
//		Send a custom packet over the network to the specified entity
int game_NetSendToPlayer(lua_State* ls)
{
	DEBUGOUT("game_NetSendToPlayer");
	luaCountArgs(ls, 3);

	RemoteActor* ra = (RemoteActor*)getReferencedActor(ls, 1);

	if (ra && ra->mType == ENTITY_REMOTEACTOR
		&& game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("lua");
		data.SetKey( game->mNet->GetEncryptionKey() );

		data.WriteString( lua_tostring(ls, 2) );
		data.WriteString( lua_tostring(ls, 3) );

		game->mNet->Privmsg( ra->GetName(), data.ToString() );
	}
}

// .NetSendEmote(#emote)
int game_NetSendEmote(lua_State* ls)
{
    luaCountArgs(ls, 1);
    netSendEmote((int)lua_tonumber(ls, 1));
    return 0;
}

//	.Version() - Return version number of the client as a string (ex: "1.2.1")
int game_Version(lua_State* ls)
{
	lua_pushstring(ls, VER_STRING);
	return 1;
}

//	.ToggleChat(bool) - Toggle visibility of the chatbox
int game_ToggleChat(lua_State* ls)
{
	luaCountArgs(ls, 1);
	game->GetChat()->SetVisible(lua_toboolean(ls, 1));
	return 0;
}

//	.ToggleHud(bool) - Toggles visibility of game hud and associated dialogs
int game_ToggleHud(lua_State* ls)
{
	luaCountArgs(ls, 1);
	game->ToggleHud(lua_toboolean(ls, 1));
	return 0;
}

/*
//	.SetScreenText("text", r, g, b, animationType, y<300>)
int game_SetScreenText(lua_State* ls)
{
	luaCountArgs(ls, 5);

	color c;
	string text;
	int animType;

	text = lua_tostring(ls, 1);
	c.r = (int)lua_tonumber(ls, 2);
	c.g = (int)lua_tonumber(ls, 3);
	c.b = (int)lua_tonumber(ls, 4);
	animType = (int)lua_tonumber(ls, 5);

	int args = lua_gettop(ls);
	int y = 300;
	if (args > 5)
		y = (int)lua_tonumber(ls, 6);

	new ScreenText(text, c, (ScreenText::animationType)animType, y);

	return 0;
}
*/
//	.ShowInfoBar("id", "text", duration, "imagefile"<nil>)
int game_ShowInfoBar(lua_State* ls)
{
	luaCountArgs(ls, 3);

	string imgfile;
	int args = lua_gettop(ls);
	if (args > 3)
		imgfile = lua_tostring(ls, 4);

	game->ShowInfoBar(lua_tostring(ls, 1), lua_tostring(ls, 2), (int)lua_tonumber(ls, 3), imgfile);
	return 0;
}

// .SetMode(mode) - Calls game->ToggleGameMode with the specified value. For swapping
//		between action, chat, duel, etc
int game_SetMode(lua_State* ls)
{
	game->ToggleGameMode((GameManager::gameMode)lua_tonumber(ls, 1));
	return 0;
}

//	mode = .GetMode()
int game_GetMode(lua_State* ls)
{
	lua_pushnumber(ls, (int)game->mGameMode);
	return 1;
}

/*
// ptr  = .NewStatsBar(actor, x, y)
int game_NewStatsBar(lua_State* ls)
{
	luaCountArgs(ls, 3);
	Actor* a = (Actor*)lua_touserdata(ls, 1);

	ActorStats* s = new ActorStats(game->mMap);
	s->SetMenuMode(ActorStats::DUEL_SCREEN_MENU);
	s->SetLinked(a);

	rect r = s->GetPosition();
	r.x = (int)lua_tonumber(ls, 2);
	r.y = (int)lua_tonumber(ls, 3);
	s->SetPosition(r);

	lua_pushlightuserdata(ls, s);
	return 1;
}

//	.RemoveStatsBar(ptr)
int game_RemoveStatsBar(lua_State* ls)
{
	luaCountArgs(ls, 1);

	if (!lua_isnil(ls, 1))
	{
		ActorStats* s = (ActorStats*)lua_touserdata(ls, 1);
		s->Die();
	}

	return 0;
}

// bool = .IsStatsBarDecreasing(ptr)
int game_IsStatsBarDecreasing(lua_State* ls)
{
	luaCountArgs(ls, 1);

	ActorStats* s = NULL;

	if (!lua_isnil(ls, 1))
		s = (ActorStats*)lua_touserdata(ls, 1);

	if (s)
		lua_pushboolean(ls, s->mCurrentHealth != s->mLinkedActor->m_iCurrentHealth);
	else
		lua_pushboolean(ls, false);

	return 1;
}
*/

//	x, y = .GetCursorPosition()
int game_GetCursorPosition(lua_State* ls)
{
	if (SDL_GetAppState() & SDL_APPMOUSEFOCUS) //if we have the mouse
	{
		lua_pushnumber(ls, gui->GetMouseX());
		lua_pushnumber(ls, gui->GetMouseY());
	}
	else
	{
		lua_pushnumber(ls, -1);
		lua_pushnumber(ls, -1);
	}

	return 2;
}

//	x, y = .GetCursorMapPosition()
int game_GetCursorMapPosition(lua_State* ls)
{
	if (SDL_GetAppState() & SDL_APPMOUSEFOCUS) //if we have the mouse
	{
		ASSERT(game->mMap);

		rect r = game->mMap->GetCameraPosition();
		lua_pushnumber(ls, gui->GetMouseX() + r.x);
		lua_pushnumber(ls, gui->GetMouseY() + r.y);
	}
	else
	{
		lua_pushnumber(ls, -1);
		lua_pushnumber(ls, -1);
	}

	return 2;
}

//	.OpenPrivateChat("nickname")
int game_OpenPrivateChat(lua_State* ls)
{
	luaCountArgs(ls, 1);
	game->GetPrivateChat( lua_tostring(ls, 1) );
}

static const luaL_Reg functions[] = {
	{"Print", game_Print},
	{"NetSendToChannel", game_NetSendToChannel},
	{"NetSendToPlayer", game_NetSendToPlayer},
	{"NetSendEmote", game_NetSendEmote},
	{"Version", game_Version},
	{"ToggleChat", game_ToggleChat},
	{"ToggleHud", game_ToggleHud},
//	{"SetScreenText", game_SetScreenText},
	{"ShowInfoBar", game_ShowInfoBar},
	{"SetMode", game_SetMode},
	{"GetMode", game_GetMode},
/*	{"NewStatsBar", game_NewStatsBar},
	{"RemoveStatsBar", game_RemoveStatsBar},
	{"IsStatsBarDecreasing", game_IsStatsBarDecreasing},*/
	{"GetCursorPosition", game_GetCursorPosition},
	{"GetCursorMapPosition", game_GetCursorMapPosition},
	{"OpenPrivateChat", game_OpenPrivateChat},
	{NULL, NULL}
};

void RegisterGameLib(lua_State* ls)
{
	luaL_register( ls, "Game", functions );
}


