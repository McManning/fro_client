/*	
	/command <param> <optional:default> 
	/loadmap id
	/savemap
	/addobject id 

	Keys:
	*delete - Delete held entity from the map
	home (?) - Clone held entity, and drops clone at held position.
	*F1 - Toggle move state. If enabled, held objects can be repositioned as the mouse moves.
	*pageup - move held object layer up
	*pagedown - move held object layer down
	*arrow keys - shift camera position

*/

#ifndef _EDITORMAP_H_
#define _EDITORMAP_H_

#include "../core/Core.h"
#include "../entity/EntityManager.h"
#include "../map/Map.h"

#define CAMERA_OFFSET_SPEED (64)

class EditorMap : public Map
{
  public:
	EditorMap();
	~EditorMap();
	
	void Event(SDL_Event* event);
	void Process(uLong ms);
	void Render(uLong ms);
		
	void RenderGrid(rect r);

	/*	Clone held object */
	void Clone();
	
	/*	Return topmost object under mouse */
	Entity* GetObjectUnderMouse();
	
	bool CanMoveHeldObject();
	
	/*	If the entity exists in the unique list, will create a clone at pos.
		Otherwise, will load a new unique entity from memory, add to list, and return.
		Returns the clone entity if created successfully, NULL otherwise.
	*/
	Entity* AddUniqueEntity(string id, point2d pos);

	/*	Loads script data (in the case of the editor, just the id for storage) 
		Returns false if the script isn't found, or "invalid" (This doesn't deal with lua at all!).
	*/
	bool LoadScript(string id);

	/*	Read DIR_MAPS/id.res into array, exploded via \n
		For each item, _loadUniqueEntity(id)
		return false if the resource list is not found, or malformed
		id is provided as a parameter so that we can load multiple resource files in. 
	*/
	bool ReadResourceList(string id);
	
	/*	Load up DIR_MAPS/id.xml, using <layout> as root, 
		for each sibling check type, create new inherited entity
		based on type, read in properties, add to map. 
		return false if the xml is not found, or malformed
	*/
	bool LoadXml();
	
	/*	Xml Parser callback */
	int ParseXmlElement(XmlFile* xf, TiXmlElement* e);
	
	/*	Loads resource list and layout */
	bool Load(string id);
	
	/*	for each unique entity, output id\n */
	bool SaveResourceList();
	
	/*	write out <layout>, for each entity, write out associated
		xml. (Type, properties, etc)
	*/
	bool SaveXml();
	
	bool Save();
	
	/*	Runs through every DIR_ENTITIES/*.xml and loads into the unique objects list. 
		Used for when we want easy access to every entity available to us.
	*/
	bool LoadAllResources();
	
	//TODO: These
	bool LoadProperties() {};
	bool QueueResources() {};
	bool LoadResources() {};
	bool LoadLayout() {};
	
	/*	File we loaded this map from: DIR_MAPS/mId.xml */
	string mId;

	Entity* mHeldObject;
	bool mCanMoveObjects;
	
	point2d mHeldOffset;
	
	byte mAmountOfObjectInfo; //0: no info, 1: little info, 2: lot of info
	
	double mRotationStep; //speed in which objects rotate (1->360)
	double mScaleStep; //speed in which to scale up/down images
	
	bool mShowGrid;
	bool mSnapToGrid;
	bool mEditSpawn;
	
	//Loaded script names
	vString mScriptList;
	
  private:
	void _renderEntities(uLong ms);
	void _adjustHeldObject();
	void _adjustSpawn();
	void _calculateHeldOffset();

	void _deleteHeldObject();
	
	/*	Drop a clone at the current position of mHeldObject */
	void _cloneHeldObject();
	
	/*	Toggle edit locking on the held object */
	void _lockHeldObject();
	
	/*	Loads specific entity data from XML and returns
		the new entity if successful, NULL otherwise. */
	Entity* _loadUniqueEntity(string id);

	/*	Fill in mWidth/mHeight based on the positions of the entities currently occupying the map.
		Just big enough to contain all entities bounding rects. */
	void _calculateDimensions();

	Image* mGridImage;
	Image* mLockIcon; //icon rendered over position locked entities
	Image* mSpawnFlagIcon;
};


#endif //_EDITORMAP_H_
