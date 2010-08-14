
#ifndef _RIGHTCLICKMENU_H_
#define _RIGHTCLICKMENU_H_

#include "Widget.h"

class RightClickMenu : public Widget
{
  public:
	
	struct sCallback 
	{
		void (*func)(RightClickMenu*, void*);
		void* userdata;
	};
		
	RightClickMenu();

	void Render();

	void AddOption(string text, void (*callback)(RightClickMenu*, void*), void* userdata = NULL);
	
	std::vector<sCallback> mCallbacks;
	
	bool mFirstEvent;
};

#endif //_RIGHTCLICKMENU_H_