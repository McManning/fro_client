
#ifndef _SHORTCUTS_H_
#define _SHORTCUTS_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

const char* const SHORTCUTS_FILENAME = "shortcuts.xml";

const int SHORTCUT_INTERVAL_MS = 5000;

/*
	Read from xml:
		<shortcuts>
			<f1>text</f1>
			<f2>text</f2>
			...
			<f8>text</f8>
		</shortcuts>
*/
class Shortcuts : public Frame 
{
  public:
	Shortcuts();
	~Shortcuts();

	bool Load();
	bool Save();
	
	/*	Run the specified shortcut. num is 1-8 (Func keys 1-8) */
	void Run(int num);
	
  private:
	string mShortcut[8]; 
};

extern Shortcuts* shortcuts;

#endif //_SHORTCUTS_H_
