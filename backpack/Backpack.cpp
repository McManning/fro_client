
#include "Backpack.h"
#include "../core/GuiManager.h"

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

Backpack::Backpack()
	: Frame(gui, "Backpack", rect(0, 0, 45*5+50*2, 45*4), "", true, false, false, true)
{
	// TODO: Create buttons and hook!
	// Say buttons are 45x45
	mSelectedItem = NULL;
	mCanSortChildren = false; // Keep the selector above stuff, etc. 

	mPreviousPageButton = new Button(this, "previous", rect(0,30,45,45), "", callback_Backpack_PreviousPage);
		mPreviousPageButton->SetImage("assets/backpack/previous.png");
		mPreviousPageButton->mHoverText = "Previous Page";
		
	mNextPageButton = new Button(this, "next", rect(Width()-45,30,45,45), "", callback_Backpack_NextPage);
		mNextPageButton->SetImage("assets/backpack/next.png");
		mNextPageButton->mHoverText = "Next Page";

	mCloseButton = new Button(this, "close", rect(Width()-45,Height()-45,45,45), "", NULL); //callback_Backpack_Close);
		mCloseButton->SetImage("assets/backpack/close.png");
		mCloseButton->mHoverText = "Close";
				
	mSelectorIcon = new Button(this, "select", rect(0,0,20,20), "", NULL);
		mSelectorIcon->mUsesImageOffsets = false;
		mSelectorIcon->SetImage("assets/backpack/selector.png");
		mSelectorIcon->SetVisible(false);
	
	mSelectorPage = 0;
		
	Load();
	
	Center();
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
						w->SetIndex(0);
					else if (p == 2 && x == 3)
						w->SetIndex(1);
						
					//w->SetIndex(-1); // TODO: Load real index!
					w->SetPosition( rect(50 + x * 45, 30 + y * 45, 40, 40) );
					w->onClickCallback = callback_BackpackItem_Click;
					w->onRightClickCallback = callback_BackpackItem_RightClick;
					w->mHoverText = "Page: " + its(p) + " (" + its(x) + "," + its(y) + ")";
					
				mPages[p].items[x+y*MAX_BACKPACK_COLUMNS] = w; 
			}
		}
	}
	
	SetPage(0);
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
	Add(mCloseButton);
	Add(mSelectorIcon);

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

void Backpack::RightClickItem(BackpackItem* item)
{
	// TODO: Display a RCM menu based on item type, and current game state
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
	else if (!mSelectedItem && item->mIndex > -1) //also prevent selecting empty slots first
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
}

