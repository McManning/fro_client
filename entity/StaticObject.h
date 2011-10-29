
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


#ifndef _STATICOBJECT_H_
#define _STATICOBJECT_H_

#include "../core/Core.h"
#include "Entity.h"

class StaticObject : public Entity
{
  public:
	StaticObject();
	~StaticObject();

	void Render();

	bool LoadImage(string file);
	
	/* Uses the supplied image as mOriginalImage and clones for mImage */
	void SetImage(Image* img); 
	
	Image* GetImage();

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

    void ToHorizontalAnimation(int frameWidth, int delay);

	bool _animate(); // Called by the mAnimationTimer
	void PlayAnimation();
	void StopAnimation();
	
	timer* mAnimationTimer;

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
