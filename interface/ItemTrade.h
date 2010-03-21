
#ifndef _ITEMTRADE_H_
#define _ITEMTRADE_H_

#include "../core/widgets/Frame.h"
#include "Inventory.h"

class Label;
class ItemTrade : public Frame
{
  public:
	ItemTrade(string trader);
	~ItemTrade();
  
	/*	Closes this window and sends trDNY */
	void CancelTrade();
	
	/*	Actually trade the queued items and close this window */
	void DoTrade();

	/*	Ready button callback. Send trRDY privmsg to mTrader and set mReady to true. 
		Call CheckForTradeOkay to in case mRemoteReady is already true.  */
	void TradeReady();

	/*	When the trader sends us trRDY */
	void RemoteReady();
	
	/*	If mReady and mRemoteReady are both true, it'll finalize the trade */
	void CheckForTradeOkay();
	
	/*	Populate mRemoteItem and set label */
	void SetRemoteItem(string id, string desc, uShort amt, uShort cost);
	
	/*	Populates mMyItem and requests the user to input an adjusted amount */
	void SetMyItem(itemProperties* item);
	
	/*	Called from AmountRequest dialog. Will update the proper item info, and send trITM to mTrader */
	void UpdateMyItem(uShort amt);
	
	string mTrader; //nick that we're trading with
	
	//Item we're trading away
	itemProperties mMyItem;
	Label* mMyItemLabel;
	bool mReady; //Set to true with local button. 
	
	//Item we're gaining
	itemProperties mRemoteItem;
	Label* mRemoteItemLabel;
	bool mRemoteReady; //Set to true on TRADEOK msg.
};

/*	Someone is trying to trade with us, see if they can */
void handleInboundTradeRequest(string nick);

/*	We are trying to trade with nick */
void handleOutboundTradeRequest(string nick);

#endif //_ITEMTRADE_H_
