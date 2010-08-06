
#ifndef _LUNEMPARTY_H_
#define _LUNEMPARTY_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "ActorStats.h"

#define MAX_PARTY_SLOTS 4

class Lunem;
class Label;
class LunemParty : public Frame
{
  public:
	LunemParty();
	~LunemParty();
	
	/**	@return false if the slot is occupied or invalid, true otherwise */
	bool SetSlot(int slot, Lunem* lu);
	
	/**	@return false if the slot was empty or invalid, true otherwise */
	bool ClearSlot(int slot);
	
	/**	@return false if the party is full */
	bool AddLunem(Lunem* lu);
	
	/**	@return NULL if the slot is empty or invalid, the lunem pointer otherwise */
	Lunem* GetSlot(int slot) const;
	
	bool IsFull() const;
	bool HasLivingMember() const;
		
	/**
		Does a RecalculateStats() for each lunem in the party. Called every time
		the map is built, in order to apply stat adjustments on a per-map basis,
		and to take advantage of the lua listeners in the map scripts.
	*/
	void RecalculateAllStats();
		
	void LoadFromDisk();
	void SaveToDisk() const;
	
	void SetHelpText(string msg);
	
	/*	Instead of SetMenuMode for use item, call this so we can store the ID */
	void SetUseItemMode(string id);
	
	/*	Sets the menu mode for all ActorStats children */
	void SetMenuMode(int mode);

	int MatchSlotWithActorStats(ActorStats* stats);
	void SendDuelSwapEvent(ActorStats* stats);
	void SendUseItemEvent(ActorStats* stats);

	Label* mInfoLabel;
	
	ActorStats* mMyInfoBar; // info bar for the player
	
	ActorStats* mInfoBar[MAX_PARTY_SLOTS];
	Lunem* mParty[MAX_PARTY_SLOTS];
	
	string mUseItemId; //item ID for use item on party member mode
};

#endif //_LUNEMPARTY_H_
