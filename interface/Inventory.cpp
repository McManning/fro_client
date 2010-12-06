
#include <lua.hpp>
#include "Inventory.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/RightClickMenu.h"
#include "../core/widgets/Scrollbar.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/widgets/Label.h"
#include "../lua/LuaCommon.h"
#include "../game/GameManager.h"
//#include "../game/Achievements.h"
#include "../map/Map.h"

Inventory* inventory;

void callback_inventoryUse(RightClickMenu* r, void* data)
{
	Inventory* i = (Inventory*)data;
	ASSERT(i);
	
	i->Use(i->mList->mSelected);
}

void callback_inventoryDrop(RightClickMenu* r, void* data)
{
	Inventory* i = (Inventory*)data;
	ASSERT(i);
	
	if (i->GetSelected())
		new IncinerateAmountRequest(i->GetSelected());
}

void callback_inventoryInfo(RightClickMenu* r, void* data)
{
	Inventory* i = (Inventory*)data;
	ASSERT(i);
}

void callback_inventoryListLeftClick(Multiline* m)
{
	Inventory* i = (Inventory*)m->GetParent();
	ASSERT(i);
	
	i->ItemSelected();
}

void callback_inventoryListRightClick(Multiline* m)
{
	Inventory* i = (Inventory*)m->GetParent();
	ASSERT(i);

	i->RightClickSelected();
}

int callback_inventoryXmlParser(XmlFile* xf, TiXmlElement* e, void* userData)
{
	Inventory* inv = (Inventory*)userData;
	ASSERT(inv)
	
	string id = e->Value();
	if (id == "item") //<item id="$" desc="$" amount="#"/>
	{
		inv->_add(xf->GetParamString(e, "id"), 	
					xf->GetParamString(e, "desc"), 
					xf->GetParamInt(e, "usetype"),
					xf->GetParamInt(e, "amount"));
	}
	
	return XMLPARSE_SUCCESS;
}

void callback_inventoryClose(Button* b) //override current close system
{	
		inventory->Save();	
	b->GetParent()->SetVisible(false);
}

void callback_IncinerateAmountRequestAccept(Button* b)
{
	IncinerateAmountRequest* a = (IncinerateAmountRequest*)b->GetParent();
	
	uShort amt = sti(a->mInput->GetText());

	if (amt > a->mItem->amount || amt < 1)
	{
		new MessagePopup("", "Invalid", "Invalid Amount. Must be 1 to " + its(a->mItem->amount));
	}
	else
	{
		inventory->Erase(a->mItem->id, amt, true);
		achievement_WasteNot(amt);
		a->Die();
	}	
}

void callback_IncinerateAmountRequestCancel(Button* b)
{
	IncinerateAmountRequest* a = (IncinerateAmountRequest*)b->GetParent();

	a->Die();
}

IncinerateAmountRequest::IncinerateAmountRequest(itemProperties* item)
	: Frame(gui, "IncinerateAmountRequest", rect(0,0,100,100), "Select Amount", true, false, false, true)
{
	mItem = item;

	//set width based on label caption
	uShort w;
	uShort h;

	Label* l;
	l = new Label(this, "", rect(5, 30, 0, 0));
	l->SetCaption("How much of \\c009" + item->id + "\\c000 do you wish to incinerate?");

	mInput = new Input(this, "amt", rect(5,55,100,20), "0123456789", 0, true, NULL);
		mInput->mHoverText = "1 to " + its(item->amount);
		mInput->SetText("1");
		mInput->SetMenuEnabled(false);
	
	//set width based on label caption
	w = l->Width() + 10;
	h = l->Height() + 70;

	SetSize(w, h);

	Button* b;
	b = new Button(this, "", rect(Width()-70,Height()-25,20,20), "", callback_IncinerateAmountRequestAccept);
		b->mHoverText = "Incinerate";
		b->SetImage("assets/buttons/okay.png");
					
	b = new Button(this, "", rect(Width()-35,Height()-25,20,20), "", callback_IncinerateAmountRequestCancel);
		b->mHoverText = "Cancel";
		b->SetImage("assets/buttons/no.png");

	ResizeChildren();
	Center();
	DemandFocus();
	
	inventory->SetActive(false);
}

IncinerateAmountRequest::~IncinerateAmountRequest()
{
	inventory->SetActive(true);
}

