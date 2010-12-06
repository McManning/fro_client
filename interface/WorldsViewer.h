
#ifndef _WORLDSVIEWER_H_
#define _WORLDSVIEWER_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/WidgetList.h"

class WorldsViewer : public Frame
{
  public:
	WorldsViewer();
	~WorldsViewer();

	struct worldInfo
	{
		string title;
		string id;
		string description;
		int usercount;
		int rating;
		Image* icon;
	};
	
	enum 
	{
		VIEW_OFFICIAL = 1,
		VIEW_PERSONAL,
		VIEW_FAVORITES
	};
	
	void RefreshWorlds(int type);
	
	std::vector<worldInfo> mWorlds;

	WidgetList* mList;
	Label* mRefreshingNotice;
	Button* mRefresh;
	Button* mOfficialList;
	Button* mPersonalsList;

	int mCurrentView; 
};

#endif //_WORLDSVIEWER_H_
