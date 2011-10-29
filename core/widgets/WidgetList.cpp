
/*
 * Copyright (c) 2011 Chase McManning
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */


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
	
	SetImage("assets/gui/widgetlist_bg.png");
	
	mScroller = new Scrollbar(NULL, "", rect(Width() - 15, 0, 15, Height()), 
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
