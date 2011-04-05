
#ifndef _WIDGETLIST_H_
#define _WIDGETLIST_H_

#include "Frame.h"

class Scrollbar;
class WidgetList : public Frame
{
  public:
	WidgetList(Widget* wParent, rect rPosition);
	~WidgetList();
	
	void UpdateChildrenPositions();
		
	int GetMaxChildWidth();
	
	// Overriden from Widget
	bool Add(Widget* child);
	bool Remove(Widget* child, bool deleteClass);
	void RemoveAll();

	Scrollbar* mScroller;
};

#endif //_WIDGETLIST_H_
