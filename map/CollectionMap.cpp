
#include "CollectionMap.h"
#include "../entity/StaticObject.h"
#include "../entity/TextObject.h"
#include "../entity/SceneActor.h"
#include "../game/GameManager.h"
#include "../core/io/FileIO.h"
#include "../core/widgets/Console.h"

TimeProfiler mapRenderProfiler("CollectionMap::Render");
TimeProfiler mapProcessProfiler("CollectionMap::Process");

TimeProfiler mapEntityRenderProfiler("CollectionMap::_renderEntities");
TimeProfiler mapBaseRenderProfiler("Map::Render");

CollectionMap::CollectionMap()
	: Map()
{
	mType = COLLECTION;
	mShowPlayerNames = game->mPlayerData.GetParamInt("map", "shownames");
}

CollectionMap::~CollectionMap()
{
	PRINT("CollectionMap::~CollectionMap");
}

void CollectionMap::Render(uLong ms)
{
	mapProcessProfiler.Start();
	Process(ms); //Here until we have a timer for it
	mapProcessProfiler.Stop();
	
	mapRenderProfiler.Start();
	
	Image* scr = Screen::Instance();
	rect r = GetScreenPosition();
	
	scr->SetClip(r);
	
	//fill with background color
	scr->DrawRect(r, mBackground);
	
	mapEntityRenderProfiler.Start();
	_renderEntities(ms);
	mapEntityRenderProfiler.Stop();

	mapBaseRenderProfiler.Start();
	Map::Render(ms);
	mapBaseRenderProfiler.Stop();
		
	scr->SetClip();
	
	mapRenderProfiler.Stop();
}

TimeProfiler mapEntitesProfiler1("::_renderEntities::Render");

void CollectionMap::_renderEntities(uLong ms)
{
	Image* scr = Screen::Instance();

	uShort i, ii;
	rect r, rr;
	Entity* e;
	color c;
	for (i = 0; i < ENTITYLEVEL_COUNT; i++) //Iterate all lists
	{
		for (ii = 0; ii < entityLevel[i].size(); ii++)
		{
			e = entityLevel[i].at(ii);
			
			if (e->IsVisible() && IsRectInCamera(e->GetBoundingRect()))
			{
				mapEntitesProfiler1.Start();
				e->Render(ms); //draw entity
				mapEntitesProfiler1.Stop();

				if (mShowInfo)
				{
					//Render collision rects
					for (uShort u = 0; u < e->mCollisionRects.size(); u++)
					{
						//Render red for solid entities, pink for passable
						if (e->IsSolid())
							c = color(255,0,0);
						else
							c = color(255,0,255);
							
						r.x = e->mCollisionRects.at(u).x + e->mPosition.x - e->mOrigin.x;
						r.y = e->mCollisionRects.at(u).y + e->mPosition.y - e->mOrigin.y;
						r.w = e->mCollisionRects.at(u).w;
						r.h = e->mCollisionRects.at(u).h;
						scr->DrawRect( ToScreenPosition(r), c, false );
					}
					
					//Render bounding rect
					r = ToScreenPosition( e->GetBoundingRect() );	
					scr->DrawRect(r, color(0,255,0), false);
						
					//Render origin marker
					r.x = r.x + e->mOrigin.x - 2;
					r.y = r.y + e->mOrigin.y - 2;
					r.w = 5;
					r.h = 5;
					scr->DrawRound(r, 2, color(0,0,255));
								
					//Render position marker
					r.x = e->mPosition.x - 2;
					r.y = e->mPosition.y - 2;
					r.w = 5;
					r.h = 5;
					r = ToScreenPosition(r);
					scr->DrawRound(r, 2, color(255,255,0));
					
					//Render destination marker & line, if actor, and we're moving
					if (e->mType >= ENTITY_ACTOR && e->mType < ENTITY_END_ACTORS)
					{
						r.x = ( (Actor*)e )->GetDestination().x;
						r.y = ( (Actor*)e )->GetDestination().y;
						rr.x = e->mPosition.x;
						rr.y = e->mPosition.y;

						if (r.x != rr.x || r.y != rr.y)
						{
							r.x -= 2;
							r.y -= 2;
							r.w = 5;
							r.h = 5;
							r = ToScreenPosition(r);
							rr = ToScreenPosition(rr);
							
							scr->DrawLine(rr.x, rr.y, r.x + 2, r.y + 2, color(0,255,255), 1);
							scr->DrawRound(r, 2, color(0,255,255));
						}
					}
				}
			} //if visible & in camera
		} //for all entities in layer
	} //for each layer
}

