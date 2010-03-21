
#include "RemotePlayerMenu.h"
#include "../map/Map.h"
#include "../entity/Actor.h"
#include "../entity/LocalActor.h"
#include "../core/widgets/Button.h"
#include "../interface/Inventory.h"
#include "../interface/ItemTrade.h"
#include "../game/GameManager.h"

#include "../game/IrcNetListeners.h"

void callback_playerMenuBeat(Button* b)
{
	RemotePlayerMenu* r = (RemotePlayerMenu*)b->GetParent();
	r->Die(); //queue up menu deletion
	game->mMap->SetKeyFocus();
	
	//get random inventory item, beat r->mLinked->mName with it if item != NULL
	if (!inventory || !game->mMap) return;
	
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

	if (!game->mMap || !r->mLinked) return;
	
	//TODO: Open up a private plaintext convo dialog with the target user. 
	game->GetPrivateChat(r->mLinked->mName);
}

void callback_playerMenuTrade(Button* b)
{
	RemotePlayerMenu* r = (RemotePlayerMenu*)b->GetParent();
	r->Die(); //queue up menu deletion

	if (!game->mMap || !r->mLinked) return;
	
	//TODO: Open up a private plaintext convo dialog with the target user. 
	handleOutboundTradeRequest(r->mLinked->mName);
}

void callback_playerMenuWhoops(Button* b)
{
	RemotePlayerMenu* r = (RemotePlayerMenu*)b->GetParent();
	r->Die(); //queue up menu deletion
	game->mMap->SetKeyFocus();
	
	game->mChat->AddMessage("\\c900Whoops, looks like this button doesn't work.");
}

RemotePlayerMenu::RemotePlayerMenu(Map* map, Actor* linked)
	: Frame(map, "remoteplayermenu", rect())
{
	mLinked = linked;
	mMap = map;
	/*
		Width = Actor bounding Rect Width + (button Width * 2)
		Height = however many max buttons on each side * button height
		x = linked entities x - button width
		y = linked entities y - height (bottoms will be lined up)
	*/

	rect r = linked->GetBoundingRect();
	r = map->ToScreenPosition(r);

	r.y -= (60 - r.h);

	rect rr = GetScreenPosition();
	r.x -= rr.x + 20;
	r.y -= rr.y;
	r.w += 40;
	r.h = 60;

	SetPosition(r);
	
	Button* b;
	
	uShort y = 0;
	uShort x = 0;
	
	string file = "assets/playermenu.png";
	
	b = new Button(this, "blockchat", rect(x, y, 20, 20), "", callback_playerMenuWhoops);
	b->mHoverText = "(Un)block chat";
		makeImage(b, "", file, rect(0,0,20,20),
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	y += 20;
	
	b = new Button(this, "blockavy", rect(x, y, 20, 20), "", callback_playerMenuWhoops);
	b->mHoverText = "(Un)block avatar";
		makeImage(b, "", file, rect(20,0,20,20),
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	y += 20;
	
	b = new Button(this, "beat", rect(x, y, 20, 20), "", callback_playerMenuBeat);
	b->mHoverText = "Beat with a random item";
		makeImage(b, "", file, rect(40,0,20,20),
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	y += 20;
	
	x = Width() - 20;
	y = 0;
				
	b = new Button(this, "close", rect(x, y, 20, 20), "", callback_closeFrame);
	b->mHoverText = "Close";
		makeImage(b, "", file, rect(60,0,20,20),
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	y += 20;
	
	b = new Button(this, "trade", rect(x, y, 20, 20), "", callback_playerMenuTrade);
	b->mHoverText = "Trade";
		makeImage(b, "", file, rect(80,0,20,20),
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	y += 20;
	
	b = new Button(this, "privmsg", rect(x, y, 20, 20), "", callback_playerMenuPrivmsg);
	b->mHoverText = "Open Private Chat";
		makeImage(b, "", file, rect(100,0,20,20),
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	y += 20;
}

RemotePlayerMenu::~RemotePlayerMenu()
{

}

void RemotePlayerMenu::Render(uLong ms)
{
/*	Image* scr = Screen::Instance();
	rect r = GetScreenPosition();
	
	scr->DrawRect(r, color(255,0,255), false);
*/
	Frame::Render(ms);	
}


void RemotePlayerMenu::Event(SDL_Event* event)
{
	Frame::Event(event);
	
	//If the user hits any keys, destroy this info frame
	if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP
		|| (event->type == SDL_MOUSEBUTTONDOWN && !HasMouseFocus()))
		Die();
}




