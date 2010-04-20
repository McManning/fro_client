
#include "RemotePlayerMenu.h"
#include "../core/widgets/Button.h"
#include "../core/net/IrcNet2.h"
#include "../game/GameManager.h"
#include "../game/IrcNetListeners.h"
#include "../map/Map.h"
#include "../entity/LocalActor.h"
#include "../entity/RemoteActor.h"
#include "../interface/Inventory.h"
#include "../interface/ItemTrade.h"

void callback_playerMenuBeat(Button* b)
{
	RemotePlayerMenu* r = (RemotePlayerMenu*)b->GetParent();
	r->Die(); //queue up menu deletion
	game->mMap->SetKeyFocus();
	
	//get random inventory item, beat r->mLinked->mName with it if item != NULL
	if (!inventory || game->mMap != r->mMap) return;
	
	//Make sure we're not waiting for a beat timer to finish up
	timer* t = timers->Find("rpbeat");
	if (t)
	{
		int seconds = (t->lastMs + t->interval - gui->GetTick()) / 1000;
		game->mChat->AddMessage("\\c900 * You must wait " + its(seconds) + " seconds.");
		return;
	}

	if (!r->mLinked)
		return;
		
	//if too far, tell them!
	if (getDistance(game->mPlayer->GetPosition(), r->mLinked->GetPosition()) > 100)
	{
		game->mChat->AddMessage("\\c900 * You're too far to hit your target!");	
		return;
	}

	itemProperties* i = inventory->GetRandom();
	if (i)
	{
		netSendBeat(r->mLinked, i->id);
	}
	else
	{
		game->mChat->AddMessage("\\c900 * You don't have any items to use!");
	}
	
	//limit the number of times they can use the beat command
	timers->Add("rpbeat", BEAT_INTERVAL_MS, false, NULL, NULL, NULL);
}

void callback_playerMenuPrivmsg(Button* b)
{
	RemotePlayerMenu* r = (RemotePlayerMenu*)b->GetParent();
	r->Die(); //queue up menu deletion

	if (!r->mLinked) return;
	
	//TODO: Open up a private plaintext convo dialog with the target user. 
	game->GetPrivateChat(r->mLinked->mName);
}

void callback_playerMenuTrade(Button* b)
{
	RemotePlayerMenu* r = (RemotePlayerMenu*)b->GetParent();
	r->Die(); //queue up menu deletion

	if (game->mMap != r->mMap || !r->mLinked) return;
	
	//TODO: Open up a private plaintext convo dialog with the target user. 
	handleOutboundTradeRequest(r->mLinked->mName);
}

void callback_playerMenuToggleBlock(Button* b)
{
	RemotePlayerMenu* r = (RemotePlayerMenu*)b->GetParent();
	r->Die(); //queue up menu deletion

	if (game->mMap != r->mMap || !r->mLinked) return;
	
	r->mLinked->SetBlocked(!r->mLinked->IsBlocked());
}

RemotePlayerMenu::RemotePlayerMenu(Map* map, RemoteActor* linked)
	: Frame(gui, "RemotePlayerMenu", rect(0,0,200,100), linked->mName, true, false, true, true)
{
	mLinked = linked;
	mMap = map;

	Button* b;
	string s;

	rect r(10, 30, Width()-20, 20);
	
	b = new Button(this, "", r, "Beat", callback_playerMenuBeat);
	r.y += 25;

	b = new Button(this, "", r, "Private Message", callback_playerMenuPrivmsg);
	r.y += 25;

	b = new Button(this, "", r, "Request Trade", callback_playerMenuTrade);
	r.y += 25;
	
	if (linked->IsBlocked())
		s = "Unblock Player";
	else
		s = "Block Player";

	b = new Button(this, "", r, s, callback_playerMenuToggleBlock);
	r.y += 25;

	r.y += 5;
	
	SetSize(Width(), r.y);
	ResizeChildren();
	Center();
	
	DemandFocus();
}
