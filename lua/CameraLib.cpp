
#include <lua.hpp>
#include "CameraLib.h"
#include "LuaCommon.h"
#include "../entity/Entity.h"
#include "../entity/TextObject.h"
#include "../game/GameManager.h"
#include "../map/Map.h"

Map* _getMap()
{
	ASSERT(game->mMap);
	return game->mMap;
}

// .SetPosition(x, y, centered<1>) - Set camera position. If centered is 0, (x,y) will be the top-left coordinates.
int camera_SetPosition(lua_State* ls)
{
	PRINT("camera_SetPosition");
	luaCountArgs(ls, 2);

	int centered = 1;
	
	int numArgs = lua_gettop(ls);
	if (numArgs > 2)
		centered = (int)lua_tonumber(ls, 3);
	
	point2d p( (sShort)lua_tonumber(ls, 1), (sShort)lua_tonumber(ls, 2) );
	
	Map* m = _getMap();
	m->SetCameraPosition( p, centered );
	
	return 0;
}

// x, y = .GetPosition() 
int camera_GetPosition(lua_State* ls)
{
	PRINT("camera_GetPosition");

	Map* m = _getMap();
	rect r = m->GetCameraPosition();
	
	lua_pushnumber(ls, r.x);
	lua_pushnumber(ls, r.y);
	return 2;
}

// w, h = .GetSize() - Will most likely return screen dimensions
int camera_GetSize(lua_State* ls)
{
	PRINT("camera_GetSize");

	Map* m = _getMap();
	rect r = m->GetCameraPosition();
	
	lua_pushnumber(ls, r.w);
	lua_pushnumber(ls, r.h);
	return 2;
}

// .Offset(x, y) - Offset by (x,y) pixels. (Negative values to move up and left, positive to move down and right)
int camera_Offset(lua_State* ls)
{
	PRINT("camera_Offset");
	luaCountArgs(ls, 2);

	point2d p( (sShort)lua_tonumber(ls, 1), (sShort)lua_tonumber(ls, 2) );
	
	Map* m = _getMap();
	m->OffsetCamera( p );
	
	return 0;
}

// .Follow(entity) - Tell the camera to follow the specified entity. Note: If this entity is deleted, everything will probably go to hell.
int camera_Follow(lua_State* ls)
{
	PRINT("camera_Follow");
	luaCountArgs(ls, 1);

	Map* m = _getMap();
	m->SetCameraFollow( (Entity*)lua_touserdata(ls, 1) );
	
	return 0;
}

// ent = .GetFollowed() - Return the entity the camera is currently following
int camera_GetFollowed(lua_State* ls)
{
	PRINT("camera_GetFollowed");

	Map* m = _getMap();

	lua_pushlightuserdata( ls, m->GetCameraFollow() );
	return 1;
}

// .NoFollow() - Disable entity following
int camera_NoFollow(lua_State* ls)
{
	PRINT("camera_NoFollow");

	Map* m = _getMap();
	m->SetCameraFollow(NULL);
	
	return 0;
}

// .SetFollowOffset(x, y) - Set the offset from the entity we're following with the camera
int camera_SetFollowOffset(lua_State* ls)
{
	PRINT("camera_SetFollowOffset");
	luaCountArgs(ls, 2);

	point2d p( (sShort)lua_tonumber(ls, 1), (sShort)lua_tonumber(ls, 2) );
	
	Map* m = _getMap();
	m->SetCameraFollowOffset( p );
	
	return 0;
}

// x, y = .GetFollowOffset()
int camera_GetFollowOffset(lua_State* ls)
{
	PRINT("camera_GetFollowOffset");

	Map* m = _getMap();
	point2d p = m->GetCameraFollowOffset();
	
	lua_pushnumber(ls, p.x);
	lua_pushnumber(ls, p.y);
	return 2;
}

