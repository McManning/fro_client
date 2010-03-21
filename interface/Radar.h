
#ifndef _RADAR_H_
#define _RADAR_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

class Button;
class Entity;
class Radar : public Frame 
{
  public:
	Radar();
	~Radar();
	
	void Render(uLong ms);
	
	void ResizeChildren();
	void SetPosition(rect r);
	
	void ZoomIn();
	void ZoomOut();

  private:
	void _renderEntityOnRadar(Entity* e);
		
	Button* mZoomIn;
	Button* mZoomOut;
	
	uShort mZoom;
};

#endif //_RADAR_H_
