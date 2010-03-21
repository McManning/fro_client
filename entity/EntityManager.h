
#ifndef _ENTITYMANAGER_H_
#define _ENTITYMANAGER_H_

#include "../core/Core.h"
#include "Entity.h"

//Used to retrieve and set entities layer on the map.
enum //entityLevel
{
	ENTITYLEVEL_ANY = -1,
	ENTITYLEVEL_GROUND,
	ENTITYLEVEL_USER,
	ENTITYLEVEL_SKY,
	ENTITYLEVEL_COUNT //<-- has to be the last member
};

class Entity;
class EntityManager
{
  public:
	EntityManager();
	~EntityManager();

	bool RemoveEntity(Entity* e, sShort level = ENTITYLEVEL_ANY);
	bool RemoveEntity(string id, byte type = ENTITY_ANY, sShort level = ENTITYLEVEL_ANY);
	bool RemoveEntityByName(string name, byte type = ENTITY_ANY, sShort level = ENTITYLEVEL_ANY);
	bool RemoveAllEntitiesById(string id, byte type = ENTITY_ANY, sShort level = ENTITYLEVEL_ANY);
	
	//Delete all entities in this manager
	void FlushEntities();
	
	/*	Locate entity by Entity::mId. Can define specific types and levels. Leave as default to search anywhere and anyone.
		Returns NULL if not found, the entity otherwise
	*/
	Entity* FindEntityById(string id, byte type = ENTITY_ANY, sShort level = ENTITYLEVEL_ANY);
		
	/*	Locate entity by Entity::mName. Can define specific types and levels. Leave as default to search anywhere and anyone.
		Returns NULL if not found, the entity otherwise
	*/
	Entity* FindEntityByName(string name, byte type = ENTITY_ANY, sShort level = ENTITYLEVEL_ANY);
	
	//checks if entity exists in this manager, on the specified level and of the specified type.
	bool EntityExists(Entity* e, byte type = ENTITY_ANY, sShort level = ENTITYLEVEL_ANY);

	//Will automatically remove any duplicates of the entity instance from the lists before adding.
	virtual void AddEntity(Entity* e, sShort level);
	
	//Tell the manager to resort entities before rendering (Flagged so it won't be sorted an unnecessary number of times)
	void QueueEntityResort();
	void ResortEntities();
	
	//Will send a message to the messenger indicating that the entity has been created
	void DispatchEntityCreateMessage(Entity* e);
	
	Entity* FindTopmostRemotePlayer(rect testRect); //TODO: NOT Entity return
	
	/*
		Returns the first topmost entity with http:// prefixing its name
	*/
	Entity* FindTopmostUrlEntityUnderRect(rect testRect);
	
	/*
		Lists of entities that exist on this map. They can be static objects, 
		npcs, players, etc. (Players and Npcs will be most common in userLevel except
		for the case of npcs that have the wonderful ability to defy gravity)
	*/
	std::vector<Entity*> entityLevel[ENTITYLEVEL_COUNT];

	Image* mLoadingAvatarIcon;

  private:
	
	bool mNeedToResort;
	
	sShort _find(string id, std::vector<Entity*>* v, byte type = ENTITY_ANY);
	sShort _findByName(string name, std::vector<Entity*>* v, byte type = ENTITY_ANY);
	sShort _find(Entity* e, std::vector<Entity*>* v);
	
	void _delete(Entity* e);
};

#endif //_ENTITYMANAGER_H_
