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

#define MAX_AVATAR_WIDTH 64
#define MAX_AVATAR_HEIGHT 96

typedef enum
{
	ENTITY_ANY = 0,
	ENTITY_ACTOR,
	ENTITY_REMOTEACTOR,
	ENTITY_LOCALACTOR,
	ENTITY_SCENEACTOR,
	//Place other actor-inherited entities here.
	ENTITY_END_ACTORS, //Do not use, just as a marker
	ENTITY_STATICOBJECT,
	ENTITY_EFFECT,
	ENTITY_WARP,
	ENTITY_SKILL
} entityType;

/*	Base class for objects on the map (players, npcs, monsters, objects, etc)
	base for generic functions such as collision detection and rendering.
*/

class Image;
class Map;
class Entity
{
  public:
	Entity();
	virtual ~Entity();

	//Return a string representation of this entity type
	string GetTypeName();
	
	virtual void Render(uLong ms) {};
	void RenderShadow();
	
	virtual void SetPosition(point2d position);
	virtual point2d GetPosition() const { return mPosition; };

	//Read configuration settings for this entity from an XML file
	//Param: online: If true, will access cache files, (false for when using editors)
	//	Online parameter is a complete hack. A better technique needs to be figured out.
	virtual int ReadXml(XmlFile* xf, TiXmlElement* e, bool online);
	
	//Write configuration settings for this entity to an XML file
	virtual void WriteXml(XmlFile* xf, TiXmlElement* root) {};
	
	/*	Returns true if any of this entities collision rects intersect r */
	bool CollidesWith(rect r);

	/*	Returns true if any of this entities rects are intersecting a solid entity */
	bool IsCollidingWithSolid();
	
	/*	Returns true if any of this entities rects are intersecting the entity */
	bool IsCollidingWithEntity(Entity* e);

	void SetVisible(bool v);
	bool IsVisible() { return mVisible; };

	//Return a rectangle that marks this entities bounds. This can be a collision rect, or it can be the bounds of 
	//the entities renderable image.  just depends on the inherited entity in question.
	//Relative to map coordinates.
	virtual rect GetBoundingRect();

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
	byte GetLayer() const { return mLayer; }; 

	void SetLayer(byte l);
	
	bool IsSolid() const { return mSolid; };
	void SetSolid(bool b) { mSolid = b; };
	
	void SetFlag(string flag, string value);
	string GetFlag(string flag);
	
	bool mLocked; //Map Editor only
	bool mShadow; //does this entity cast a shadow
	
	sShort mJumpHeight;
 // protected:
	bool mSolid; //can other entities pass through our collision rects?
	bool mVisible;
	byte mLayer;
	
	std::map<string, string> mFlags;
};

#endif //_ENTITY_H_
