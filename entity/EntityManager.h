
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
		LAYER_STATIC_LOWER_START = 0,
		LAYER_STATIC_LOWER_END = 500,
		LAYER_GROUND = 1000,
		LAYER_USER = 2000,
		LAYER_SKY = 3000,
		LAYER_STATIC_UPPER_START = 5000,
		LAYER_STATIC_UPPER_END  = 5500,
	};

	bool RemoveEntity(Entity* e);
	
	bool RemoveEntityById(string id, entityType type = ENTITY_ANY);
	bool RemoveEntityByName(string name, entityType type = ENTITY_ANY);
	
	bool RemoveAllEntitiesById(string id, entityType type = ENTITY_ANY);
	bool RemoveAllEntitiesByName(string name, entityType type = ENTITY_ANY);
	
	//Delete all entities in this manager
	void FlushEntities();
	
	/*	Locate entity by Entity::mId. Can define specific types and levels. Leave as default to search anywhere and anyone.
		Returns NULL if not found, the entity otherwise
	*/
	Entity* FindEntityById(string id, entityType type = ENTITY_ANY);
		
	/*	Locate entity by Entity::mName. Can define specific types and levels. Leave as default to search anywhere and anyone.
		Returns NULL if not found, the entity otherwise
	*/
	Entity* FindEntityByName(string name, entityType type = ENTITY_ANY);
	
	//	Returns false if the entity isn't found, true otherwise
	bool FindEntity(Entity* e);
	
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

  private:
	
	bool mNeedToResort;
	void _delete(Entity* e);
	void _addToDeleteQueue(Entity* e);
	void _deleteQueuedEntities();
};

#endif //_ENTITYMANAGER_H_