void IncinerateAmountRequest::Render()
{
	//if (gui->GetDemandsFocus() == this)
	//	gui->RenderDarkOverlay();
	
	Frame::Render();
}

Inventory::Inventory() :
	Frame(gui, "inventory", rect(), "[IN RENOVATION]", true, true, true, true)
{
	SetSize(250, 250);
	Center();

	//TODO: Randomize!
	mRandomMultiplier = 1;
	mRandomAdditive = 0;
	mTradeDialog = NULL;
	mFirstItemIndex = -1;
	
	mList = makeList(this, "list", rect(0,0,0,0));
		mList->onLeftSingleClickCallback = callback_inventoryListLeftClick;
		mList->onRightSingleClickCallback = callback_inventoryListRightClick;
		//mList->mScrollbar->mTabImage->mDst.h = 10;

//	mInfo = new Multiline(this, "info", rect(0,0,0,0));
		//mInfo->mScrollbar->mTabImage->mDst.h = 10; //shrink our scrollbar a bit to give more room

	//make bottom buttons
	mTrade = new Button(this, "trade", rect(0,0,20,20), "", NULL);
		mTrade->mHoverText = "Trade Selected";
		mTrade->SetImage("assets/buttons/inv_trade.png");
		mTrade->SetVisible(false);
	
/*	mDrop = new Button(this, "drop", rect(0,0,20,20), "", NULL);
		mDrop->mHoverText = "Incinerate Selected";
		mDrop->SetImage("assets/buttons/inv_drop.png");
	
	mUse = new Button(this, "use", rect(0,0,20,20), "", NULL);
		mUse->mHoverText = "Use Selected";
		mUse->SetImage("assets/buttons/inv_use.png");
*/
	mCash = new Button(this, "", rect(0,0,20,20), "", NULL);
		mCash->SetImage("assets/buttons/dorra.png");
			
	SetCash(0);
			
	ResizeChildren(); //get them into position

	inventory = this;

	Load();
	
	mList->SetTopLine(0); //set it to display the top of the list.
	
	if (mClose) //override current close method
		mClose->onClickCallback = callback_inventoryClose;
}

Inventory::~Inventory()
{
	for (int i = 0; i < mInventory.size(); i++)
		delete mInventory.at(i);	
	
	inventory = NULL;
}

void Inventory::SetCash(int amount)
{
	if (amount < 0) return;
	
	mCashAmount = (amount + mRandomAdditive) * mRandomMultiplier;
	mCash->mHoverText = "Dorra Total: " + its(GetCash());
}
	
int Inventory::GetCash()
{
	return (mCashAmount / mRandomMultiplier) - mRandomAdditive;
}

bool Inventory::Has(string id, uShort amount)
{
	id = id.substr(0, MAX_ITEM_ID);
	for (int i = 0; i < mInventory.size(); i++)
		if (mInventory.at(i)->id == id)
			return (mInventory.at(i)->amount >= amount);
			
	return false;
}

bool Inventory::Save()
{
	if (!game)
		return false;
		
	//Get master element
	TiXmlElement* top = game->mPlayerData.mDoc.FirstChildElement();
	TiXmlElement* e;
	
	//Erase old inventory data
	e = top->FirstChildElement("inventory");
	top->RemoveChild(e);

	//Write cash
	e = top->FirstChildElement("cash");
	game->mPlayerData.SetParamInt(e, "amount", GetCash());
		
	//add new inventory
	top = game->mPlayerData.AddChildElement(top, "inventory");
	
	//Write all the items in reverse (Newest last)
	for (int i = mInventory.size()-1; i >= 0; --i)
	{
		e = game->mPlayerData.AddChildElement(top, "item"); 
		game->mPlayerData.SetParamString(e, "id", mInventory.at(i)->id);
		game->mPlayerData.SetParamString(e, "desc", mInventory.at(i)->description);
		game->mPlayerData.SetParamInt(e, "usetype", mInventory.at(i)->useType);
		game->mPlayerData.SetParamInt(e, "amount", mInventory.at(i)->amount);
	}
	
	game->SavePlayerData();

	return true;
}

bool Inventory::Load()
{
	if (!game)
		return false;

	TiXmlElement* top = game->mPlayerData.mDoc.FirstChildElement();
	TiXmlElement* e;
	
	//Load Cash
	e = top->FirstChildElement("cash");
	SetCash( game->mPlayerData.GetParamInt(e, "amount") );

	//Load Inventory
	e = top->FirstChildElement("inventory");
	
	game->mPlayerData.SetParser(callback_inventoryXmlParser);
	bool success = (game->mPlayerData.ParseElement(e, this) == XMLPARSE_SUCCESS);

	return success;
}

