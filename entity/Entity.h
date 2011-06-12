/*
	Some notes:
		ScriptControllers are worthless bulk. Replace them with inherited entities that have
		process timers. This way we can have a warp entity and such that aren't just blank entities
		being puppeted by script controllers. 
*/

#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <map>
#include "../core/Core.h"

typedef enum
{
	ENTITY_ANY = 0,
	
	ENTITY_ACTOR = 1000,
	ENTITY_REMOTEACTOR,
	ENTITY_LOCALACTOR,
	//ENTITY_LUNEM,
	//Place other actor-inherited entities here.
	ENTITY_END_ACTORS, //Do not use, just as a marker
	
	ENTITY_STATICOBJECT = 2000,
	ENTITY_TEXT,
	ENTITY_DAMAGEICON,
	ENTITY_EFFECT,
	
} entityType;

/*	Base class for objects on the map (players, npcs, monsters, objects, etc)
	base for generic functions such as collision detection and rendering.
*/

class Image;
class Map;
class ChatBubble;
struct lua_State;
class Entity
{
  public:
	Entity();
	virtual ~Entity();

	//Return a string representation of this entity type
	string GetTypeName();
	
	virtual void Render();
	void RenderShadow();
	
	virtual Image* GetImage() { return NULL; };
	
	virtual void SetPosition(point2d position);
	virtual point2d GetPosition() const { return mPosition; };

	/*	index - Index of the stack where our new value for the property should be */
	virtual int LuaSetProp(lua_State* ls, string& prop, int index);
	virtual int LuaGetProp(lua_State* ls, string& prop);

	/*	Returns true if any of this entities collision rects intersect r */
	bool CollidesWith(rect r);

	/*	Returns true if any of this entities rects are intersecting a solid entity */
	bool IsCollidingWithSolid();
	
	/*	Returns true if any of this entities rects are intersecting the entity */
	bool IsCollidingWithEntity(Entity* e);

	bool IsVisibleInCamera();

	void SetVisible(bool v);
	bool IsVisible() { return mVisible; };
	
	virtual void SetName(string name) { mName = name; };
	const string GetName() const { return mName; };
	
	void SetShadow(bool b);
	
	bool IsPositionRelativeToScreen();
	
	void AddPositionRectForUpdate();

	//Return a rectangle that marks this entities bounds. This can be a collision rect, or it can be the bounds of 
	//the entities renderable image.  just depends on the inherited entity in question.
	//Relative to map coordinates.
	virtual rect GetBoundingRect() = 0;

	rect GetShadowRect();
	
	void Say(string msg, bool bubble = true, bool inChat = true);
	
	entityType mType;

	string mId; //classname of this entity
	string mName; //special subname of this entity
	
	//Our origin point. Collision rects (and images of inherited entities)
	//are offset from this position.
	point2d mOrigin; 

	point2d mPosition;
	point2d mPreviousPosition; //mPosition before it was changed.

	std::vector<rect> mCollisionRects; 

	/*	if this entity can only be positioned to match 16x16 tile points
		Will set it's mPosition to the bottom center of the tile it resides.
	*/
	bool mSnapToGrid;

	Map* mMap; //TODO: I do NOT like this reference. Find a better way.
	
	//get the layer this entity exists on
	int GetLayer() const { return mLayer; }; 

	void SetLayer(int l);
	
	/**
		Load a collision map file into this entities collision rect array. Where collision 
		map is a binary file containing a bunch of raw rects
		
		@param file the collision map file to load in
		@return 1 on success, 0 otherwise
	*/
	int LoadCollisionFile(string file);
		
	bool IsSolid() const { return mSolid; };
	void SetSolid(bool b) { mSolid = b; };

	void ClearActiveChatBubble();
	
	bool mLocked; //Map Editor only
	bool mShadow; //does this entity cast a shadow

 // protected:
	bool mSolid; //can other entities pass through our collision rects?
	bool mVisible;
	int mLayer;
	bool mManagerCanDeleteMe; //Can the EntityManager delete this entity?
	bool mDead; // should this be deleted by the manager?
	int mClickRange;

	ChatBubble* mActiveChatBubble;
	
	std::map<string, string> mFlags;
};

#endif //_ENTITY_H_
