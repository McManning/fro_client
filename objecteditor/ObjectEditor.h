
/*	Old Object Properties
		bool use colorkey
		rendering style (repeat, screen, map position, etc) Never used.
		animation delay
		animation style
		image and version
		source rect
		point2d offset from origin
	ENTITY BASE PROPERTIES:
		snap to grid
		collision data
		id
		map position
		solidity
		
		
	Console Commands (Because I'm too lazy to make dialogs..)
	
	oe_command <parameter> <optionalParameter:defaultValue> - Comments
	
	oe_toanim <width> <delay:1000> - Converts to a horizontal animation format (if not already animated)
	/resetorigin - Reset origin point to bottom center
	/resetrects - Reset collision rects.
	/saveobject <id> - Save to DIR_ENTITIES/id.xml
	/loadobject <id> - Load from DIR_ENTITIES/id.xml. Will not save current work!
	/solid <1 || 0> - Set solidity of this object (default solidity is 1)
	/editorigin - Enter origin edit mode
	/editrects - Enter collision rect edit mode
	/shadow <1 || 0> - Set whether or not this object casts a shadow (default is 1)

	XML, work in progress
	<object id="x"> <!-- Main loader -->
		<base solid="1" shadow="1"/> <!-- Entity, base properties -->
		
		<image ext="png" ver="#" frameset="id" /> <!-- StaticObject, tells it load the image and change the frameset if specified -->
		
		<animation w="#" delay="#"/> <!-- StaticObject, converts loaded image to horiz anim if not already animated -->

		<origin position="x,y" /> <!-- Entity -->
		
		<rect>serializedRect</rect> <!-- Entity -->
		<rect>serializedRect</rect> <!-- Entity -->
		...
	</object>

*/

#ifndef _OBJECTEDITOR_H_
#define _OBJECTEDITOR_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

#define CAMERA_OFFSET_SPEED (64)

class Console;
class ObjectEditor : public Frame
{
  public:
	ObjectEditor();
	~ObjectEditor();
	
	/*
		On mouse up:
			if editing rects
				if rclick
					delete all rects touching point
				else if lclick
					if not holding a rect
						start new rect at point
					else
						end holding rect at point
			else if editing origin
				if lclick
					set origin to point
	*/
	void Event(SDL_Event* event);
	
	/*	Renders (in order): 
			Frame, mImage in frame, collision rects, origin point, position of cursor relative to our image
	*/
	void Render();
	void RenderGrid(rect r);

	void SetPosition(rect r);

	/*
		Load the specified object id into the editor.
		Will attempt to load DIR_ENTITIES/id.xml. If the file doesn't exist, it will then attempt to
		load DIR_ENTITIES/id.img where img is any supported image format. If successful, it will 
		create a default xml config for the object. If it can't find a file to load, will return false.
	*/
	bool Load(string id);
	
	int ParseXml(XmlFile* xf, TiXmlElement* e);
	bool LoadXml();
		
	void SetDefaults();
	
	/*	Saves to DIR_ENTITIES/mId.xml */
	bool Save();

	rect GetImagePosition();
	
	/*	Convert the loaded object into a horizontal animation, if not already animated. Returns true on success, false if already animated */
	bool ConvertToHorizontalAnimation(uShort width, uShort delay);

	/*	Delete all collision rects intersecting with the given rect. 
		Returns true if it deleted at least one. False otherwise
	*/
	bool RemoveAllRectsIntersectingWith(rect r);
	
	/*	Delete all collision rects */
	void RemoveAllRects();

	void ResetOrigin();
		
	void DisplayInfo();
	
	void OnConsoleInput(Console* c, string s);

	void OffsetCamera(sShort x, sShort y);
	
	string mId;
	
	bool mEditingRects;
	bool mEditingOrigin;
	rect mHoldingRect;

	//Object Properties
	Image* mImage; //the renderable object
	string mImageFile; //image file we're using
	//string mImageVersion; //image version
	
	std::vector<rect> mCollisionRects;
	point2d mOrigin; //origin point, used when positioning on the map. Default is bottom center.
	bool mSolid; //is this object solid?
	bool mShadow; //does this object render a shadow
	uShort mDelay; //frame delay, for saving to xml for images not orginally animated
	byte mLayer; //map layer this entity exists on
	
	point2d mCamera; //camera position that decides what part of the object to render
	color mBackground;
	
	string mWarpDestinationId;
	string mWarpDestinationObject;
	
	bool mShowGrid;
  private:
		
	void _adjustOrigin();
	void _adjustHoldingRect();

	/* Attempt to load various formats into mImage */
	bool _tryToLoadImage();
	
	Image* mGridImage;

};

void callback_CreateActor(Console* c, string s); //create_actor id w h

#endif //_OBJECTEDITOR_H_