itemProperties* Inventory::Add(string id, string description, int useType, int amount)
{
	string msg = id;
	if (amount > 1)
		msg += " (" + its(amount) + "x)";
	
	game->mChat->AddMessage("\\c090 * Gained " + msg);
	
	MessageData md("GAIN_ITEM");
	md.WriteString("id", id);
	md.WriteString("description", description);
	md.WriteInt("amount", amount);
	messenger.Dispatch(md, this);
	
	game->ShowInfoBar("item", msg, 7000, "assets/infobar_item.png");
	
	if (!Has(id, 1))
		achievement_TreasureHunter();

	itemProperties* prop = _add(id, description, useType, amount);
	Save();
	
	return prop;
}

itemProperties* Inventory::_add(string id, string description, int useType, int amount) 
{
	PRINT("Adding Item " + id + " DESC: " + description + " amt: " + its(amount));
	id = id.substr(0, MAX_ITEM_ID);
	description = description.substr(0, MAX_ITEM_DESCRIPTION);

	int i;
	
	//check for existing item, if one does, tack on amount and return
	for (i = 0; i < mInventory.size(); i++)
	{
		if (mInventory.at(i)->id == id)
		{
			mInventory.at(i)->amount += amount; //TODO: range check
			mList->SetLine(i, its(mInventory.at(i)->amount) + "x " 
							+ mInventory.at(i)->id);
			return mInventory.at(i);
		}
	}
	
	itemProperties* prop = new itemProperties;
	prop->id = id;
	prop->description = description;
	prop->useType = useType;
	if (amount < 1) 
		amount = 1;
	prop->amount = amount;

	//didn't find, add as a new one to the top
	mInventory.insert(mInventory.begin(), prop);

	//rewrite list
	mList->Clear();
	for (i = 0; i < mInventory.size(); i++)
		mList->AddMessage(its(mInventory.at(i)->amount) + "x " + mInventory.at(i)->id);

	return prop;
}

itemProperties* Inventory::Find(string id)
{
	for (int i = 0; i < mInventory.size(); i++)
	{
		if (mInventory.at(i)->id == id)
		{
			return mInventory.at(i);
		}
	}
	return NULL;
}

itemProperties* Inventory::GetRandom()
{
	if (mInventory.empty())
		return NULL;
	
	int index = rnd2(0, mInventory.size());
	return mInventory.at(index);
}

void Inventory::ItemSelected()
{
	if (mList->mSelected < 0 || mList->mSelected >= mList->mLines.size()) return;

	// do a swap
	if (mFirstItemIndex >= 0)
	{
		SwapItems(mFirstItemIndex, mList->mSelected);
	}
	else //prep for swap
	{
		mFirstItemIndex = mList->mSelected;
			
		//Set the info box to the description of the item
	//	mInfo->Clear();
		//mInfo->AddMessage(mInventory.at(mList->mSelected)->description);
	//	mInfo->SetTopLine(0); //set it to display the top of the message
	}
}

void Inventory::SwapItems(int a, int b)
{
	itemProperties* temp;
	
	temp = mInventory.at(a);
	mInventory.at(a) = mInventory.at(b);
	mInventory.at(b) = temp;
	
	// Refresh listing data
	mList->SetLine(a, its(mInventory.at(a)->amount) + "x " + mInventory.at(a)->id);
	mList->SetLine(b, its(mInventory.at(b)->amount) + "x " + mInventory.at(b)->id);
	
	mFirstItemIndex = -1;

}

void Inventory::RightClickSelected()
{
	// stop a swap in progress
	mFirstItemIndex = -1;
	
	RightClickMenu* m = new RightClickMenu();
		m->AddOption("Info", callback_inventoryInfo, this);
		m->AddOption("Incinerate", callback_inventoryDrop, this);
		m->AddOption("Use", callback_inventoryUse, this);
}

void Inventory::Erase(string id, uShort amount, bool removeOnlyIfHasEnough)
{
	id = id.substr(0, MAX_ITEM_ID);
	for (int i = 0; i < mInventory.size(); i++)
	{
		if (mInventory.at(i)->id == id)
		{
			Erase(i, amount, removeOnlyIfHasEnough);
		}
	}
}

