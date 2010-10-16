
#ifndef _BACKPACK_H_
#define _BACKPACK_H_

#include "../core/widgets/Frame.h"
#include "BackpackItem.h"

const int MAX_BACKPACK_PAGES = 3;
const int MAX_BACKPACK_ROWS = 3;
const int MAX_BACKPACK_COLUMNS = 5;

/**
	A pane containing multiple BackpackItem icons
*/
class Backpack : public Frame
{
  public:
	Backpack();
	~Backpack();
	
	struct page
	{
		BackpackItem* items[MAX_BACKPACK_ROWS*MAX_BACKPACK_COLUMNS];
	};

	/** Load our Backpack data from web, or something */
	bool Load();
	
	/** Save it all somewhere */
	void Save();
	
	/**	Opens an RCM for the selected item, options change based on current game state */
	void RightClickItem(BackpackItem* item);
	
	/**	Swaps this item with another selected, or selects this item, or deselects this item if previously selected */
	void SelectItem(BackpackItem* item);

	/**	Swaps positions of two items in the backpack. Can get a little complex, if we're dealing with items on different pages */
	void SwapItems(BackpackItem* a, BackpackItem* b);

	/**	Add a certain amount of an item, by index, to our Backpack */
	bool AddItem(int index, int amount);
	
	/**	Erase the specific amount of an item, by index */
	void EraseItem(int index, int amount);
	
	/**	@return true if we have >= amount of the item */
	bool HasItem(int index, int amount);
	
	/**	Use the specific item, if we have it. Whether it can be used is baed on its
		use type and the current ingame situation/mode */
	void UseItem(int index);
	
	void SetPage(int p);
	void NextPage();
	void PreviousPage();

	page mPages[MAX_BACKPACK_PAGES];
	
	int mCurrentPage;
	
	Button* mPreviousPageButton;
	Button* mNextPageButton;
	Button* mCloseButton;
	Button* mSelectorIcon;
	
	int mSelectorPage; // what page the mSelectorIcon is on
	
  private:
  
	/** Will delete all stored BackpackItem widgets */
	void _deleteAllItems();
	
	BackpackItem* _findItem(int index, int& page, int& slot);
	
	/**	Called when the contents of the backpack changes. 
		When the cloud system is used, it will attempt to reupload our backpack to 
		the master. Until then, it'll ... save to file or something */
	void _itemsUpdated();
	
	BackpackItem* mSelectedItem; /// TODO: Remember to NULL this when the inventory hides!
};

#endif // _BACKPACK_H_
