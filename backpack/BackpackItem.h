
#ifndef _BACKPACKITEM_H_
#define _BACKPACKITEM_H_

#include "../core/widgets/Button.h"

/** 
	A single BackpackItem, should be in an BackpackItemList parent.
	When this item is clicked, it'll send the event to the parent. Who will
	act accordingly (do a slot swap, or whatever)
	
*/
class BackpackItem : public Button
{
  public:
	BackpackItem();
	
	enum { // use types
		USE_TYPE_NONE = 0,
	};
	
	/**	Sets the index number of this item, and loads the associated information 
		from disk, or wherever. (The image filename, the title, description, use type, etc.
	*/
	void SetIndex(int index);
	
	void SetAmount(int amount);
	
	void Erase();
	
	/** Sets this item as selected or not. When selected, it should change how it renders.. or some such */
	void SetSelected(bool s);
	
	// Item properties here
	int mAmount;
	int mIndex; //item index, used to load the following information:
	string mDescription;
	string mIconFile;
	int mUseType;
};

#endif // _BACKPACKITEM_H_