void CollectionMap::Process(uLong ms)
{
	Map::Process(ms);
}

bool CollectionMap::IsRectBlocked(rect r)
{
	uShort i, ii;
	Entity* e;
	for (i = 0; i < ENTITYLEVEL_COUNT; i++) //Iterate all lists
	{
		for (ii = 0; ii < entityLevel[i].size(); ii++)
		{
			e = entityLevel[i].at(ii);
			
			if ( e->IsSolid() && e->CollidesWith(r) && e->mType != ENTITY_LOCALACTOR )
				return true;
		} //for all entities in layer
	} //for each layer
	
	return false;
}

bool CollectionMap::_queueFile(string url, string file, string hash)
{
	if (fileExists(file) && !hash.empty())
	{
		// Check to see if the old files hash doesn't match the new file
		string oldhash = md5file(file);
		if (oldhash != hash)
		{
			DEBUGOUT("Invalid Hash: Old[" + oldhash + "] != New[" + hash + "]");
			removeFile(file);
		}
		else
		{
			DEBUGOUT("File up to date: " + file);
			game->mLoader.mTotalResources++;
			game->mLoader.mCompletedResources++;
			return true;	
		}
	}
	
	//Make sure the directory structure is built internally
	buildDirectoryTree(file);

	game->mLoader.mTotalResources++;

#ifdef _DOWNLOADMANAGER_H_
	DEBUGOUT("Downloading: " + url + " => " + file);
	return downloader->QueueDownload(url, file, 
							&game->mLoader, 
							callback_mapResourceDownloadSuccess, 
							callback_mapResourceDownloadFailure,
							false, true, hash);
#endif
	return true;
}

bool CollectionMap::_queueResource(XmlFile* xf, TiXmlElement* e)
{
	string id = e->Value();
	
	ASSERT(game);
	
	TiXmlElement* e2;
	string file;
	string url;
	string hash;
	if (id == "object")
	{
		e2 = e->FirstChildElement("image");
		if (e2)
		{
			// Local File: cache/entities/file
			// Remote File: ROOT/entities/file
			file = DIR_ENTITIES + xf->GetParamString(e2, "file");
			url = game->mMasterUrl + file;
			file = DIR_CACHE + file;
			
			hash = xf->GetParamString(e2, "md5");
			
			if (!_queueFile(url, file, hash))
				return false;
		}
	}
	else if (id == "actor")
	{
		e2 = e->FirstChildElement("avatar");
		if (e2)
		{
			// Local File: cache/entities/file
			// Remote File: ROOT/entities/file
			file = DIR_ENTITIES + xf->GetParamString(e2, "file");
			url = game->mMasterUrl + file;
			file = DIR_CACHE + file;
			
			hash = xf->GetParamString(e2, "md5");
			
			if (!_queueFile(url, file, hash))
				return false;
		}
	}
	else if (id == "script") //<script id="something" file="script.lua" md5="hash" active="0" />
	{
		// Local File: cache/scripts/file
		// Remote File: ROOT/scripts/file
		file = DIR_SCRIPTS + xf->GetParamString(e, "file");
		url = game->mMasterUrl + file;
		file = DIR_CACHE + file;
		
		hash = xf->GetParamString(e, "md5");
		
		if (!_queueFile(url, file, hash))
			return false;
	}
	
	return true;
}

bool CollectionMap::_loadScript(XmlFile* xf, TiXmlElement* e) //<script id="something" file="script.lua" md5="hash" active="0" />
{
	//If this isn't an active script, don't create a LuaManager
	if (xf->GetParamInt(e, "active") == 0)
		return true;
	
	LuaManager* l = new LuaManager;
	if (l->ReadXml(xf, e, !mOfflineMode) == XMLPARSE_CANCEL)
	{
		SetLoadError( "Script Load Error: " + xf->GetParamString(e, "id") + ": " + l->GetError() );
		delete l;
		return false;
	}
	
	l->OnLoad();

	mScripts.push_back(l);

	return true;
}

