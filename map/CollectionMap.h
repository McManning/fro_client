/*
Map Load Procedure:
1. Download xml from master
2. Read base properties
3. If CollectionMap type, create new CollectionMap and link xml
	4. CollectionMap::QueueResources()
	5. Wait for all resources to finish downloading (successfully)
	6. CollectionMap::LoadScripts()
	7. CollectionMap::LoadLayout()
	8. Finalize by adding local player
3. If MazeMap type, create new MazeMap and link xml
	4. MazeMap::QueueResources() (files necessary to generate walls, etc)
	5. Wait for all resources to finish downloading (successfully)
	6. MazeMap::LoadScripts()
	7. MazeMap::LoadLayout() (generate maze walls, place objects)
	8. Finalize by adding local player

BASICALLY, MazeMap will be inherited from CollectionMap. We should put Queue/Load 
into Map base. We could then also inherit them for EditorMap to load from a different
location. 
*/

#ifndef _COLLECTIONMAP_H_
#define _COLLECTIONMAP_H_

#include "Map.h"

/*
	Basic game map. Contains a collection of entities, and renders all normally.
*/
class CollectionMap : public Map
{
  public:
	CollectionMap();
	virtual ~CollectionMap();
	
	/*	Render all visible entities */
	virtual void Render(uLong ms);

	/*	Update camera, entity order, etc */
	virtual void Process(uLong ms);
	
	/*	Returns true if a solid entity has a collision rect intersecting
		our test map rect */
	bool IsRectBlocked(rect r);
	
	/*	Load global map properties from <properties> */
	virtual bool LoadProperties();
	
	/*	Go through all children of <resources> and <scripts> in mXml and queues necessary 
		external resources for download (images, lua files, etc) */
	virtual bool QueueResources();
	
	/*	Go through all children of <scripts> in mXml and load into memory */
	virtual bool LoadScripts();
	
	/*	Go through all children of <layout> in mXml. Make clones of uniques and set properties 
		At this point, we're assuming that all uniques have their resources loaded.
	*/
	virtual bool LoadLayout();
	
	/*	Will attempt to locate that entity in <resources>, load an entity from the settings in that, and return. 
		NULL returns if the entity isn't a resource (Actually, it'll throw a fatal error currently ;D) */
	Entity* AddEntityFromResources(string id, point2d pos);
		
  private:
	/*	Strip the files we need from each resource and send to the download if we don't have it locally */
	bool _queueResource(XmlFile* xf, TiXmlElement* e);
	
	/*	Makes sure the directory structure is there, queues up file download, and adds to total resource count 
		Returns true if success, false if anything screwed up 
	*/
	bool _queueFile(string url, string file, string hash);
	
	/*	Load a new entity and set properties */
	Entity* _loadEntityResource(XmlFile* xf, TiXmlElement* e);
	
	/*	Read resources in from a .res file and individual object xml, rather than <resources> in the main xml */
	bool _loadResFile();
	
	/*	Read in a new lua manager */
	bool _loadScript(XmlFile* xf, TiXmlElement* e);
	
	/*	Load a clone of a unique object onto the map and set the clones unique properties */
	bool _readLayoutItem(XmlFile* xf, TiXmlElement* e);
	
	void _renderEntities(uLong ms);
};

#endif //_COLLECTIONMAP_H_