// .IsRectVisible(x, y, w, h) - Returns 1 if the rect is visible in the camera, 0 otherwise.
int camera_IsRectVisible(lua_State* ls)
{
	PRINT("camera_IsRectVisible");
	luaCountArgs(ls, 4);
	
	Map* m = _getMap();

	rect r; 
	r.x = (sShort)lua_tonumber(ls, 1);
	r.y = (sShort)lua_tonumber(ls, 2);
	r.w = (uShort)lua_tonumber(ls, 3);
	r.h = (uShort)lua_tonumber(ls, 4);
	
	int visible = m->IsRectInCamera(r);
	lua_pushnumber(ls, visible);
	
	return 1;
}

// .IsEntityVisible(entity) - Returns 1 if the entity is visible in the camera, 0 otherwise.
int camera_IsEntityVisible(lua_State* ls)
{
	PRINT("camera_IsEntityVisible");
	luaCountArgs(ls, 1);
	
	Map* m = _getMap();
	Entity* e = (Entity*)lua_touserdata(ls, 1);

	int visible = 0;
	if ( m->FindEntity(e) != -1 )
		visible = e->IsVisibleInCamera();

	lua_pushnumber(ls, visible);
	
	return 1;
}

//	.IsPanning() - return 1 if the camera is currently panning to a destination, 0 otherwise
int camera_IsPanning(lua_State* ls)
{
	PRINT("camera_IsPanning");
	Map* m = _getMap();
	
	lua_pushnumber( ls, !m->mCameraDestinationStack.empty() );
	
	return 1;
}

//	.PanTo(x, y) - Erase the destination list in the camera and pan directly to the coordinates.
int camera_PanTo(lua_State* ls)
{
	PRINT("camera_PanTo");
	luaCountArgs(ls, 2);
	
	Map* m = _getMap();
	
	point2d p( (sShort)lua_tonumber(ls, 1), (sShort)lua_tonumber(ls, 2) );
	m->mCameraDestinationStack.clear();
	m->AddCameraDestination(p);
	
	return 0;
}

//	.AddToPanningStack(x, y) - Add the coordinates to the end of our panning stack
int camera_AddToPanningStack(lua_State* ls)
{
	PRINT("camera_AddToPanningStack");
	luaCountArgs(ls, 2);
	
	Map* m = _getMap();
	
	point2d p( (sShort)lua_tonumber(ls, 1), (sShort)lua_tonumber(ls, 2) );
	m->AddCameraDestination(p);
	
	return 0;
}

//	.ClearPanningStack() - return 1 if the camera is currently panning to a destination, 0 otherwise
int camera_ClearPanningStack(lua_State* ls)
{
	PRINT("camera_IsPanning");
	Map* m = _getMap();
	
	m->mCameraDestinationStack.clear();
	
	return 0;
}

//	.GetSpeed() - Return pan speed
int camera_GetSpeed(lua_State* ls)
{
	PRINT("camera_GetSpeed");
	Map* m = _getMap();
	
	lua_pushnumber( ls, m->mCameraSpeed );
	
	return 1;
}

//	.SetSpeed(speed) - Set pan speed
int camera_SetSpeed(lua_State* ls)
{
	PRINT("camera_SetSpeed");
	Map* m = _getMap();

	m->mCameraSpeed = (uShort)lua_tonumber(ls, 1);
	
	return 0;
}

static const luaL_Reg functions[] = {
	{"SetPosition", camera_SetPosition},
	{"GetPosition", camera_GetPosition},
	{"GetSize", camera_GetSize},
	{"Offset", camera_Offset},
	{"Follow", camera_Follow},
	{"GetFollowed", camera_GetFollowed},
	{"NoFollow", camera_NoFollow},
	{"SetFollowOffset", camera_SetFollowOffset},
	{"GetFollowOffset", camera_GetFollowOffset},
	{"IsRectVisible", camera_IsRectVisible},
	{"IsEntityVisible", camera_IsEntityVisible},
	
	//WORK IN PROGRESS
	{"IsPanning", camera_IsPanning},
	{"PanTo", camera_PanTo},
	{"AddToPanningStack", camera_AddToPanningStack},
	{"ClearPanningStack", camera_ClearPanningStack},
	{"GetSpeed", camera_GetSpeed},
	{"SetSpeed", camera_SetSpeed},
	{NULL, NULL}
};

void RegisterCameraLib(lua_State* ls)
{
	luaL_register( ls, "Camera", functions );
}





