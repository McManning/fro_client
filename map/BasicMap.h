
#ifndef _BASICMAP_H_
#define _BASICMAP_H_

#include "Map.h"

/*
	Basic game map. Contains a collection of entities, and renders all normally.
*/
class BasicMap : public Map
{
  public:
	BasicMap();
	virtual ~BasicMap();
	
	/*	Render all visible entities */
	virtual void Render();

	/*	Returns true if a solid entity has a collision rect intersecting
		our test map rect */
	bool IsRectBlocked(rect r);

  private:
	void _renderEntities();
};

#endif //_BASICMAP_H_
