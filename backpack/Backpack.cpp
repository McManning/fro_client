
#include "Backpack.h"
#include "../core/GuiManager.h"
#include "../core/TimerManager.h"
#include "../core/widgets/RightClickMenu.h"
#include "../core/widgets/Label.h"
#include "../game/GameManager.h"


const int BACKPACK_WIDTH = (45*MAX_BACKPACK_COLUMNS+50*2);
const int BACKPACK_HEIGHT = (5+MAX_BACKPACK_ROWS*45+5+15+5);

const int BACKPACK_SLIDE_SPEED = 5;
const int BACKPACK_SLIDE_MS = 25;

void callback_BackpackItem_RightClick(Button* b)
{
	Backpack* pack = (Backpack*)b->GetParent();
	ASSERT(pack);

	pack->RightClickItem( (BackpackItem*)b );
}

void callback_BackpackItem_Click(Button* b)
{
	Backpack* pack = (Backpack*)b->GetParent();
	ASSERT(pack);

	pack->SelectItem( (BackpackItem*)b );
}

void callback_Backpack_NextPage(Button* b)
{
	Backpack* pack = (Backpack*)b->GetParent();
	ASSERT(pack);

	pack->NextPage();
}

void callback_Backpack_PreviousPage(Button* b)
{
	Backpack* pack = (Backpack*)b->GetParent();
	ASSERT(pack);

	pack->PreviousPage();
}

void callback_Backpack_Toggle(Button* b)
{
	Backpack* pack = (Backpack*)b->GetParent();
	ASSERT(pack);

	pack->ToggleState(pack->GetPosition().y < 0);
}

uShort timer_BackpackSlideUp(timer* t, uLong ms)
{
	Backpack* pack = (Backpack*)t->userData;
	rect r;
	
	if (pack && pack->mVelocity < 0)
	{
		r = pack->GetPosition();
		if (r.y + r.h > pack->mOpenPack->Height())
		{
			r.y += pack->mVelocity;
			pack->SetPosition(r);
			return TIMER_CONTINUE;
		}
		else
		{
			pack->mVelocity = 0;
		}
	}
	
	return TIMER_DESTROY;
}

uShort timer_BackpackSlideDown(timer* t, uLong ms)
{
	Backpack* pack = (Backpack*)t->userData;
	rect r;
	
	if (pack && pack->mVelocity > 0)
	{
		r = pack->GetPosition();
		if (r.y < 0)
		{
			r.y += pack->mVelocity;
			pack->SetPosition(r);
			return TIMER_CONTINUE;
		}
		else
		{
			pack->mVelocity = 0;
		}
	}
	
	return TIMER_DESTROY;
}

Backpack::Backpack()
	: Frame(gui, "Backpack", rect(gui->Width()/2-BACKPACK_WIDTH/2, -BACKPACK_HEIGHT + 25, BACKPACK_WIDTH, BACKPACK_HEIGHT), 
			"", false, false, false, true)
{
	// TODO: Create buttons and hook!
	// Say buttons are 45x45
	mSelectedItem = NULL;
	mVelocity = 0;
	mDorra = 0;
	mCanSortChildren = false; // Keep the selector above stuff, etc. 

	mPreviousPageButton = new Button(this, "", rect(0,5,45,45), "", callback_Backpack_PreviousPage);
		mPreviousPageButton->SetImage("assets/backpack/previous.png");
		mPreviousPageButton->mHoverText = "Previous Page";
		
	mNextPageButton = new Button(this, "", rect(Width()-45,5,45,45), "", callback_Backpack_NextPage);
		mNextPageButton->SetImage("assets/backpack/next.png");
		mNextPageButton->mHoverText = "Next Page";

	mSelectorIcon = new Button(this, "", rect(0,0,20,20), "", NULL);
		mSelectorIcon->mUsesImageOffsets = false;
		mSelectorIcon->SetImage("assets/backpack/selector.png");
		mSelectorIcon->SetVisible(false);

	mClosePack = new Button(this, "", rect(Width()/2-40,Height()-25,80,25), "", callback_Backpack_Toggle);
		mClosePack->SetImage("assets/backpack/close.png");
		mClosePack->mHoverText = "Close Backpack";
		mClosePack->SetVisible(false);
		
	mOpenPack = new Button(this, "", rect(Width()/2-40,Height()-25,80,25), "", callback_Backpack_Toggle);
		mOpenPack->SetImage("assets/backpack/open.png");
		mOpenPack->mHoverText = "Open Backpack";

	mDorraIcon = new Button(this, "", rect(mOpenPack->GetPosition().x + mOpenPack->Width() + 20,Height()-23,20,20), "", NULL);
		mDorraIcon->mUsesImageOffsets = false;
		mDorraIcon->SetImage("assets/backpack/dorra.png");

	mDorraLabel = new Label(this, "", rect(mDorraIcon->GetPosition().x + mDorraIcon->Width() + 5, Height()-20, 0, 0), its(mDorra));

	mSelectorPage = 0;
		
	SetImage("assets/backpack/bg.png");
		
	Load();
}

