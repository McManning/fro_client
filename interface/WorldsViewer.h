
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
	Button* mOfficialList;
	Button* mPersonalsList;
	
	Image* mDefaultIcon;

	void RefreshWorlds(int type);
	int mCurrentView; 
	
	enum 
	{
		VIEW_OFFICIAL = 1,
		VIEW_PERSONAL,
		VIEW_FAVORITES
	};
	
  private:
	void _load();
	int _renderSingle(rect r, int index);
};

#endif //_WORLDSVIEWER_H_
