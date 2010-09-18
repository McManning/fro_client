
/*
	I want a more efficient signal-attachment method to widget classes...
	Something that doesn't require such a large amount of code.
	
	OPTIMIZETODO: Child deleting will require a redraw.. 
*/

#ifndef _WIDGET_H_
#define _WIDGET_H_

#include "../Common.h"
#include "../FontManager.h"

//because all widgets will include Widget, and they all use SDL_Events crap
#include <SDL/SDL_events.h> 

enum //widgetType
{
	WIDGET_UNKNOWN = 0,
	WIDGET_BUTTON,
	WIDGET_CHECKBOX,
	WIDGET_DROPDOWN,
	WIDGET_FRAME,
	WIDGET_INPUT,
	WIDGET_LABEL,
	WIDGET_LIST,
	WIDGET_MULTILINE,
	WIDGET_SCROLLBAR,
	WIDGET_SMALLSELECT
};

class Widget
{
  public:
	Widget();
	virtual ~Widget();

	Widget* GetParent();
	
	sShort GetScreenX();
	sShort GetScreenY();

	//GuiManager decides if any of these are called or not. 
	virtual void Event(SDL_Event* event);

	//Renders this widget and all children
	virtual void Render();

	rect GetPosition()
	{
		return mPosition;
	};
	
	uShort Width()
	{
		return mPosition.w;
	};
	
	uShort Height()
	{
		return mPosition.h;	
	};
	
	rect GetScreenPosition()
	{
		return rect( GetScreenX(), GetScreenY(), mPosition.w, mPosition.h );	
	}
	
	bool IsVisible()
	{
		return mVisible;
	};
	
	bool IsActive()
	{
		if (!mActive) return false;
		
		//Recursively check if our parent is active. If not, neither are
		if (GetParent())
			return GetParent()->IsActive();
		else
			return mActive;
	};
	
	/* Returns true if this widget or any children have key focus */
	bool HasKeyFocusInTree();
	
	/* Returns true if this widget or any children have mouse focus */
	bool HasMouseFocusInTree();
	
	/* Returns true if this widget has key focus */
	bool HasKeyFocus();
	
	/* Returns true if this widget has mouse focus */
	bool HasMouseFocus();
	
	/*	Adds our widgets rect to the clipping system to allow it to be
		redrawn to the screen next render
	*/
	void FlagRender();

	virtual void SetPosition(rect r);
	virtual void SetVisible(bool b);
	virtual void SetActive(bool b);
	
	void SetTemporary(bool b) { mTemporary = b; };
	
	void SetSize(uShort w, uShort h);
	
	//center our position in the parent
	void Center(); 
	
	//Delete this widget and all children from the gui
	virtual void Die();

	/*	if b == true, no other widget besides this one and it's children will be given 
		input focus. If b == false, then if this widget demanded focus, it will no longer.
	*/
	void DemandFocus(bool b = true);
	
	void SetKeyFocus(bool b = true);
	
	virtual bool Add(Widget* child);
	virtual bool Remove(Widget* child, bool deleteClass);
	
	/*	Deep searching method. Allows us to grab Objects from the 
		children of our childrens children
	*/
	Widget* Get(string id, bool searchDeep = false, uShort type = WIDGET_UNKNOWN);
	
	sShort Get(Widget* w);
	
	Widget* At(uShort i);

	void MoveToTop();
	void MoveToBottom();
	
	virtual void SetImage(string file);
	
	string mId;
	Widget* mParent; //Do not set this directly. Have the parent call an Add()
	std::vector<Widget*> mChildren; 
	
	//True: Children will not be event handled if their position is outside our rect.
	bool mConstrainChildrenToRect; 

	//Goes to the next sibling that can accept keyboard input.
	void TabToNextSibling();
	
	/*	Calculate an offset of our source image based on widget state. (Mouse hover, normal, disabled, etc)
		And the height the image would usually be that we are going to use. 
	*/
	int CalculateImageOffset(int height);

	Image* mImage; //the main image used for rendering the widget

	Font* mFont; //a lot of widgets tend to have a font reference, so it's global now
	color mFontColor;
	
	color mBorderColor;
	
	uShort mType; //what kind of widget
	
	bool mSortable;
	bool mCanSortChildren;
	
	string mHoverText;
	uShort mHoverDelay;
	
	bool mTemporary; //this widget will auto-delete itself on MOUSEBUTTONUP/KEYUP/KEYDOWN
	bool mUsesImageOffsets; // If false, CalculateImageOffset will always return 0
	
  protected:
	
	rect mPosition; 
	
	bool mVisible; //if not visible, the widget won't render or receive events
	bool mActive; //Disabled widgets won't receive events
};

#endif //_WIDGET_H_
