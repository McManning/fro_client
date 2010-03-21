
#ifndef _OPENURL_H_
#define _OPENURL_H_

#include "../Core.h"
#include "Frame.h"

#define OPENURL_MAX_WIDTH 400
#define OPENURL_MIN_WIDTH 150

class OpenUrl : public Frame 
{
  public:
	OpenUrl(string url);
	~OpenUrl();
	
	void Render(uLong ms);

	string mUrl;
};

#endif //_OPENURL_H_