bool CollectionMap::_readLayoutItem(XmlFile* xf, TiXmlElement* e)
{
	string id = e->Value();

	PRINT("CollectionMap::_readLayoutItem " + id);

	if (id == "object") //<object class="x" name="x" position="x,y" rotation="1.5" scale="1.75" hue="r,g,b" />
	{
		StaticObject* so = (StaticObject*)AddEntityFromResources( 	xf->GetParamString(e, "class"), 
																	xf->GetParamPoint2d(e, "position") 
																);
		if (!so)
		{
			SetLoadError(xf->GetParamString(e, "class") + " does not exist");
			return false;
		}
		
		AddEntity(so, so->mLayer);

		so->mName = xf->GetParamString(e, "name");
		so->mLocked = xf->GetParamInt(e, "lock");
		//so->mWarpDestination = xf->GetParamString(e, "destination");
		so->SetAA(xf->GetParamInt(e, "aa"));
		double rot = xf->GetParamDouble(e, "rotation");
		double scale = xf->GetParamDouble(e, "scale");
		if (scale == 0.0) //in case the tag was never set
			scale = 1.0;
			
		//only do the work if we need to
		if (rot != 0.0 || scale != 1.0)
			so->Rotozoom( rot, scale );
	}
	else if (id == "actor") //<actor class="mId" name="mName" position="x,y" />
	{
		SceneActor* sa = (SceneActor*)AddEntityFromResources( 	xf->GetParamString(e, "class"), 
																xf->GetParamPoint2d(e, "position") 
															);
		if (!sa)
		{
			SetLoadError(xf->GetParamString(e, "class") + " does not exist");
			return false;
		}
		
		AddEntity(sa, sa->mLayer);
		
		sa->mName = xf->GetParamString(e, "name");
	}
	else if (id == "text") //<text layer="1" size="12" position="x,y" rotation="0.0">Text Here</text>
	{
		TextObject* to = new TextObject();
		to->mMap = this;
		AddEntity(to, xf->GetParamInt(e, "layer"));
		to->SetText(xf->GetText(e),
					xf->GetParamInt(e, "size"),
					xf->GetParamDouble(e, "rotation"));	
		to->SetPosition(xf->GetParamPoint2d(e, "position"));
	}
	else if (id == "size") //<size w="#" h="#"/>
	{
		mWidth = xf->GetParamInt(e, "w");
		mHeight = xf->GetParamInt(e, "h");
	}
	else if (id == "spawn") //<spawn position="x,y"/>
	{
		mSpawnPoint = xf->GetParamPoint2d(e, "position");
	}

	PRINT("CollectionMap::_readLayoutItem end");
	
	return true;
}
bool CollectionMap::LoadProperties()
{
	if (!mXml)
		return false;
		
	//grab bounding map element
	TiXmlElement* e;
	TiXmlElement* e2;

	e = mXml->mDoc.FirstChildElement("map");
	if (e)
		e = e->FirstChildElement("properties");

	if (!e)
		return false;

	e2 = e->FirstChildElement("channel");
	if (e2)
	{
		mChannelId = mXml->GetText(e2);	
		DEBUGOUT("SET CHANNEL TO: " + mChannelId);
	}
	else
	{
		mChannelId.clear();
	}
	
	e2 = e->FirstChildElement("global");
	if (e2)
	{
		if (mXml->ParamExists(e2, "gravity"))
			SetGravity(mXml->GetParamInt(e2, "gravity"));

		if (mXml->ParamExists(e2, "background"))
			mBackground = mXml->GetParamColor(e2, "background");
	}
	
	return true;
}