Backpack::~Backpack()
{
	_deleteAllItems();
}

bool Backpack::Load()
{
	int p, x, y;
	BackpackItem* w;
	
	for (int p = 0; p < MAX_BACKPACK_PAGES; ++p)
	{
		for (y = 0; y < MAX_BACKPACK_ROWS; ++y)
		{
			for (x = 0; x < MAX_BACKPACK_COLUMNS; ++x)
			{
				// Load widget and position accordingly
				w = new BackpackItem();
				
					// For testing
					if (p == 1 && x < 1)
						w->SetIndex(1);
					else if (p == 2 && x == 3)
						w->SetIndex(2);
						
					//w->SetIndex(0); // TODO: Load real index!
					w->SetPosition( rect(50 + x * 45, 5 + y * 45, 40, 40) );
					w->onClickCallback = callback_BackpackItem_Click;
					w->onRightClickCallback = callback_BackpackItem_RightClick;
					//w->mHoverText = "Page: " + its(p) + " (" + its(x) + "," + its(y) + ")";
					
				mPages[p].items[x+y*MAX_BACKPACK_COLUMNS] = w; 
			}
		}
	}
	
	SetPage(0);
}

void Backpack::ToggleState(bool b)
{
	if (mVelocity == 0)
	{
		if (b) // slide it out
		{
			mVelocity = BACKPACK_SLIDE_SPEED;
			timers->Add("", BACKPACK_SLIDE_MS, false, 
						timer_BackpackSlideDown, NULL, this);
		}
		else // slide it in
		{
			mVelocity = -BACKPACK_SLIDE_SPEED;
			timers->Add("", BACKPACK_SLIDE_MS, false, 
						timer_BackpackSlideUp, NULL, this);
		}
		
		// Toggle buttons
		mClosePack->SetVisible(b);
		mOpenPack->SetVisible(!b);
	}
}

void Backpack::SetPage(int p)
{
	int i;
	
	ASSERT(p < MAX_BACKPACK_PAGES);

	// Remove all current children
	for (i = 0; i < mChildren.size(); ++i)
	{
		if (mChildren.at(i))
			mChildren.at(i)->mParent = NULL;
	}
	mChildren.clear();
		
	// Add new children
	for (i = 0; i < MAX_BACKPACK_ROWS*MAX_BACKPACK_COLUMNS; ++i)
	{
		mChildren.push_back(mPages[p].items[i]);
		mPages[p].items[i]->mParent = this;
	}

	// Re-add our buttons
	Add(mPreviousPageButton);
	Add(mNextPageButton);
	Add(mSelectorIcon);
	Add(mOpenPack);
	Add(mClosePack);
	Add(mDorraIcon);
	Add(mDorraLabel);

	mCurrentPage = p;
	
	mPreviousPageButton->SetActive( mCurrentPage != 0 );
	mNextPageButton->SetActive( mCurrentPage != MAX_BACKPACK_PAGES - 1 );
	mSelectorIcon->SetVisible( p == mSelectorPage && mSelectedItem );

	FlagRender();
}

void Backpack::NextPage()
{
	if (mCurrentPage < MAX_BACKPACK_PAGES-1)
	{
		SetPage(mCurrentPage + 1);
	}
}

void Backpack::PreviousPage()
{
	if (mCurrentPage > 0)
	{
		SetPage(mCurrentPage - 1);
	}
}

void Backpack::_deleteAllItems()
{
	int i, p;

	for (p = 0; p < MAX_BACKPACK_PAGES; ++p)
	{
		if (p != mCurrentPage) // let the Widget class delete the ones we have visible
		{
			for (i = 0; i < MAX_BACKPACK_ROWS*MAX_BACKPACK_COLUMNS; ++i)
			{
				if (mPages[p].items[i])
				{
					mPages[p].items[i]->mParent = NULL;
					delete mPages[p].items[i];
				}
			}
		}
	}
	
	mSelectedItem = NULL;
}

void callback_Backpack_RCM_Info(RightClickMenu* m, void* userdata)
{
	BackpackItem* item = (BackpackItem*)userdata;
	
	if (item)
	{
		game->mChat->AddMessage("*** Item: " + item->mId + " x" + its(item->mAmount) + " ***");
		game->mChat->AddMessage("Desc: " + item->mDescription);
		game->mChat->AddMessage("Index: " + its(item->mIndex) + " UseType: " + its(item->mUseType));
	}
}

void callback_Backpack_RCM_Use(RightClickMenu* m, void* userdata)
{
	BackpackItem* item = (BackpackItem*)userdata;
	
	if (item)
	{
		game->mChat->AddMessage("OAK: This isn't the time to use that!");
	}
}

void Backpack::RightClickItem(BackpackItem* item)
{
	// TODO: Display a RCM menu based on item type, and current game state

	if (item->mIndex > 0)
	{
		RightClickMenu* m = new RightClickMenu();
			m->AddOption("Info", callback_Backpack_RCM_Info, item);
			m->AddOption("Use", callback_Backpack_RCM_Use, item);
	}
}

