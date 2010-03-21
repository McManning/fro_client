
#ifndef _MAPEDITORDIALOG_H_
#define _MAPEDITORDIALOG_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

class Console;
class EditorMap;
class Multiline;
class Button;
class Input;
class MapEditorDialog : public Frame
{
  public:
	MapEditorDialog();
	~MapEditorDialog();

	void Render(uLong ms);

	void Event(SDL_Event* event);

	void ResizeChildren();
	void SetPosition(rect r);

	void CopySelectedObjectInList();

	/*	Repopulates mUniqueObjectsList. If we have text in the search box, will only
		populate with objects matching the search phrase. Otherwise, everything.
	*/
	void UpdateUniqueObjectsList();
	
	EditorMap* mMap;
	Multiline* mUniqueObjectsList;
	Button* mLoadAllResources;
	Input* mSearch;

};

#endif //_MAPEDITORDIALOG_H_
