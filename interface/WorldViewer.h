
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

