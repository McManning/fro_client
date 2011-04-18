	
#ifndef _WORLDVIEWER_H_
#define _WORLDVIEWER_H_

#include "../core/widgets/Frame.h"

class Button;
class WidgetList;
class Input;
class WorldViewer : public Frame
{
  public:
	WorldViewer();
	~WorldViewer();
	
	struct worldData {
		string name;
		string id;
		string description;
		int users;
	};

	enum sortType
	{	
		SORT_NAME = 0,
		SORT_USERS,
	};
	
	enum listType
	{
		LIST_ALL = 0,
		LIST_OFFICIAL,
		LIST_USER,
	};
	
	void SortWorldData(sortType sort);
	bool LoadWorldDataFromFile(string filename);
	void FailedToLoadWorldData(string reason);
	void UpdateWorldList();
	void RequestWorldList(listType type);
	void SetControlState(bool active);
	
	Frame* CreateWorldInfoFrame(int width, worldData& data);
	
	std::vector<worldData> mWorldData;
	
	WidgetList* mWorldList;
	Frame* mRecommendedFrame;
	Button* mReloadButton;
	Input* mFilterInput;
	
	sortType mCurrentSortType;
	listType mCurrentListType;
};

#endif //_WORLDVIEWER_H_

