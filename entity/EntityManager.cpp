
#include "EntityManager.h"
#include <algorithm> //stable_sort, find

EntityManager::EntityManager()
{
	mNeedToResort = false;
}

EntityManager::~EntityManager()
{
	FlushEntities();
}

bool EntityManager::RemoveEntity(Entity* e)
{
	_addToDeleteQueue(e);
	return true;
	
	/*if (!e) return true;
	
	for (int i = 0; i < mEntities.size(); ++i)
	{
		if (mEntities.at(i) == e)
		{
			_delete(mEntities.at(i));
			mEntities.erase(mEntities.begin() + i);
			return true;
		}
	}
	return false;*/
}

bool EntityManager::RemoveEntityById(string id, entityType type)
{
	for (int i = 0; i < mEntities.size(); ++i)
	{
		if (mEntities.at(i) && mEntities.at(i)->mId == id 
			&& (mEntities.at(i)->mType == type || type == ENTITY_ANY))
		{
			_addToDeleteQueue(mEntities.at(i));
			return true;
		}
	}
	return false;
}

bool EntityManager::RemoveEntityByName(string name, entityType type)
{
	for (int i = 0; i < mEntities.size(); ++i)
	{
		if (mEntities.at(i) && mEntities.at(i)->GetName() == name
			&& (mEntities.at(i)->mType == type || type == ENTITY_ANY))
		{
			_addToDeleteQueue(mEntities.at(i));
			return true;
		}
	}
	return false;
}

bool EntityManager::RemoveAllEntitiesById(string id, entityType type)
{
	bool found = false;
	for (int i = 0; i < mEntities.size(); ++i)
	{
		if (mEntities.at(i) && mEntities.at(i)->mId == id 
			&& (mEntities.at(i)->mType == type || type == ENTITY_ANY))
		{
			_addToDeleteQueue(mEntities.at(i));
			found = true;
		}
	}

	return found;
}

bool EntityManager::RemoveAllEntitiesByName(string name, entityType type)
{
	bool found = false;
	for (int i = 0; i < mEntities.size(); ++i)
	{
		if (mEntities.at(i) && mEntities.at(i)->GetName() == name 
			&& (mEntities.at(i)->mType == type || type == ENTITY_ANY))
		{
			_addToDeleteQueue(mEntities.at(i));
			found = true;
		}
	}

	return found;
}

void EntityManager::FlushEntities()
{
	for (int i = 0; i < mEntities.size(); ++i)
		_delete(mEntities.at(i));

	mEntities.clear();
}

Entity* EntityManager::FindEntityById(string id, entityType type)
{
	for (int i = 0; i < mEntities.size(); ++i)
	{
		if (mEntities.at(i) && mEntities.at(i)->mId == id 
			&& (mEntities.at(i)->mType == type || type == ENTITY_ANY))
		{
			return mEntities.at(i);
		}
	}
	return NULL;
}

Entity* EntityManager::FindEntityByName(string name, entityType type)
{
	for (int i = 0; i < mEntities.size(); ++i)
	{
		if (mEntities.at(i) && mEntities.at(i)->GetName() == name)
		{
			if (mEntities.at(i)->mType == type || type == ENTITY_ANY)
			{
				return mEntities.at(i);
			}
		}
	}
	return NULL;
}

// Sort based on both entity layer and position, 
// keeping the lowest level entities at the bottom of the list
bool entitySort( Entity* a, Entity* b )
{
	if ( a->GetLayer() < b->GetLayer())
		return true;
	
	if ( a->GetLayer() > b->GetLayer())
		return false;

	return ( (a->mPosition.y < b->mPosition.y) );
}

bool EntityManager::FindEntity(Entity* e)
{
	if (e)
		return binary_search(mEntities.begin(), mEntities.end(), e, entitySort);

	return false;
}

void EntityManager::AddEntity(Entity* e)
{
	ASSERT(e);

	//if we already have it, don't add again
	if (FindEntity(e))
		return;
	
	mEntities.push_back(e);
	DispatchEntityCreateMessage(e);
	
	QueueEntityResort();
}


void EntityManager::QueueEntityResort()
{
	mNeedToResort = true;
}

void EntityManager::ResortEntities()
{
	if (!mNeedToResort)
		return;
		
	mNeedToResort = false;

	// do a bit of cleanup
	_deleteQueuedEntities();

	stable_sort( mEntities.begin(), mEntities.end(), entitySort);
}

void EntityManager::_delete(Entity* e)
{
	if (e->mManagerCanDeleteMe)
	{	
		e->AddPositionRectForUpdate(); // preparing for removal
		
		SAFEDELETE(e);
	}
	else
	{
		e->mMap = NULL;	
	}
}

void EntityManager::DispatchEntityCreateMessage(Entity* e)
{
	MessageData md("ENTITY_CREATE");
	md.WriteUserdata("entity", e);
	messenger.Dispatch(md, e);
}

void EntityManager::_addToDeleteQueue(Entity* e)
{
	e->mDead = true;
	mNeedToResort = true;
	
	MessageData md("ENTITY_DESTROY");
	md.WriteUserdata("entity", e);
	messenger.Dispatch(md, e);
}

void EntityManager::_deleteQueuedEntities()
{
	for (int i = 0; i < mEntities.size(); ++i)
	{
		if (mEntities.at(i)->mDead)
		{
			_delete(mEntities.at(i));
			mEntities.erase(mEntities.begin() + i);
			--i;
		}
	}
}

