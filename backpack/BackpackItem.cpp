
#include "BackpackItem.h"
//#include "ItemsDatabaseManager.h"

BackpackItem::BackpackItem()
	: Button(NULL, "", rect(0,0,40,40), "", NULL)
{
	mUsesImageOffsets = false;
	SetIndex(0);
}

/*	NOTES
		- The image will be some local file named INDEX.png, in assets somewhere. 
			Probably assets/backpack/*.png
		- When the local file does not exist, it will load a assets/backpack/loading.png
			and download from the master. 
		- If the item has an index of 0, it will load assets/backpack/empty.png
		- Full data will come from a local file. It MIGHT be possible to request it in xml 
			format from the master, but I'm not sure how effective that would be. 
			And it would be problematic for trades, if it's slow to respond. 

*/
void BackpackItem::SetIndex(int index)
{
	mIndex = index;
	
	if (index == 0)
	{
		mAmount = 0;
		mUseType = USE_TYPE_NONE;
		mId.clear();
		mIconFile = "assets/backpack/empty.png";
		mDescription.clear();
	}
	else
	{
		//g_itemsDabaseManager.GetDetails(this);
		// Work on database LATER

		mAmount = 1;
		mUseType = USE_TYPE_NONE;
		mId = "???";
		mIconFile = "assets/backpack/empty.png";
		mDescription = "???";

		mAmount = 1;
	}
	
	// TODO: if file doesn't exist, download it
	SetImage(mIconFile);
}

void BackpackItem::SetAmount(int amount)
{
	if (mAmount > 99)
		mAmount = 99;
		
	mAmount = amount;
	//TODO: something else. Change a child label or something. Idk! Hover text?
}

void BackpackItem::Erase()
{
	SetIndex(0);
}

/** Sets this item as selected or not. When selected, it should change how it renders.. or some such */
void BackpackItem::SetSelected(bool s)
{
	// TODO: This!
}
	
