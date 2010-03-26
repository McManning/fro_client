
#ifndef _STATICOBJECT_H_
#define _STATICOBJECT_H_

#include "../core/Core.h"
#include "Entity.h"

class StaticObject : public Entity
{
  public:
	StaticObject();
	~StaticObject();

	void Render(uLong ms);

	void LoadImage(string file);

	rect GetBoundingRect();
	
	/*	Rotate/zoom this object. Will create a full copy of mOriginalImage
		(Copying SDL_Image and all), manipulate that copy, and replace mImage.
		Degree = 0 to 360
		Zoom = Percentage * 0.01 (ie: 1 = normal size, 2 = double, etc)
		
		WARNING: The origin and collision rects do not change when Rotozooming,
			so they can be vastly off target
	*/
	void Rotozoom(double degree = 0.0, double zoom = 1.0);
	
	/*	If mImage is rotated/zoomed, will reload it, using the new
		AA settings 
	*/
	void SetAA(bool b);
	
	//Image to render
	Image* mImage;
	
	//before modifications were made, this was the version loaded from file
	Image* mOriginalImage; 
	
	/*	Set warp properties and add an event listener if we don't already have one */
	void SetWarp(string id, string objectName);

	string GetWarpId() const { return mWarpDestinationId; };
	string GetWarpObject() const { return mWarpDestinationObject; };

	//listener for when the player moves in/our of our collision, and we have warp properties
	MessageListener* mWarpEntityMoveListener;
	
	double mRotation;
	double mScale;

	bool mUseAA; //Will this entity use AA when scaling/rotating?
	
	uShort mDepth;
	
  protected:
			
	//warp properties
	string mWarpDestinationId; //map we're warping to
	string mWarpDestinationObject; //the object we're going to warp to. (If blank, will warp to default spawn)
	
};

#endif //_STATICOBJECT_H_
