
#ifndef _INVENTORY_H_
#define _INVENTORY_H_

#define MAX_ITEM_ID 30
#define MAX_ITEM_DESCRIPTION 200

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

struct itemProperties 
{
	string id;
	string description;
	uShort amount;

	TiXmlElement* element; //for reverse referencing
};


/*	Dialog that forces the user to input an amount for the item they're going to destroy */
class Input;
class IncinerateAmountRequest : public Frame 
{
  public:
	IncinerateAmountRequest(itemProperties* item);
	~IncinerateAmountRequest();
	void Render();
	
	itemProperties* mItem;
	Input* mInput;
};

class Button;
class Multiline;
class Inventory : public Frame 
{
  public:
	Inventory();
	~Inventory();
	
	void ResizeChildren();
	void SetPosition(rect r);

	itemProperties* Add(string id, string description, uShort amount);
	itemProperties* Find(string id);
	void Erase(sShort index, uShort amount, bool removeOnlyIfHasEnough);
	void Erase(string id, uShort amount, bool removeOnlyIfHasEnough);
	bool Has(string id, uShort amount);
	void Use(sShort index);

	/*	Returns NULL if nothing is selected. Else returns the proper itemProperties */
	itemProperties* GetSelected();
	
	/*	Returns a random item from our inventory. NULL if we don't have anything */
	itemProperties* GetRandom();
	
	bool Load();
	bool Save();
	
	void SetCash(int amount);
	int GetCash();
	
	void ItemSelected(); //Callback for mList
	
	itemProperties* _add(string id, string description, uShort amount);
	
	void RightClickSelected();
	void SwapItems(int a, int b);
	
	Button* mUse;
	Button* mDrop;
	Button* mTrade;
	Button* mCash;
	
	Multiline* mList;
	Multiline* mInfo;
	
	int mFirstItemIndex;

	std::vector<itemProperties*> mInventory;

	int mCashAmount;
	
	int mRandomAdditive;
	int mRandomMultiplier;
	
	//linked trade dialog. (Abstract Widget so we can use different types)
	Widget* mTradeDialog; 
};

extern Inventory* inventory;

#endif //_INVENTORY_H_
