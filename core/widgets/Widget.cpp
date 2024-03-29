
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


#include "Widget.h"
#include "../Screen.h"
#include "../ResourceManager.h"
#include "../GuiManager.h"
#include "../FontManager.h"
#include "Console.h"

Widget::Widget() 
{
	mParent = NULL;
	mVisible = true;
	mActive = true;
	mSortable = true;
	mCanSortChildren = true;
	mTemporary = false;
	mConstrainChildrenToRect = true;
	mUsesImageOffsets = true;
	mType = WIDGET_UNKNOWN;
	mFont = NULL;
	mBorderColor.a = 0; //don't render
	mHoverDelay = 3000;
	mImage = NULL;
}

Widget::~Widget()
{	
	for (int i = 0;  i < mChildren.size(); i++)
	{
		mChildren.at(i)->mParent = NULL;
		delete mChildren.at(i);
	}
	
	mChildren.clear();
	
	if (mParent)
	{
		mParent->Remove(this, false);
		mParent = NULL;
	}
	
	resman->Unload(mImage);
	
	gui->DereferenceWidget(this);
}

bool Widget::Add(Widget* child) 
{
	if (!child)
		return false;
		
	if ( Get(child) != -1 )
	{
		FATAL("Cloning");
		return false;
	}
	
	mChildren.push_back(child);
	child->mParent = this;
	
	FlagRender();
	
	return true;
}

bool Widget::Remove(Widget* child, bool deleteClass)
{
	if (child)
	{
		for (int i = 0; i < mChildren.size(); ++i)
		{
			if (mChildren.at(i) == child)
			{
				child->mParent = NULL;
				if (deleteClass)
					SAFEDELETE(child);
				mChildren.erase(mChildren.begin() + i);
				
				FlagRender();
				
				return true;
			}
		}
	}

	return false; //not found
}

void Widget::RemoveAll()
{
	for (int i = 0; i < mChildren.size(); ++i)
	{
		if (mChildren.at(i))
		{
			mChildren.at(i)->mParent = NULL;
			SAFEDELETE(mChildren.at(i));
		}
	}
	
	mChildren.clear();
	FlagRender();
}

//Deep searching method. Allows us to grab Objects from the mChildren of our mChildrens mChildren
Widget* Widget::Get(string id, bool searchDeep, uShort type) 
{
	Widget* w;
	Widget* w2 = NULL;
	for (int i = 0;  i < mChildren.size(); ++i)
	{
		w = mChildren.at(i);
		if (w->mId == id && (w->mType == type || type == WIDGET_UNKNOWN))
			return w;
			
		if (searchDeep)
		{
			w2 = w->Get(id, true, type);
			if (w2)
				return w2;
		}
	}

	return NULL;
}

sShort Widget::Get(Widget* w) 
{
	if (w)
	{
		for (int i = 0;  i < mChildren.size(); ++i)
		{
			if (mChildren.at(i) == w)
				return i;
		}
	}

	return -1;
}

Widget* Widget::At(uShort i) 
{
	if (!mChildren.empty() && i < mChildren.size()) 
		return mChildren.at(i);
		
	return NULL;
}

void Widget::MoveToTop()
{
	if (!mParent || mParent->mChildren.empty() || !mSortable || !mParent->mCanSortChildren) 
		return;
		
	//if we're already at top, don't do the equations below
	if (mParent->mChildren.at(mParent->mChildren.size() - 1) == this)
		return; 
	
	sShort pos = mParent->Get(this);

	ASSERT(pos >= 0 && pos < mParent->mChildren.size());

	// see if we should move at all
	if (pos < mParent->mChildren.size()-1)
	{
		mParent->mChildren.erase(mParent->mChildren.begin() + pos);
		mParent->mChildren.push_back(this);
	
		mParent->MoveToTop(); //Continue down the tree until this widget is at the VERY TOP.
		FlagRender();
	}
}

void Widget::MoveToBottom()
{
	if (!mParent || !mSortable || !mParent->mCanSortChildren) 
		return;
		
	ASSERT( !mParent->mChildren.empty() );
		
	//if we're already at bottom, don't do the equations below
	if (mParent->mChildren.at(0) == this)
		return; 
	
	sShort pos = mParent->Get(this);

	ASSERT(pos >= 0 && pos < mParent->mChildren.size());

	mParent->mChildren.erase(mParent->mChildren.begin() + pos);
	mParent->mChildren.insert(mParent->mChildren.begin(), this);

	DEBUGOUT("Widget to bottom: " + mId);

	FlagRender();
}

Widget* Widget::GetParent() 
{
	return mParent;
}

sShort Widget::GetScreenX() 
{
	if (mParent)
		return mParent->GetScreenX() + mPosition.x;
	else
		return mPosition.x;
}

sShort Widget::GetScreenY()
{
	if (mParent)
		return mParent->GetScreenY() + mPosition.y;
	else
		return mPosition.y;
}

void Widget::Event(SDL_Event* event)
{
	/*	TODO: This doesn't work with our current event system
		Because as the widget is added to global event handlers when created, the
		mousebuttonup or keyup that creates the event will also be passed to this
		widget, thus instantly killing it after it's creation. Another method must be
		found.
	*/
	//if (mTemporary && (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP
	//					|| event->type == SDL_MOUSEBUTTONDOWN))
	//	Die();
}

void Widget::Render()
{
	Image* scr = Screen::Instance();

	if (mBorderColor.a != 0) //default will have a zero alpha
		scr->DrawRound(GetScreenPosition(), 0, mBorderColor);

	for (int i = 0;  i < mChildren.size(); i++)
	{
		if (mChildren.at(i)->mVisible)
			mChildren.at(i)->Render();	
	}
}

