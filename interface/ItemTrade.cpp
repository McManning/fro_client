
#include "ItemTrade.h"
#include "../entity/RemoteActor.h"
#include "../game/GameManager.h"
#include "../core/Core.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/net/IrcNet2.h"
#include "../core/net/DataPacket.h"

#ifdef TRADE_ENABLED
/*	Dialog that forces the user to accept or deny a trade.
	If they accept, it'll tell the remote nick and activate ItemTrade. Else, it'll tell the remote nick that we denied.
 */
class TradeRequest : public Frame 
{
  public:
	TradeRequest(string nick);
	~TradeRequest();
	
	void Render();
	
	void SendAccept();
	void SendDeny(string reason);
	
	string mNick;
	
	Label* mTimeout;
};

#endif

/*	Someone is trying to trade with us, see if they can */
void handleInboundTradeRequest(RemoteActor* ra)
{
	string reason;
	
#ifdef TRADE_ENABLED	
	//if we're already in a trade, or auto-deny trade requests, tell them.
	if ( gui->Get("TradeRequest") || gui->Get("ItemTrade") )
	{
		reason = "Already in a trade";
	}
	else if (game->mPlayerData.GetParamInt("map", "trading") == 0)
	{
		reason = "Does not accept trade requests";	
	}
	else if (ra->IsBlocked())
	{
		reason = "You are blocked";	
	}
#else
	reason = "Trade Disabled";
#endif
	
	//denied
	if (!reason.empty())
	{
		if (game->mNet && game->mNet->GetState() == ONCHANNEL)
		{
			DataPacket data("trDNY");
			data.SetKey( game->mNet->GetEncryptionKey() );
			data.WriteString(reason);
			game->mNet->Privmsg( ra->mName, data.ToString() );
		}
	}
	else
	{
#ifdef TRADE_ENABLED
		new TradeRequest(ra->mName);
#endif
	}
}

void handleOutboundTradeRequest(string nick)
{	
#ifdef TRADE_ENABLED
	if ( gui->Get("TradeRequest") || gui->Get("ItemTrade") )
	{
		game->mChat->AddMessage("\\c900 * You're already in a trade");
		return;
	}
	
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("trREQ");
		data.SetKey( game->mNet->GetEncryptionKey() );
		game->mNet->Privmsg( nick, data.ToString() );
	}
#else
	game->mChat->AddMessage("Trading Disabled");
#endif
}

#ifdef TRADE_ENABLED
void callback_TradeRequestAccept(Button* b)
{
	TradeRequest* t = (TradeRequest*)b->GetParent();

	new ItemTrade(t->mNick);
	t->SendAccept();
	
	t->Die();
}

void callback_TradeRequestDeny(Button* b)
{
	TradeRequest* t = (TradeRequest*)b->GetParent();

	t->SendDeny("Denied Request");
	
	t->Die();
}

//Count down until auto-close
uShort timer_TradeRequestTimeout(timer* t, uLong ms)
{
	TradeRequest* r = (TradeRequest*)t->userData;
	
	if (!r)
		return TIMER_DESTROY;
		
	if (15 - t->runCount < 1)
	{
		r->SendDeny("Request Ignored");
		r->Die();
		return TIMER_DESTROY;
	}
	
	r->mTimeout->SetCaption( its(15 - t->runCount) + "sec" );
	return TIMER_CONTINUE;
}

TradeRequest::TradeRequest(string nick)
	: Frame(gui, "TradeRequest", rect(0,0,100,100), "Trade Request", true, false, false, true)
{
	mNick = nick;
	
	//set width based on label caption, up to max
	uShort w;
	uShort h;

	Label* l;
	l = new Label(this, "", rect(5, 30));
	l->SetCaption(nick + "\\c000 requests a trade. Accept?");
		
	//set width based on label caption, up to max
	w = l->Width() + 10;
	h = l->Height() + 60;

	SetSize(w, h);
		
	//center label
	l->Center();
	
	mTimeout = new Label(this, "", rect(Width()/2-20, Height()-25));
		mTimeout->SetCaption("15sec");
		
	Button* b;
	b = new Button(this, "", rect(10,Height()-25,20,20), "", callback_TradeRequestAccept);
		b->mHoverText = "Accept";
		b->SetImage("assets/buttons/okay.png");

	b = new Button(this, "", rect(Width()-35,Height()-25,20,20), "", callback_TradeRequestDeny);
		b->mHoverText = "Deny";
		b->SetImage("assets/buttons/no.png");

	ResizeChildren();
	Center();
	DemandFocus();
	
	timers->Add("TradeRequestTimeout", 1000, false, timer_TradeRequestTimeout, NULL, this);
}

