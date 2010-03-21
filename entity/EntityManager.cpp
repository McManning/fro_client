
#include "EntityManager.h"
#include <algorithm> //stable_sort

EntityManager::EntityManager()
{
	mNeedToResort = false;
	
	mLoadingAvatarIcon = resman->LoadImg("assets/avatarloadanim.png");
	mLoadingAvatarIcon->ConvertToHorizontalAnimation( rect(0, 0, 46, 40), 100 );

}

EntityManager::~EntityManager()
{
	FlushEntities();
	
	resman->Unload(mLoadingAvatarIcon);
}

bool EntityManager::RemoveEntity(Entity* e, sShort level)
{
	if (!e) return true;
	
	sShort index;
	for (uShort i = 0; i < ENTITYLEVEL_COUNT; i++)
	{
		if (level == i || level == ENTITYLEVEL_ANY)
		{
			index = _find(e, &entityLevel[i]);
			if (index > -1)
			{
				_delete(entityLevel[i].at(index));
				entityLevel[i].erase(entityLevel[i].begin() + index);
				return true;
			}
		}
	}
	return false;
}

bool EntityManager::RemoveEntity(string id, byte type, sShort level)
{
	sShort index;
	for (uShort i = 0; i < ENTITYLEVEL_COUNT; i++)
	{
		if (level == i || level == ENTITYLEVEL_ANY)
		{
			index = _find(id, &entityLevel[i], type);
			if (index > -1)
			{
				_delete(entityLevel[i].at(index));
				entityLevel[i].erase(entityLevel[i].begin() + index);
				return true;
			}
		}
	}
	return false;
}

bool EntityManager::RemoveEntityByName(string name, byte type, sShort level)
{
	sShort index;
	for (uShort i = 0; i < ENTITYLEVEL_COUNT; i++)
	{
		if (level == i || level == ENTITYLEVEL_ANY)
		{
			index = _findByName(name, &entityLevel[i], type);
			if (index > -1)
			{
				_delete(entityLevel[i].at(index));
				entityLevel[i].erase(entityLevel[i].begin() + index);
				return true;
			}
		}
	}
	return false;
}

bool EntityManager::RemoveAllEntitiesById(string id, byte type, sShort level)
{
	bool found = false;
	for (uShort i = 0; i < ENTITYLEVEL_COUNT; i++) //Iterate all lists
	{
		if (level == i || level == ENTITYLEVEL_ANY) //If this list is acceptable..
		{
			for (uShort ii = 0; ii < entityLevel[i].size(); ii++) //Iterate all entities in that list
			{
				if (entityLevel[i].at(i)->mId == id)
				{
					if (entityLevel[i].at(i)->mType == type || type == ENTITY_ANY) //If Valid Entity, kill it
					{
						_delete(entityLevel[i].at(ii));
						entityLevel[i].erase(entityLevel[i].begin() + ii);
						ii--;
						found = true;
					}
				}
			}
		}
	}
	return found;
}

void EntityManager::FlushEntities()
{
	uShort i, ii;
	for (i = 0; i < ENTITYLEVEL_COUNT; i++) //Iterate all lists
	{
		for (ii = 0; ii < entityLevel[i].size(); ii++)
			_delete(entityLevel[i].at(ii));

		entityLevel[i].clear();
	}
}

Entity* EntityManager::FindEntityById(string id, byte type, sShort level)
{
	if (level >= ENTITYLEVEL_COUNT)
		return NULL;
	
	sShort index;
	byte i;
	for (i = 0; i < ENTITYLEVEL_COUNT; i++)
	{
		if (level == i || level == ENTITYLEVEL_ANY)
		{
			index = _find(id, &entityLevel[i], type);
			if (index > -1)
				return entityLevel[i].at(index);
		}
	}
	return NULL;
}

Entity* EntityManager::FindEntityByName(string name, byte type, sShort level)
{
	if (level >= ENTITYLEVEL_COUNT)
		return NULL;
	
	sShort index;
	byte i;
	for (i = 0; i < ENTITYLEVEL_COUNT; i++)
	{
		if (level == i || level == ENTITYLEVEL_ANY)
		{
			index = _findByName(name, &entityLevel[i], type);
			if (index > -1)
				return entityLevel[i].at(index);
		}
	}
	return NULL;
}

bool EntityManager::EntityExists(Entity* e, byte type, sShort level)
{
	if (level >= ENTITYLEVEL_COUNT || !e)
		return false;

	byte i;
	for (i = 0; i < ENTITYLEVEL_COUNT; i++)
	{
		if (level == i || level == ENTITYLEVEL_ANY) {
			if ( _find(e, &entityLevel[i]) > -1 )
				return true;
		}
	}
	return false;
}

