
#include "WidgetList.h"

void callback_WidgetListScroller(Scrollbar* s)
{
	WidgetList* f = (WidgetList*)s->mParent;
	f->UpdateChildrenPositions();
}

WidgetList::WidgetList(Widget* wParent, rect rPosition)
	: Frame(wParent, "", rPosition)
{
	mCanSortChildren = false;
	
	mScroller = new Scrollbar(this, "", rect(Width() - 20, 0, 20, Height()), 
								VERTICAL, 1, 0, 0, callback_WidgetListScroller);
}

int WidgetList::GetMaxChildWidth()
{
	return Width() - 20;
}

void WidgetList::UpdateChildrenPositions()
{
	int h = 0;
	int min = mScroller->GetValue();
	rect r;

	// Reposition visible widgets, and hide all invisible ones
	for (int i = 0; i < mChildren.size(); ++i)
	{
		if (i >= min && h + mChildren.at(i)->Height() <= Height())
		{
			mChildren.at(i)->SetVisible(true);
			
			r = mChildren.at(i)->GetPosition();
			r.y = h;
			mChildren.at(i)->SetPosition(r);
			
			h += r.h;
		}
		else
		{
			mChildren.at(i)->SetVisible(false);
		}
	}

	mScroller->SetMax(mChildren.size());
}

bool WidgetList::Add(Widget* child)
{
	bool result = Widget::Add(child);
	UpdateChildrenPositions();
	return result;
}
	
bool WidgetList::Remove(Widget* child, bool deleteClass)
{
	bool result = Widget::Remove(child, deleteClass);
	UpdateChildrenPositions();
	return result;
}


	
