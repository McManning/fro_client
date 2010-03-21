
#ifndef _MAP_H_
#define _MAP_H_

#include <map>

#include "BubbleManager.h"
#include "../lua/LuaManager.h"
#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "../entity/EntityManager.h"

#define TILE_SIZE (16)

class Map : public Frame, public EntityManager
{
  public:
	Map();
	virtual ~Map();

	enum mapType 
	{
		NONE = 0,
		EDITOR,
		COLLECTION,
		PURGATORY,
		MAZE
	};

	virtual void Render(uLong ms);
	virtual void Event(SDL_Event* event);
	virtual void Process(uLong ms);
	virtual bool IsRectBlocked(rect r);
	virtual void ResizeChildren();
	
	point2d mSpawnPoint;
	
	uShort mWidth;
	uShort mHeight;

	mapType mType;
	
	color mBackground; //fill color for maps
	
/*	Camera Procedures */

	/*	Either set the point to the top left or center the camera around the point */
	void SetCameraPosition(point2d p, bool centered);

	/*	Set the x/y of the rect to the top left or center around the rects dimensions */
	void SetCameraPosition(rect r, bool centered);
	
	/*	Return the rect that contains all renderable area in a map */
	rect GetCameraPosition();
		
	void OffsetCamera(sShort offsetX, sShort offsetY);
	void OffsetCamera(point2d p);
	
	/*	Update the position of our camera based on various internal states */
	virtual void UpdateCamera(uLong ms);
	
	/*	Returns true if the MAP POSITION rect is visible in our camera */
	bool IsRectInCamera(rect mapRect);

	/*	Returns a rect converted from screen position to camera position. */
	rect ToCameraPosition(rect screenRect);
	
	/*	Returns a rect converted from map position to screen position. */
	rect ToScreenPosition(rect mapRect);

	void SetCameraFollow(Entity* e) { mFollowedEntity = e; };
	Entity* GetCameraFollow() { return mFollowedEntity; };

	void SetCameraFollowOffset(point2d p) { mCameraFollowOffset = p; };
	point2d GetCameraFollowOffset() { return mCameraFollowOffset; };

	void SetCameraSpeed(uShort speed) { mCameraSpeed = speed; };
	uShort GetCameraSpeed() const { return mCameraSpeed; };

 	void AddCameraDestination(point2d p);

/*	Map Loading Procedures */

	/*	Load general map properties */
	virtual bool LoadProperties() = 0;

	/*	Queue up resources this map needs. Can either check if they exist in disk or download from url */
	virtual bool QueueResources() = 0; 
	
	/*	Load scripts we need. */
	virtual bool LoadScripts() = 0;
	
	/*	Clone unique entities, generate maze walls, whatever */
	virtual bool LoadLayout() = 0;
	
	/*	Overloaded from EntityManager so we can dispatch ENTITY_CREATE event */
	void AddEntity(Entity* e, sShort level);
	
	/*	Main method to load a new entity. Depending on the situation (and inherited map type), 
		they could be internally created, or configured through some xml file in either mXml 
		or an external source, etc etc.
		This should NOT actually add the entity to the map, but just create it, and let the caller deal with it */
	virtual Entity* AddEntityFromResources(string id, point2d pos) = 0;
	
	XmlFile* mXml;
	
	void SetLoadError(string err);
	string GetLoadError() const { return mLoadError; };

	uShort GetGravity() const { return mGravity; };
	void SetGravity(uShort g) { mGravity = g; };

	void SaveFlags();
	void LoadFlags();

	void SetFlag(string flag, string value);
	string GetFlag(string flag);
	
	BubbleManager mBubbles;
	
	string mChannelId; //set by LoadProperties
	bool mShowPlayerNames;

	//Should this map behave like online maps? 
	bool mOfflineMode;

	bool mShowInfo; //show extra info while rendering

	std::vector<LuaManager*> mScripts;
	std::vector<point2d> mCameraDestinationStack;
	uShort mCameraSpeed;
  private:
	void _constrainCameraToMap();
	void _constrainCameraX();
	void _constrainCameraY();

	Entity* mFollowedEntity; //entity the camera is following
	bool mStopCameraAtMapEdge;
	
	point2d mCameraPosition;
	point2d mCameraFollowOffset;
	
	string mLoadError;

	uShort mGravity;
		
	std::map<string, string> mFlags;
};	



#endif //_MAP_H_
