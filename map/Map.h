
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
	~Map();

	void Render();
	void Event(SDL_Event* event);
	void Process();
	bool IsRectBlocked(rect r);
	void ResizeChildren();
	void Die(); //Graceful cleanup

    void CreateChatbox();
    void CreateHud();

	void HandleLeftClick();
	void HandleRightClick();

	/*	Will attempt to return the entity directly under the mouse, if it is clickable. */
	Entity* GetEntityUnderMouse(bool mustBeClickable, bool actorsOnly);
	
	Entity* GetNextEntityUnderMouse(Entity* start, bool mustBeClickable, bool actorsOnly);
	
	void CheckForClickableEntity();
		
	point2d mSpawnPoint;
	
	uShort mWidth;
	uShort mHeight;

	color mBackground; //fill color for maps
	string mWorkingDir;
	
/*	Camera Procedures */

	/*	Either set the point to the top left or center the camera around the point */
	void SetCameraPosition(point2d p, bool centered);

	/*	Set the x/y of the rect to the top left or center around the rects dimensions */
	void SetCameraPosition(rect r, bool centered);
	
	/*	Return the rect that contains all renderable area in a map */
	rect GetCameraPosition();
	
	void AddCameraRectForUpdate(bool force);
		
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
	
	void HideChat();
	void ShowChat();

	void SaveFlags();
	void LoadFlags();

	void SetFlag(string flag, string value);
	string GetFlag(string flag);

	bool mShowPlayerNames; //TODO: Move to Game or something
	bool mEditorMode;
	bool mShowDebug;
	
	std::vector<point2d> mCameraDestinationStack;
	uShort mCameraSpeed;
	
	lua_State* mLuaState;
	
	bool mStopCameraAtMapEdge;
	
	rect mCameraBounds; 
	
	Console* mChat;
	Frame* mHud;
	
  private:
	void _constrainCameraToMap();
	void _constrainCameraX();
	void _constrainCameraY();
	void _renderEntities();

	Entity* mFollowedEntity; //entity the camera is following
	
	point2d mCameraPosition;
	point2d mCameraFollowOffset;
	rect mOldCameraRect; //used for determining if the camera moved last frame

	uShort mGravity;
		
	std::map<string, string> mFlags;
};	



#endif //_MAP_H_
