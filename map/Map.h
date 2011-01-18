
#ifndef _MAP_H_
#define _MAP_H_

#include <map>

#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "../entity/EntityManager.h"

#define TILE_SIZE (16)

class lua_State;
class RemoteActor;
class Map : public Frame, public EntityManager
{
  public:
	Map();
	virtual ~Map();

	enum mapType 
	{
		NONE = 0,
		BASIC,
		EDITOR,
		PURGATORY,
		MAZE
	};

	virtual void Render();
	virtual void Event(SDL_Event* event);
	virtual void Process();
	virtual bool IsRectBlocked(rect r);
	virtual void ResizeChildren();
	virtual void Die(); //Graceful cleanup

	void HandleLeftClick();
	void HandleRightClick();
	
	void ClickRemoteActor(RemoteActor* ra);
	
	/*	Will attempt to return the entity directly under the mouse, if it is clickable. */
	Entity* GetEntityUnderMouse(bool mustBeClickable, bool playersOnly);
	
	Entity* GetNextEntityUnderMouse(Entity* start, bool mustBeClickable, bool playersOnly);
	
	void CheckForClickableEntity();
		
	point2d mSpawnPoint;
	
	uShort mWidth;
	uShort mHeight;

	mapType mType;
	
	color mBackground; //fill color for maps
	string mWorkingDir;
	
/*	Camera Procedures */

	/*	Either set the point to the top left or center the camera around the point */
	void SetCameraPosition(point2d p, bool centered);

	/*	Set the x/y of the rect to the top left or center around the rects dimensions */
	void SetCameraPosition(rect r, bool centered);
	
	/*	Return the rect that contains all renderable area in a map */
	rect GetCameraPosition();
	
	void AddCameraRectForUpdate();
		
	void OffsetCamera(sShort offsetX, sShort offsetY);
	void OffsetCamera(point2d p);
	
	/*	Update the position of our camera based on various internal states */
	virtual void UpdateCamera();
	
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

	uShort GetGravity() const { return mGravity; };
	void SetGravity(uShort g) { mGravity = g; };

	void SaveFlags();
	void LoadFlags();

	void SetFlag(string flag, string value);
	string GetFlag(string flag);

	bool mShowPlayerNames; //TODO: Move to Game or something
	bool mEditorMode;
	
	std::vector<point2d> mCameraDestinationStack;
	uShort mCameraSpeed;
	
	lua_State* mLuaState;
	
	bool mStopCameraAtMapEdge;
	
	rect mCameraBounds; 
	
  private:
	void _constrainCameraToMap();
	void _constrainCameraX();
	void _constrainCameraY();

	Entity* mFollowedEntity; //entity the camera is following
	
	point2d mCameraPosition;
	point2d mOldCameraPosition; //used for determining if the camera moved last frame
	point2d mCameraFollowOffset;

	uShort mGravity;
		
	std::map<string, string> mFlags;
};	



#endif //_MAP_H_