void Widget::FlagRender()
{
	if (IsVisible())
		g_screen->AddRect(GetScreenPosition());
}

void Widget::SetPosition(rect r)
{
    if (mPosition.x != r.x || mPosition.y != r.y || mPosition.w != r.w || mPosition.h != r.h)
    {
    	FlagRender(); //for old position
    
    	if (mParent && mParent->mConstrainChildrenToRect)
    	{
    		if (r.x + r.w > mParent->Width() && r.x == 0) r.w = mParent->Width();
    		if (r.y + r.h > mParent->Height() && r.y == 0) r.h = mParent->Height();
    		
    		if (r.x + r.w > mParent->Width()) r.x = mParent->Width() - r.w;
    		if (r.y + r.h > mParent->Height()) r.y = mParent->Height() - r.h;
    		
    		if (r.x < 0) r.x = 0;
    		if (r.y < 0) r.y = 0;
    	}
    	
    	mPosition = r;
    	FlagRender(); //for new position
    }
}

void Widget::SetVisible(bool b)
{
	if (mVisible != b)
	{
		FlagRender();
		mVisible = b;
		FlagRender();
	}
}

bool Widget::IsVisible()
{
	if (!mVisible) return false;
	
	//Recursively check if our parent is visible. If not, neither are we
	if (GetParent())
		return GetParent()->IsVisible();
	else
		return true;
}

void Widget::SetActive(bool b)
{
	if (mActive != b)
	{
		mActive = b;
		FlagRender();
	}
}

bool Widget::IsActive()
{
	if (!mActive) return false;
	
	//Recursively check if our parent is active. If not, neither are we
	if (GetParent())
		return GetParent()->IsActive();
	else
		return true;
}

bool Widget::HasKeyFocusInTree()
{
	if (gui->hasKeyFocus == this)
		return true;
		
	for (int i = 0;  i < mChildren.size(); i++)
	{
		if (mChildren.at(i)->HasKeyFocusInTree())
			return true;
	}
	
	return false;
}

bool Widget::HasMouseFocusInTree()
{
	if (gui->hasMouseFocus == this)
		return true;
		
	for (int i = 0;  i < mChildren.size(); i++)
	{
		if (mChildren.at(i)->HasMouseFocusInTree())
			return true;
	}
	
	return false;
}

bool Widget::HasKeyFocus()
{
	return (gui->hasKeyFocus == this);
}

bool Widget::HasMouseFocus()
{
	return (gui->hasMouseFocus == this);
}

/**
	@return true if this widget has GuiManager::GetPreviousMouseXY() in its rect
	@todo What if this widget moved? Would this return false positives?
*/
bool Widget::HadMouseFocus()
{
	return (gui->previousMouseFocus == this);
}

/**
	@return true if the last mouse move event counted as the mouse entering this 
		widget for the first time
*/
bool Widget::DidMouseEnter()
{
	return (!HadMouseFocus() && HasMouseFocus());
}

/**
	@return true if the last mouse move event counted as the mouse leaving this 
		widget for the first time
*/
bool Widget::DidMouseLeave()
{
	return (HadMouseFocus() && !HasMouseFocus());
}

//Goes to the next sibling that can accept keyboard input. 
//TODO: This. Since our list reorganizes every time a widget is clicked, it'll screw up
//the whole system. Figure out a working technique :[
void Widget::TabToNextSibling()
{
/*	int i;
	bool foundMe = false;
	if (mParent)
	{
		for (i = mParent->mChildren.size() - 1; i >= 0; i--)
		{
			DEBUGOUT("Checking: " + mParent->mChildren.at(i)->mId);
			if (mParent->mChildren.at(i) == this)
			{
				foundMe = true;
				DEBUGOUT("Found Me");
				continue;
			}
			else if (mParent->mChildren.at(i)->mType == WIDGET_INPUT)
			{
				if (foundMe)
				{
					gui->hasKeyFocus = mParent->mChildren.at(i);
					DEBUGOUT("Setting Focus: " + gui->hasKeyFocus->mId);
					break;
				}
			}
		}
	}*/
}
	
void Widget::SetSize(uShort w, uShort h)
{
	SetPosition( rect(mPosition.x, mPosition.y, w, h) );
}

//center our position in the parent
void Widget::Center()
{
	Image* scr = Screen::Instance();
	int w = (mParent) ? mParent->Width() : scr->Width();
	int h = (mParent) ? mParent->Height() : scr->Height();
	
	SetPosition( rect(w / 2 - mPosition.w /  2, h / 2 - mPosition.h /  2, mPosition.w, mPosition.h) );
}

void Widget::Die()
{
	gui->RemoveWidget(this);
}

void Widget::DemandFocus(bool b)
{
	if (b)
		gui->AddToDemandFocusStack(this);
	else
		gui->RemoveFromDemandFocusStack(this);
}

void Widget::SetKeyFocus(bool b)
{
	if (b)
	{
		gui->hasKeyFocus = this;
		MoveToTop();
	}
	else if (gui->hasKeyFocus == this)
	{
		gui->hasKeyFocus = NULL;
	}
}

/*	Calculate an offset of our source image based on widget state. (Mouse hover, normal, disabled, etc)
	And the height the image would usually be that we are going to use. 
*/
int Widget::CalculateImageOffset(int height)
{
	if (mUsesImageOffsets)
	{
		if (!IsActive())
		{
			return height * 3; //disabled
		}
		else if (HasMouseFocus())
		{
			if (gui->IsMouseButtonDown(MOUSE_BUTTON_LEFT) || gui->IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
				return height * 2;
			else
				return height;
		}
	}
	
	return 0;
}

void Widget::SetImage(string file)
{
	resman->Unload(mImage);
	mImage = resman->LoadImg(file);
}