void EntityManager::AddEntity(Entity* e, sShort level)
{
	ASSERT(e);

	//if they're at an invalid level, just set to top
	if (level >= ENTITYLEVEL_COUNT)
		level = ENTITYLEVEL_COUNT - 1;

	if (level < 0)
		level = 0;

	//Check for overflowing the vectors
	ASSERT(entityLevel[level].size() < SHRT_MAX)

	sShort index;
	//if it's already somewhere on the map, erase the old version
	for (uShort i = 0; i < ENTITYLEVEL_COUNT; i++)
	{
		index = _find(e, &entityLevel[i]);
		if (index > -1)
		{
			entityLevel[i].erase(entityLevel[i].begin() + index);
			break;
		}
	}
	
	entityLevel[level].push_back(e);
	e->mLayer = level;

	QueueEntityResort();
}

void EntityManager::QueueEntityResort()
{
	mNeedToResort = true;
}

bool entitySort( Entity* a, Entity* b )
{
   /*  //if there's a match between two, object type always wins.
     if (a->mPosition == b->getY()) {
		if (first->type == ENTITY_OBJECT) return false;
		if (second->type == ENTITY_OBJECT) return true;
	}*/
	return a->mPosition.y < b->mPosition.y;
}

void EntityManager::ResortEntities()
{
	if (!mNeedToResort)
		return;
		
	mNeedToResort = false;

	//Sort all layers
	//OPTIMIZETODO: Only sort the layers that have changed to reduce processing.
	//Or collect a vector of entities that moved, and then shift them.
	byte i;
	for (i = 0; i < ENTITYLEVEL_COUNT; i++)
	{
		//TODO: replace with qsort!
		stable_sort( entityLevel[i].begin(), entityLevel[i].end(), entitySort );
	}
}

sShort EntityManager::_find(string id, std::vector<Entity*>* v, byte type)
{
	for (uShort i = 0; i < v->size(); i++)
	{
		if (v->at(i)->mId == id)
		{
			if (v->at(i)->mType == type || type == ENTITY_ANY)
				return i; //Return only if it's the valid searched type, or if given an ambiguous search
		}
	}
	return -1;
}

sShort EntityManager::_findByName(string name, std::vector<Entity*>* v, byte type)
{
	for (uShort i = 0; i < v->size(); i++)
	{
		if (v->at(i)->mName == name)
		{
			if (v->at(i)->mType == type || type == ENTITY_ANY)
				return i; //Return only if it's the valid searched type, or if given an ambiguous search
		}
	}
	return -1;
}

sShort EntityManager::_find(Entity* e, std::vector<Entity*>* v)
{
	for (uShort i = 0; i < v->size(); i++)
	{
		if (v->at(i) == e)
			return i;
	}
	return -1;
}

void EntityManager::_delete(Entity* e)
{
	PRINT("\t\t\tDELETING " + e->mId);

	if (e->mType != ENTITY_LOCALACTOR) //Can't delete the local player manager
	{	
		MessageData md("ENTITY_DESTROY");
		md.WriteUserdata("entity", e);
		messenger.Dispatch(md, e);
		
		SAFEDELETE(e);
	}
}

/*
	UPGRADETODO: Pixel level testing. But.. I don't care that much.
		But for the record, we would just offset the pixel address by the difference
		between the point and our test rect, and then test the alpha level or colorkey
		value. If it's blank, then this entity couldn't be selected and we move on.
		Of course the issue with this is blank avatars, in which case we'd need another
		solution. (Actually, single pixel avatars pose the same problem too)
*/
Entity* EntityManager::FindTopmostRemotePlayer(rect testRect) //TODO: NOT Entity return
{
	Entity* a;

	//Iterate in reverse since the topmost entities are at the top of the list
	for (sShort i = entityLevel[ENTITYLEVEL_USER].size() - 1; i >= 0; i--)
	{
		a = entityLevel[ENTITYLEVEL_USER].at(i);
		if ( a && (a->mType == ENTITY_REMOTEACTOR)
				&& areRectsIntersecting(a->GetBoundingRect(), testRect) )
		{
			return a;
		}
	}

	return NULL;
}

/*
	Returns the first topmost entity with http:// in its name and returns
*/
Entity* EntityManager::FindTopmostUrlEntityUnderRect(rect testRect)
{
	Entity* e;

	int i, ii;
	for (i = ENTITYLEVEL_COUNT-1; i >= 0; --i) //Iterate all lists
	{
		for (ii = entityLevel[i].size()-1; ii >= 0; --ii) //Iterate all entities in that list
		{
			e = entityLevel[i].at(ii);
			if ( e && (e->mName.find("http://", 0) == 0)
				&& areRectsIntersecting(e->GetBoundingRect(), testRect) )
			{
				return e;
			}
		}
	}

	return NULL;
}

void EntityManager::DispatchEntityCreateMessage(Entity* e)
{
	MessageData md("ENTITY_CREATE");
	md.WriteUserdata("entity", e);
	messenger.Dispatch(md, e);
}