void Backpack::SelectItem(BackpackItem* item)
{
	// TODO: Swap if another is selected, unselect if the currently selected is this item, 
	// if nothing is selected, set this item as selected, etc.
	
	if (mSelectedItem == item)
	{
		mSelectedItem = NULL;
		item->SetSelected(false);
		
		mSelectorIcon->SetVisible(false);
	}
	else if (!mSelectedItem && item->mIndex > 0) //also prevent selecting empty slots first
	{
		mSelectedItem = item;
		item->SetSelected(true);
		
		// Attach our selector to the item
		rect r = item->GetPosition();
		r.x += 20;
		r.y -= 10;
		r.w = mSelectorIcon->Width();
		r.h = mSelectorIcon->Height();
		
		mSelectorIcon->SetVisible(true);
		mSelectorIcon->SetPosition(r);
		mSelectorPage = mCurrentPage;
	}
	else if (mSelectedItem)
	{
		SwapItems(mSelectedItem, item);
		mSelectedItem = NULL;
		
		mSelectorIcon->SetVisible(false);
	}
}

/**	Swaps positions of two items in the backpack.  Can get a little complex, if we're dealing with items on different pages */
void Backpack::SwapItems(BackpackItem* a, BackpackItem* b)
{
	/* 	Set a's position to b's
		Set b's position to a's
		Set a's page&slot to b's, and visa versa
		Set a's parent to b's, v/v
	*/
	int p, i;
	Widget* temp;
	rect pos;
	int ap = -1, ai, bp = -1, bi;
	
	// Locate a and b 
	for (p = 0; p < MAX_BACKPACK_PAGES && (ap < 0 || bp < 0); ++p)
	{
		for (i = 0; i < MAX_BACKPACK_ROWS*MAX_BACKPACK_COLUMNS && (ap < 0 || bp < 0); ++i)
		{
			if (mPages[p].items[i] == a)
			{
				ap = p;
				ai = i;
			}
			else if (mPages[p].items[i] == b)
			{
				bp = p;
				bi = i;	
			}
		}
	}
	
	// Do various swaps
	pos = a->GetPosition();
	a->SetPosition(b->GetPosition());
	b->SetPosition(pos);
	
	mPages[ap].items[ai] = b;
	mPages[bp].items[bi] = a;
	
	// Refresh the page and send an update
	SetPage(mCurrentPage);
	_itemsUpdated();
}

/**	Add a certain amount of an item, by index, to our Backpack */
bool Backpack::AddItem(int index, int amount)
{
	int page, slot;
	BackpackItem* i = _findItem(index, page, slot);
	
	if (i)
	{
		i->SetAmount(i->mAmount + amount);
	}
	else
	{
		// TODO: find empty slot and set to new index, trigger a load for data, etc.
	}
	
	_itemsUpdated();
}

/**	Erase the specific amount of an item, by index */
void Backpack::EraseItem(int index, int amount)
{
	int n;
	BackpackItem* i = _findItem(index, n, n);
	
	if (i)
	{
		i->Erase();
		_itemsUpdated();
	}
}

/**	@return true if we have >= amount of the item */
bool Backpack::HasItem(int index, int amount)
{
	int n;
	BackpackItem* i = _findItem(index, n, n);
	
	return (i && i->mAmount >= amount);
}

/**	Use the specific item, if we have it. Whether it can be used is baed on its
	use type and the current ingame situation/mode */
void Backpack::UseItem(int index)
{
	int page, slot;
	BackpackItem* i = _findItem(index, page, slot);
	ASSERT(i);
	
	// TODO: Use!
}

BackpackItem* Backpack::_findItem(int index, int& page, int& slot)
{
	int i, p;

	for (p = 0; p < MAX_BACKPACK_PAGES; ++p)
	{
		for (i = 0; i < MAX_BACKPACK_ROWS*MAX_BACKPACK_COLUMNS; ++i)
		{
			if (mPages[p].items[i] && mPages[p].items[i]->mIndex == index)
			{
				page = p;
				slot = i;
				return mPages[p].items[i];
			}
		}
	}
	
	return NULL;
}

/**	Called when the contents of the backpack changes. 
	When the cloud system is used, it will attempt to reupload our backpack to 
	the master. Until then, it'll ... save to file or something */
void Backpack::_itemsUpdated()
{
	// TODO: This!
	
	/* 	Should create an encrypted string containing info on all the items we own. Then upload
		that string to the server via a POST message. (Since it'd probably be too long for a GET)
		Contents:
			- Item index in every slot (2 bytes)
			- Amount of a particular item in every slot (1 byte)
				If the slot is empty (0), it will skip this amount byte.
	*/
	
}

/**	Called when the order of the backpack items change. 
	When the cloud system is used, it will attempt to reupload our backpack to 
	the master. Until then, it'll ... save to file or something

	Unlike _itemsUpdated(), this shouldn't upload to the cloud as soon as something changes,
	but instead only update every so often. Backpack sorting is less important than backpack
	contents, and it's fine if we lose the sort due to a crash or something.
*/
void Backpack::_itemsResorted()
{
	// TODO: This!
}

