
#include "LunemParty.h"
#include "../entity/Lunem.h"
#include "../game/GameManager.h"
#include "../map/Map.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Label.h"

void callback_partyClose(Button* b)
{
	b->GetParent()->SetVisible(false);	
}

LunemParty::LunemParty() :
	Frame(gui, "party", rect(0,0,420,235), "My Party", true, false, true, true)
{
	rect r;
	
	r.x = 10;
	r.y = 35;
	r.w = Width() - 20;
	r.h = 35;
	
	mInfoLabel = new Label(this, "", r, "");
		mInfoLabel->mMaxWidth = r.w;
		
	r.x = 111;
	r.y = 70;
	r.w = 200;
	r.h = 50;
	mMyInfoBar = new ActorStats(this);
		mMyInfoBar->SetLinked((Actor*)game->mPlayer);
		mMyInfoBar->SetPosition(r);

	int i = 0;

	r.x = 5;
	for (int x = 0; x < 2; ++x)
	{
		r.y = 125;
		for (int y = 0; y < 2; ++y)
		{
			mInfoBar[i] = new ActorStats(this);
			mInfoBar[i]->SetLinked(NULL);
			mInfoBar[i]->SetPosition(r);
			mParty[i] = NULL;
			
			r.y += r.h + 5;
			++i;
		}
		r.x = Width() - r.w - 5;
	}

	LoadFromDisk();
	Center();
	
	if (mClose) //override current close method
		mClose->onClickCallback = callback_partyClose;
	
	SetMenuMode(ActorStats::PARTY_VIEW_MENU);
}

void LunemParty::SetHelpText(string msg)
{
	mInfoLabel->SetCaption(msg);
	
	rect r = mInfoLabel->GetPosition();
	r.x = Width() / 2 - r.w / 2;
	
	mInfoLabel->SetPosition(r);	
}

LunemParty::~LunemParty()
{
	SaveToDisk();	
	
	// clear slots
	for (int i = 0; i < MAX_PARTY_SLOTS; ++i)
	{
		if (mParty[i])
		{
			mParty[i]->mManagerCanDeleteMe = true;
			
			// If it's not on the map, delete instance
			if (!mParty[i]->mMap || !mParty[i]->mMap->FindEntity(mParty[i]))
				delete mParty[i];
			
			mParty[i] = NULL;
			mInfoBar[i]->SetLinked(NULL);
		}
	}
}

bool LunemParty::SetSlot(int slot, Lunem* lu)
{
	if (slot < 0 || slot >= MAX_PARTY_SLOTS || mParty[slot])
		return false;
		
	mParty[slot] = lu;
	mInfoBar[slot]->SetLinked(lu);
	
	lu->mManagerCanDeleteMe = false;
	
	return true;
}

bool LunemParty::ClearSlot(int slot)
{
	if (slot < 0 || slot >= MAX_PARTY_SLOTS || !mParty[slot])
		return false;
	
	Lunem* e = mParty[slot];
		
	e->mManagerCanDeleteMe = true;
		
	// if it's not on the map, delete instance
	if ( !( e->mMap && e->mMap->FindEntity(e) ) )
	{
		delete e;
	}

	// Shift slots and fill anything empty
	for (int i = 0; i < MAX_PARTY_SLOTS-1; ++i)
	{
		mParty[i] = mParty[i+1];
		mInfoBar[i]->SetLinked(mParty[i]);
	}
	
	// Last slot will always be null
	mParty[MAX_PARTY_SLOTS-1] = NULL;
	mInfoBar[MAX_PARTY_SLOTS-1]->SetLinked(NULL);
	
	return true;
}

bool LunemParty::AddLunem(Lunem* lu)
{
	// Add to the first empty slot
	for (int i = 0; i < MAX_PARTY_SLOTS; ++i)
	{
		if (!mParty[i])
			return SetSlot(i, lu);
	}
	
	return false;
}

Lunem* LunemParty::GetSlot(int slot) const
{
	if (slot < 0 || slot >= MAX_PARTY_SLOTS)
		return NULL;
		
	return mParty[slot];
}

void LunemParty::RecalculateAllStats()
{
	for (int i = 0; i < MAX_PARTY_SLOTS; ++i)
	{
		if (mParty[i])
			mParty[i]->RecalculateStats();
	}
}

