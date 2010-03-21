
#ifndef _FILEPICKER_H_
#define _FILEPICKER_H_

#include "../Core.h"
#include "Frame.h"

/*	Currently, this is a simplified version that just accepts input from the user and checks for a filename. 
	The final version should actually be able to browse directories, filter results, etc.
	The WinOS file picker won't be used because there's still focus issues regarding that. 
		Sometimes it appears behind the app screen and we can't fix the order, or it appears and is disabled from any input, etc.
*/
class FilePicker : public Frame 
{
  public:
	FilePicker(void (*cb)(FilePicker*), void* userdata);
	~FilePicker();
	
	void Render(uLong ms);

	string GetFile();
	void Done();
	
	void (*onSelectCallback)(FilePicker*);
	void* mUserdata;
};

#endif //_FILEPICKER_H_
