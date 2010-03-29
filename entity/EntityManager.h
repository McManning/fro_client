
#ifndef _ENTITYMANAGER_H_
#define _ENTITYMANAGER_H_

#include "../core/Core.h"
#include "Entity.h"

class Entity;
class EntityManager
{
  public:
	EntityManager();
	~EntityManager();
	
	enum //Some common entity levels
	{
		LAYER_GROUND = 1000,
		LAYER_USER = 1001,
		LAYER_SKY = 1002,
	};

	bool RemoveEntity(Entity* e);
	
	bool RemoveEntityById(string id, byte type = ENTITY_ANY);
	bool RemoveEntityByName(string name, byte type = ENTITY_ANY);
	
	bool RemoveAllEntitiesById(string id, byte type = ENTITY_ANY);
	bool RemoveAllEntitiesByName(string name, byte type = ENTITY_ANY);
	
	//Delete all entities in this manager
	void FlushEntities();
	
	/*	Locate entity by Entity::mId. Can define specific types and levels. Leave as default to search anywhere and anyone.
		Returns NULL if not found, the entity otherwise
	*/
	Entity* FindEntityById(string id, byte type = ENTITY_ANY);
		
	/*	Locate entity by Entity::mName. Can define specific types and levels. Leave as default to search anywhere and anyone.
		Returns NULL if not found, the entity otherwise
	*/
	Entity* FindEntityByName(string name, byte type = ENTITY_ANY);
	
	//	Returns the index number of the selected entity. Or -1 if it isn't found
	int FindEntity(Entity* e);
	
	/*	Will add the specified entity to the manager. If it already exists, this will do nothing. 
		Dispatches ENTITY_CREATE if successful
	*/
	void AddEntity(Entity* e);
	
	//Tell the manager to resort entities before rendering (Flagged so it won't be sorted an unnecessary number of times)
	void QueueEntityResort();
	void ResortEntities();
	
	//Will send a message to the messenger indicating that the entity has been created
	void DispatchEntityCreateMessage(Entity* e);

	/*
		Lists of entities that exist on this map. They can be static objects, 
		npcs, players, etc. (Players and Npcs will be most common in userLevel except
		for the case of npcs that have the wonderful ability to defy gravity)
	*/
	std::vector<Entity*> mEntities;

	Image* mLoadingAvatarIcon;

  private:
	
	bool mNeedToResort;
	void _delete(Entity* e);
};

#endif //_ENTITYMANAGER_H_