bool CollectionMap::QueueResources()
{
	//In offline mode, we assume we have all the necessary files.
	if (mOfflineMode) 
		return true;

	ASSERT(mXml);
	
//Queue up everything needed for <resources>
	TiXmlElement* e = mXml->mDoc.FirstChildElement("map");
	if (e) e = e->FirstChildElement("resources");
	if (!e) 
	{
		SetLoadError("<resources> not found");
		return false;
	}
	
	//do _queueResource for every resource listed 
	e = e->FirstChildElement();
	while (e)
	{
		if (!_queueResource(mXml, e))
			return false;

		e = e->NextSiblingElement();
	}
	
//Also do the same for <scripts>! Except, it's not a fatal problem if we don't have any scripts!
	e = mXml->mDoc.FirstChildElement("map");
	if (e) e = e->FirstChildElement("scripts");
	if (!e)
	{
		//SetLoadError("<scripts> not found");
		return true;
	}
	
	e = e->FirstChildElement();
	while (e)
	{
		if (!_queueResource(mXml, e))
			return false;

		e = e->NextSiblingElement();
	}
	
	return true;
}

bool CollectionMap::LoadScripts()
{
	ASSERT(mXml);

	TiXmlElement* e = mXml->mDoc.FirstChildElement("map");
	if (e) e = e->FirstChildElement("scripts");
	if (!e) 
	{
		SetLoadError("<scripts> not found");
		return false;
	}
	
	//do _loadScript for every script listed 
	e = e->FirstChildElement();
	while (e)
	{
		if (!_loadScript(mXml, e))
			return false;

		e = e->NextSiblingElement();
	}
	return true;
}

bool CollectionMap::LoadLayout()
{
	ASSERT(mXml);
	PRINT("CollectionMap::LoadLayout");
	
	TiXmlElement* e;

	e = mXml->mDoc.FirstChildElement("map");
	if (e)
		e = e->FirstChildElement("layout");

	if (!e) 
	{
		SetLoadError("<layout> not found");
		return false;
	}
	
	//do _readLayoutItem for every resource listed
	e = e->FirstChildElement();
	while (e)
	{
		if (!_readLayoutItem(mXml, e))
		{
			return false;
		}
		e = e->NextSiblingElement();
	}
	
	PRINT("CollectionMap::LoadLayout end");
	return true;
}

Entity* CollectionMap::_loadEntityResource(XmlFile* xf, TiXmlElement* e)
{
	string type = e->Value();
	Entity* ent = NULL;
	
	//create entity based on root element
	if (type == "object")
	{
		ent = new StaticObject();
	}
	else if (type == "actor")
	{
		ent = new SceneActor();
	}
	else
	{
		FATAL("Unknown entity type: " + type);	
	}

	if (ent)
	{
		ent->mId = xf->GetParamString(e, "id");
		ent->mMap = this;
		
		//pass all the children to the readers
		e = e->FirstChildElement();
		while (e)
		{
			if (ent->ReadXml(xf, e, !mOfflineMode) == XMLPARSE_CANCEL)
			{
				FATAL("Issue while loading: " + ent->mId);
				delete ent;
				return NULL;
			}
			e = e->NextSiblingElement(); //go to next element
		}
	}

	return ent;
}

Entity* CollectionMap::AddEntityFromResources(string id, point2d pos)
{
	TiXmlElement* e;
	Entity* ent = NULL;
	
	ASSERT(mXml);
	PRINT("CollectionMap::AddEntityFromResources");

	if (mOfflineMode) //Load from editor/entities/ID.xml
	{
		string file = DIR_EDITOR;
		file += DIR_ENTITIES + id + ".xml";
		
		XmlFile* xf = new XmlFile();
		if (!xf->LoadFromFile(file))
		{
			string err = mXml->GetError();
			delete xf;
			FATAL(err);
		}
		
		e = xf->mDoc.FirstChildElement();
		
		game->mChat->AddMessage("Adding Entity " + id);
		ent = _loadEntityResource(xf, e);
		if (ent)
			ent->SetPosition(pos);

		return ent;
	}
	else //load from our <resources> list
	{
		e = mXml->mDoc.FirstChildElement("map");
		if (e)
			e = e->FirstChildElement("resources");
	
		if (!e) 
		{
			FATAL("<resources> not found");
		}

		//find our matching entity
		e = e->FirstChildElement();
		while (e)
		{
			if (mXml->GetParamString(e, "id") == id)
			{
				game->mChat->AddMessage("Adding Entity " + id);
				ent = _loadEntityResource(mXml, e);
				if (ent)
				{
					ent->SetPosition(pos);
					return ent;
				}
			}
			e = e->NextSiblingElement();
		}
		
		//never found
		return NULL;
	}
}
