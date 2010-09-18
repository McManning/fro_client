
#ifndef _WIDGETLIST_H_
#define _WIDGETLIST_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "../core/widgets/Scrollbar.h"

class WidgetList : public Frame
{
  public:
	WidgetList(Widget* wParent, rect rPosition);
	~WidgetList();
	
	void UpdateChildrenPositions();
		
	// Overriden from Widget
	bool Add(Widget* child);
	bool Remove(Widget* child, bool deleteClass);
	
	Scrollbar* mScroller;
};

#endif //_WIDGETLIST_H_