bool LunemParty::IsFull() const
{
	for (int i = 0; i < MAX_PARTY_SLOTS; ++i)
	{
		if (!mParty[i])
			return false;
	}
	return true;
}

bool LunemParty::HasLivingMember() const
{
	for (int i = 0; i < MAX_PARTY_SLOTS; ++i)
	{
		if (mParty[i] && mParty[i]->m_iCurrentHealth > 0)
			return true;
	}
	return false;	
}

void LunemParty::LoadFromDisk()
{
	string filename = DIR_PROFILE;
	filename += "party.save";
	
	FILE* f = fopen(filename.c_str(), "rb");
	if (!f)
		return;
		
	char count;
	fread(&count, sizeof(count), 1, f);
		
	// Load all party members from the save file
	for (int i = 0; i < count && i < MAX_PARTY_SLOTS; ++i)
	{
		console->AddMessage("Reading in Lunem from disk...");
		mParty[i] = new Lunem;
			mParty[i]->ReadFromFile(f);
			mParty[i]->mManagerCanDeleteMe = false;
			
		mInfoBar[i]->SetLinked(mParty[i]);
	}
	
	fclose(f);
}

void LunemParty::SaveToDisk() const
{
	string filename = DIR_PROFILE;
	filename += "party.save";
	
	FILE* f = fopen(filename.c_str(), "wb");
	if (!f)
		return;

	int i;
	unsigned char count = 0;
	
	// count how many party members we really have
	for (i = 0; i < MAX_PARTY_SLOTS; ++i)
	{
		if (mParty[i])
			++count;
	}
	
	// write the number of party members we have
	fwrite(&count, sizeof(count), 1, f);
		
	// Write data from all party members to the file
	for (i = 0; i < count; ++i)
	{
		mParty[i]->WriteToFile(f);
	}
	
	fclose(f);
}

// Rather than SetMenuMode, use this so we can set an ID too
void LunemParty::SetUseItemMode(string id)
{
	mUseItemId = id;	
	SetMenuMode(ActorStats::USE_ITEM_MENU);
}

void LunemParty::SetMenuMode(int mode)
{
	mMyInfoBar->SetMenuMode(mode);
	for (int i = 0; i < MAX_PARTY_SLOTS; ++i)
		mInfoBar[i]->SetMenuMode(mode);
		
	switch (mode)
	{
		case ActorStats::PARTY_VIEW_MENU:
			SetHelpText("Right click a party member to interact.");
			break;
		case ActorStats::DUEL_SWAP_MENU:
			SetHelpText("Right click a party member to summon to battle.");
			break;
		case ActorStats::USE_ITEM_MENU:
			SetHelpText("Right click a party member to use \\c007" + mUseItemId + "\\c999 on.");
			break;
		default: break; 
	}
}

/**
	@param stats the ActorStats widget that triggered the event. Used to determine what slot
		we're dealing with
*/

int LunemParty::MatchSlotWithActorStats(ActorStats* stats)
{
	int slot = -1;
	
	if (stats == mMyInfoBar)
	{	
		slot = -1;
	}
	else
	{
		for (int i = 0; i < MAX_PARTY_SLOTS; ++i)
		{
			if (stats == mInfoBar[i])
			{
				slot = i;
				break;
			}
		}
	}
	
	return slot;
}

void LunemParty::SendDuelSwapEvent(ActorStats* stats)
{
	MessageData md;
	int slot = MatchSlotWithActorStats(stats);

	md.SetId("DUEL_USE_SWAP");
	md.WriteInt("slot", slot); //party slot
	messenger.Dispatch(md);
	
	game->EndPlayersDuelTurn();
}

void LunemParty::SendUseItemEvent(ActorStats* stats)
{
	MessageData md;
	int slot = MatchSlotWithActorStats(stats);

	// Change the event ID if we're in a duel or not
	if (game->IsInDuel())
	{
		md.SetId("DUEL_USE_ITEM");
		
		game->EndPlayersDuelTurn();
	}
	else
	{
		md.SetId("USE_ITEM");	
		
		SetVisible(false);
	}

	md.WriteString("id", mUseItemId); //item id
	md.WriteInt("slot", slot); //party slot
	messenger.Dispatch(md);

}


