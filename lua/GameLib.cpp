
#include <lua.hpp>
#include "GameLib.h"
#include "LuaCommon.h"
#include "../game/GameManager.h"
#include "../map/Map.h"
#include "../core/net/IrcNet2.h"
#include "../core/net/DataPacket.h"

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
		data.SetKey( game->mNet->GetChannel()->mEncryptionKey );

		data.WriteString( lua_tostring(ls, 1) );
		data.WriteString( lua_tostring(ls, 2) );

		game->mNet->MessageToChannel( data.ToString() );
	}
}

//	.NetSendToNick("nick", "id", "message")
//		Send a custom packet over the network to the specified nick
int game_NetSendToNick(lua_State* ls)
{
	PRINT("game_NetSendToNick");
	luaCountArgs(ls, 3);

	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("lua");
		data.SetKey( game->mNet->GetChannel()->mEncryptionKey );
		
		data.WriteString( lua_tostring(ls, 2) );
		data.WriteString( lua_tostring(ls, 3) );
		
		game->mNet->Privmsg( lua_tostring(ls, 1), data.ToString() );
	}
}

static const luaL_Reg functions[] = {
	{"Print", game_Print},
	{"NetSendToChannel", game_NetSendToChannel},
	{"NetSendToNick", game_NetSendToNick},
	{NULL, NULL}
};

void RegisterGameLib(lua_State* ls)
{
	luaL_register( ls, "Game", functions );
}


