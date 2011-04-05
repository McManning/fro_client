
#include "WidgetList.h"
#include "Scrollbar.h"

void callback_WidgetListScroller(Scrollbar* s)
{
	WidgetList* f = (WidgetList*)s->mParent;
	f->UpdateChildrenPositions();
}

WidgetList::WidgetList(Widget* wParent, rect rPosition)
	: Frame(wParent, "", rPosition)
{
	mCanSortChildren = false;
	mConstrainChildrenToRect = false;
	
	mScroller = new Scrollbar(NULL, "", rect(Width() - 20, 0, 20, Height()), 
								VERTICAL, 1, 0, 0, callback_WidgetListScroller);
	Add(mScroller);
}

WidgetList::~WidgetList()
{
	
}

int WidgetList::GetMaxChildWidth()
{
	return Width() - 20;
}

void WidgetList::UpdateChildrenPositions()
{
	ASSERT(mScroller);
	
	int h = 0;
	int min = mScroller->GetValue();
	rect r;

	// Reposition visible widgets, and hide all invisible ones
	for (int i = 0; i < mChildren.size(); ++i)
	{
		if (mChildren.at(i) != mScroller)
		{
			if (i > min && h < Height()) // allow a bit of overflow
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
	}

	mScroller->SetMax(mChildren.size() - 1);
}

bool WidgetList::Add(Widget* child)
{
	bool result = Widget::Add(child);
	UpdateChildrenPositions();
	return result;
}
	
bool WidgetList::Remove(Widget* child, bool deleteClass)
{
	bool result = false;
	if (child != mScroller)
	{
		result = Widget::Remove(child, deleteClass);
		UpdateChildrenPositions();
	}
	return result;
}

void WidgetList::RemoveAll()
{
	for (int i = 0; i < mChildren.size(); ++i)
	{
		if (mChildren.at(i) && mChildren.at(i) != mScroller)
		{
			mChildren.at(i)->mParent = NULL;
			SAFEDELETE(mChildren.at(i));
		}
	}
	
	mChildren.clear();
	mChildren.push_back(mScroller);
	FlagRender();
	UpdateChildrenPositions();
}