void Inventory::Erase(sShort index, uShort amount, bool removeOnlyIfHasEnough)
{
	if (index < 0 || index >= mList->mLines.size()) return;
	

	if (amount <= mInventory.at(index)->amount || !removeOnlyIfHasEnough)
	{	
		game->mChat->AddMessage("\\c900 * Lost " + its(amount) 
								+ "x " + mInventory.at(index)->id + "!");

		MessageData md("LOSE_ITEM");
		md.WriteString("id", mInventory.at(index)->id);
		md.WriteInt("amount", amount);
		messenger.Dispatch(md, this);

		//reduce amount, if there's none left, erase.
		if (mInventory.at(index)->amount - amount < 1)
		{
			delete mInventory.at(index);
			mInventory.erase(mInventory.begin() + index);
			mList->EraseLine(index);
			
			if (mList->mSelected == index)
				mInfo->Clear(); //the selected items info would be in the box
		}
		else
		{
			mInventory.at(index)->amount -= amount;
			mList->SetLine(index, its(mInventory.at(index)->amount) + "x " 
								+ mInventory.at(index)->id);
		}
		
		Save();
	}
}

void Inventory::Use(sShort index)
{
	if (index < 0 || index >= mList->mLines.size()) return;

	MessageData md;
	
	if (game->IsInDuel())
	{
		md.SetId("DUEL_USE_ITEM");
	}
	else
	{
		md.SetId("USE_ITEM");
	}
		
	md.WriteString("id", mInventory.at(index)->id);
	messenger.Dispatch(md, this);
}

itemProperties* Inventory::GetSelected()
{
	if (mList->mSelected < 0 || mList->mSelected >= mList->mLines.size()) return NULL;
	
	return mInventory.at(mList->mSelected);
}

void Inventory::ResizeChildren() //overridden so we can move things around properly
{

	mList->SetPosition( rect(10, 30, Width() - 20, Height() - 65) );
	//mInfo->SetPosition( rect(10, 35 + mList->Height(), Width() - 20, 60) );
	
//	mUse->SetPosition( rect(Width()-43,Height()-25,20,20) );	
//	mDrop->SetPosition( rect(Width()-76,Height()-25,20,20) );
	mTrade->SetPosition( rect(Width()-109,Height()-25,20,20) );
	
	mCash->SetPosition( rect(5,Height()-25,20,20) );

	Frame::ResizeChildren(); //takes care of titlebar stuff (close button, sizer, caption, etc)
}

void Inventory::SetPosition(rect r)
{
	if (r.w > 135 && r.h > 224)
		Frame::SetPosition(r);
}

int Inventory::LuaReadItemTable(lua_State* ls, int virtualIndex)
{
	//Virtual index can NOT be an offset from the top, must be an absolute position.
	// This is because the lua_next() will screw up unless we specify an absolute.
	if (virtualIndex < 0)
		virtualIndex = lua_gettop(ls) + virtualIndex + 1;
	
	if (!lua_istable(ls, virtualIndex))
		return 0;
	
	string key, id, description;
	int amount = 1, useType = 0;
	
	lua_pushnil(ls);
	while (lua_next(ls, virtualIndex) != 0)
	{
		if (lua_isstring(ls, -2))
		{
			key = lua_tostring(ls, -2);
			if (key == "ID")
				id = lua_tostring(ls, -1);
			else if (key == "Description")
				description = lua_tostring(ls, -1);
			else if (key == "UseType")
				useType = (int)lua_tonumber(ls, -1);
			else if (key == "Amount")
				amount = (int)lua_tonumber(ls, -1);
		}
		lua_pop(ls, 1); //pop value	
	}
	
	// add item
	Add(id, description, useType, amount);

	return 1;
}

int Inventory::LuaWriteItemTable(lua_State* ls, string id)
{
	itemProperties* ip = Find(id);
	
	if (!ip)
	{
		lua_pushnil(ls);
	}
	else
	{
		//Construct a table to store all data
		lua_newtable(ls);
		int top = lua_gettop(ls);

		LUAT_ADD_STRING("ID", ip->id);
		LUAT_ADD_STRING("Description", ip->description);
		LUAT_ADD_INT("UseType", ip->useType);
		LUAT_ADD_INT("Amount", ip->amount);
	}

	return 1;
}