TradeRequest::~TradeRequest()
{
	timers->RemoveMatchingUserData(this);	
}

void TradeRequest::Render()
{
	//if (gui->GetDemandsFocus() == this)
	//	gui->RenderDarkOverlay();
	
	Frame::Render();
}

void TradeRequest::SendAccept()
{
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		//Tell the remote trader to bring up their trade window
		DataPacket data("trOK");
		data.SetKey( game->mNet->GetEncryptionKey() );
		game->mNet->Privmsg( mNick, data.ToString() );
	}
}

void TradeRequest::SendDeny(string reason)
{
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		//tell them we denied
		DataPacket data("trDNY");
		data.SetKey( game->mNet->GetEncryptionKey() );
		data.WriteString("Denied Request");
		game->mNet->Privmsg( mNick, data.ToString() );
	}
}

/*	Dialog that forces the user to input an amount for the item they're going to trade */
class AmountRequest : public Frame 
{
  public:
	AmountRequest(ItemTrade* trade, itemProperties* item);
	void Render();
	
	uShort mMax;
	ItemTrade* mTrade;
	Input* mInput;
};

void callback_AmountRequestAccept(Button* b)
{
	AmountRequest* a = (AmountRequest*)b->GetParent();
	
	uShort amt = sti(a->mInput->GetText());
	
	//DO SOMETHING
	if (amt > a->mMax || amt < 1)
	{
		new MessagePopup("", "Invalid", "Invalid Amount. Must be 1 to " + its(a->mMax));
	}
	else
	{
		a->mTrade->UpdateMyItem(amt);
		a->Die();
	}
	
	
}

AmountRequest::AmountRequest(ItemTrade* trade, itemProperties* item)
	: Frame(gui, "AmountRequest", rect(0,0,100,100), "Set Amount", true, false, false, true)
{
	mMax = item->amount;
	mTrade = trade;
	
	//set width based on label caption
	uShort w;
	uShort h;

	Label* l;
	l = new Label(this, "", rect(5, 30, 0, 0));
	l->SetCaption("How much of \\c009" + item->id + "\\c000 do you wish to trade?");

	mInput = new Input(this, "amt", rect(5,55,100,20), "0123456789", 0, true, NULL);
		mInput->mHoverText = "1 to " + its(mMax);
		mInput->SetText("1");
		mInput->SetMenuEnabled(false);
	
	//set width based on label caption, up to max
	w = l->Width() + 10;
	h = l->Height() + 70;

	SetSize(w, h);

	Button* b;
	b= new Button(this, "", rect(Width()-35,Height()-25,20,20), "", callback_AmountRequestAccept);
		b->mHoverText = "Accept";
		b->SetImage("assets/buttons/okay.png");

	ResizeChildren();
	Center();
	DemandFocus();
}

void AmountRequest::Render()
{
	//if (gui->GetDemandsFocus() == this)
	//	gui->RenderDarkOverlay();
	
	Frame::Render();
}


void inventory_remoteTradeCallback(Button* b)
{
	ItemTrade* i = (ItemTrade*)inventory->mTradeDialog;

	i->SetMyItem(inventory->GetSelected());	
}

void callback_ItemTradeReady(Button* b)
{
	ItemTrade* i = (ItemTrade*)b->GetParent();
	
	i->TradeReady();
}

void callback_ItemTradeCancel(Button* b)
{
	ItemTrade* i = (ItemTrade*)b->GetParent();
	
	i->CancelTrade();
}

void callback_ItemTradePrivmsg(Button* b)
{
	ItemTrade* i = (ItemTrade*)b->GetParent();

	Console* c = game->GetPrivateChat(i->mTrader);
	if (c)
		c->SetPosition(rect(gui->Width() - c->Width(), 0, c->Width(), c->Height()));
}

