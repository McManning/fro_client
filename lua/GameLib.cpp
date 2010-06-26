
#include <lua.hpp>
#include "GameLib.h"
#include "ActorLib.h" //for getReferencedActor
#include "LuaCommon.h"
#include "../game/GameManager.h"
#include "../map/Map.h"
#include "../core/net/IrcNet2.h"
#include "../core/net/DataPacket.h"
#include "../entity/RemoteActor.h"
#include "../interface/ScreenText.h"

// .Print("Message")
int game_Print(lua_State* ls)
{
	PRINT("game_Print");
	luaCountArgs(ls, 1);

	game->mChat->AddFormattedMessage( lua_tostring(ls, 1) );
	
	return 0;
}

//	.NetSendToChannel("id", "message")
//		Send a custom packet over the network to all members of the channel
int game_NetSendToChannel(lua_State* ls)
{
	PRINT("game_NetSendToChannel");
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
	PRINT("game_NetSendToPlayer");
	luaCountArgs(ls, 3);

	RemoteActor* ra = (RemoteActor*)getReferencedActor(ls, 1);

	if (ra && ra->mType == ENTITY_REMOTEACTOR 
		&& game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("lua");
		data.SetKey( game->mNet->GetEncryptionKey() );
		
		data.WriteString( lua_tostring(ls, 2) );
		data.WriteString( lua_tostring(ls, 3) );
		
		game->mNet->Privmsg( ra->mName, data.ToString() );
	}
}

//	.Version() - Return version number of the client as a string (ex: "1.2.1")
int game_Version(lua_State* ls)
{
	lua_pushstring(ls, APP_VERSION);
	return 1;	
}

//	.ToggleChat(bool) - Toggle visibility of the chatbox
int game_ToggleChat(lua_State* ls)
{
	luaCountArgs(ls, 1);
	game->mChat->SetVisible(lua_toboolean(ls, 1));
	return 1;
}

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

static const luaL_Reg functions[] = {
	{"Print", game_Print},
	{"NetSendToChannel", game_NetSendToChannel},
	{"NetSendToPlayer", game_NetSendToPlayer},
	{"Version", game_Version},
	{"ToggleChat", game_ToggleChat},
	{"SetScreenText", game_SetScreenText},
	{NULL, NULL}
};

void RegisterGameLib(lua_State* ls)
{
	luaL_register( ls, "Game", functions );
}


