
#include "BackpackItem.h"

BackpackItem::BackpackItem()
	: Button(NULL, "", rect(0,0,40,40), "", NULL)
{
	SetIndex(-1);
}

/*	NOTES
		- The image will be some local file named INDEX.png, in assets somewhere. 
			Probably assets/backpack/*.png
		- When the local file does not exist, it will load a assets/backpack/loading.png
			and download from the master. 
		- If the item has an index of -1, it will load assets/backpack/empty.png
		- Full data will come from a local file. It MIGHT be possible to request it in xml 
			format from the master, but I'm not sure how effective that would be. 
			And it would be problematic for trades, if it's slow to respond. 

*/
void BackpackItem::SetIndex(int index)
{
	// TODO: SOMETHING! I'm thinking loading from a lua database file, or something. Or binary, binary is fine too. 
	// Maybe there's an ItemManager that has some database loaded into memory, and this can request data, or some shit. 
	
	if (index < 0)
	{
		mIndex = -1;
		mAmount = 0;
		mUseType = USE_TYPE_NONE;
		mTitle.clear();
		mDescription.clear();
	
		SetImage("assets/backpack/empty.png");
	}
	else
	{
		// TODO: If it doesn't exist, download, load temp, blah blah blah.
		SetImage("assets/backpack/" + its(index) + ".png");
		
		mIndex = index;
		mAmount = 1;
		mUseType = USE_TYPE_NONE;
		mTitle = "TODO: This items title!";
		mDescription = "TODO: this!";
	}
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
	SetIndex(-1);
}

/** Sets this item as selected or not. When selected, it should change how it renders.. or some such */
void BackpackItem::SetSelected(bool s)
{
	// TODO: This!
}
	