ItemTrade::ItemTrade(string trader)
	: Frame(gui, "ItemTrade", rect(0,0,200,160), "Trade", true, false, false, true)
{
	Center();
	
	mTrader = trader;
	mRemoteReady = false;
	mReady = false;
	
	Label* l;
	Button* b;
	
	uShort y = 30;
	
	l = new Label(this, "", rect(5, y));
		l->SetCaption("My Item");
	y += 25;
	
	mMyItemLabel = new Label(this, "", rect(15, y));
		mMyItemLabel->SetCaption("\\c009Waiting for item");
	y += 25;

	l = new Label(this, "", rect(5, y));
		l->SetCaption(trader + "\\c000's Item");
	y += 25;
	
	mRemoteItemLabel = new Label(this, "", rect(15, y));
		mRemoteItemLabel->SetCaption("\\c009Waiting for item");
	y += 25;
	
	b= new Button(this, "", rect(10,Height()-25,20,20), "", callback_ItemTradeReady);
		b->mHoverText = "Ready To Trade";
		b->SetImage("assets/buttons/okay.png");

	b= new Button(this, "", rect(Width()-30,Height()-25,20,20), "", callback_ItemTradeCancel);
		b->mHoverText = "Cancel Trade";
		b->SetImage("assets/buttons/no.png");
	
	b = new Button(this, "privmsg", rect(90, Height()-25, 20, 20), "", callback_ItemTradePrivmsg);
		b->mHoverText = "Open Private Chat";
		b->SetImage("assets/buttons/options_user.png");
	
	//bring the inventory up, and set it to be NEXT TO our trade dialog
	inventory->SetVisible(true);
	inventory->Center();
	
	rect r = inventory->GetPosition();
	r.x = mPosition.x - r.w;
	inventory->SetPosition(r);
	
	inventory->mTrade->SetVisible(true);
	inventory->mTrade->onClickCallback = inventory_remoteTradeCallback;
	inventory->mTradeDialog = this;
}

ItemTrade::~ItemTrade()
{
	inventory->mTrade->SetVisible(false);
	inventory->SetActive(true);
	inventory->mTradeDialog = NULL;
}

void ItemTrade::CancelTrade()
{
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		//send our cancel to the trader
		DataPacket data("trDNY");
		data.SetKey( game->mNet->GetEncryptionKey() );
		data.WriteString("Cancelled the trade");
	
		game->mNet->Privmsg( mTrader, data.ToString() );
	}
	
	game->mChat->AddMessage("\\c900 * Trade with " + mTrader + "\\c900 has been cancelled.");
	inventory->mTrade->SetVisible(false);
	Die();
}

void ItemTrade::DoTrade()
{
	//delete our item from inventory and add what they gave us
	//output trade success to console
	
	inventory->Erase(mMyItem.id, mMyItem.amount, false);
	
	if (!mRemoteItem.id.empty())
	{
		inventory->Add(mRemoteItem.id, mRemoteItem.description, mRemoteItem.amount);
		//game->mChat->AddMessage("\\c900 * Gained " + its(mRemoteItem.amount) + "x " + mRemoteItem.id + "\\c900!");	
	}
	
	Die();
}

void ItemTrade::TradeReady()
{	
	if (mReady) //so we don't keep sending trRDY
		return;
	
	mReady = true;

	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		//Send our ready state to the trader
		DataPacket data("trRDY");
		data.SetKey( game->mNet->GetEncryptionKey() );
		game->mNet->Privmsg( mTrader, data.ToString() );
	
	}
	
	SetCaption("Trade - Ready");
	inventory->mTrade->SetVisible(false);
	inventory->SetActive(false);
	
	CheckForTradeOkay();
}

void ItemTrade::RemoteReady()
{
	if (mRemoteReady)
		return;
		
	mRemoteReady = true;
	SetCaption("Trade - They're Ready");
	
	CheckForTradeOkay();
}

void ItemTrade::CheckForTradeOkay()
{
	if (mRemoteReady && mReady)
	{	
		DoTrade();
		Die(); //don't need trade window anymore
	}
}

void ItemTrade::SetMyItem(itemProperties* item)
{
	if (!item)
		return;
		
	mMyItem = *item;
	
	//update label
	mMyItemLabel->SetCaption("Waiting for amount");
	
	//request user to input an amount adjustment
	new AmountRequest(this, &mMyItem);
}

void ItemTrade::UpdateMyItem(uShort amt)
{
	mMyItem.amount = amt;
	mMyItemLabel->SetCaption("x" + its(mMyItem.amount) + " " + mMyItem.id);
	
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		DataPacket data("trITM");
		data.SetKey( game->mNet->GetEncryptionKey() );
		data.WriteString(mMyItem.id);
		data.WriteString(mMyItem.description);
		data.WriteInt(mMyItem.amount);
		game->mNet->Privmsg( mTrader, data.ToString() );
	}
}

void ItemTrade::SetRemoteItem(string id, string desc, uShort amt)
{
	mRemoteItem.id = id;
	mRemoteItem.description = desc;
	mRemoteItem.amount = amt;

	//update label
	mRemoteItemLabel->SetCaption("x" + its(amt) + " " + id);
}


#endif //TRADE_ENABLED
