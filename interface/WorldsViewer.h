
#ifndef _WORLDSVIEWER_H_
#define _WORLDSVIEWER_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "../core/widgets/Scrollbar.h"
#include "../core/widgets/Button.h"

class WorldsViewer : public Frame
{
  public:
	WorldsViewer();
	~WorldsViewer();
	
	void Render();
	void SetPosition(rect r);
	void ResizeChildren();
	
	void UpdateChannelCount(string channel, int count);
	
	struct worldInfo
	{
		string title;
		string id;
		string description;
		int usercount;
		int rating;
		Image* icon;
	};

	std::vector<worldInfo> mWorlds;

	Frame* mListFrame;
	Scrollbar* mScroller;
	Button* mRefresh;
	
	Image* mDefaultIcon;

	void RefreshWorlds();
  private:
	void _load();
	int _renderSingle(rect r, int index);
};

#endif //_WORLDSVIEWER_H_
