
#include "EditorMap.h"
#include "../entity/StaticObject.h"
#include "../entity/SceneActor.h"
#include "../core/widgets/Console.h"
#include "../core/io/FileIO.h"
#include "../game/GameManager.h"

int callback_EditorMapParseElement(XmlFile* xf, TiXmlElement* e, void* userData)
{
	EditorMap* m = (EditorMap*)userData;
	
	if (!m)
		return XMLPARSE_CANCEL;

	return m->ParseXmlElement(xf, e);	
}

EditorMap::EditorMap()
{
	mHeldObject = NULL;
	mGridImage = NULL;
	mCanMoveObjects = false;
	mShowGrid = false;
	mEditSpawn = false;
	mSnapToGrid = false;
	mFont = fonts->Get();
	mAmountOfObjectInfo = 2;
	mLockIcon = resman->LoadImg("assets/me_lockicon.png");
	mSpawnFlagIcon = resman->LoadImg("assets/me_spawnflag.png");
	mRotationStep = 5.0;
	mScaleStep = 0.05;
}

EditorMap::~EditorMap()
{
	resman->Unload(mLockIcon);
	resman->Unload(mGridImage);
	resman->Unload(mSpawnFlagIcon);
}

void EditorMap::Process(uLong ms)
{
	Map::Process(ms);
}

void EditorMap::Event(SDL_Event* event)
{
	if (!HasKeyFocus())
		return;
		
	switch (event->type)
	{
		case SDL_KEYUP: 
		{
			switch (event->key.keysym.sym)
			{
				case SDLK_m: mCanMoveObjects = !mCanMoveObjects; break;
				case SDLK_DELETE: _deleteHeldObject(); break;
				case SDLK_c: _cloneHeldObject(); break;
				case SDLK_i: mAmountOfObjectInfo++; if (mAmountOfObjectInfo > 2) mAmountOfObjectInfo = 0; break;
				case SDLK_l: _lockHeldObject(); break;
				case SDLK_g: mShowGrid = !mShowGrid; break;
				case SDLK_s: mEditSpawn = !mEditSpawn; break;
				default: break;	
			}
		} break;
		case SDL_KEYDOWN:
		{
			switch (event->key.keysym.sym)
			{
				case SDLK_DOWN: OffsetCamera(0, CAMERA_OFFSET_SPEED); break;
				case SDLK_UP: OffsetCamera(0, -CAMERA_OFFSET_SPEED); break;
				case SDLK_LEFT: OffsetCamera(-CAMERA_OFFSET_SPEED, 0); break;
				case SDLK_RIGHT: OffsetCamera(CAMERA_OFFSET_SPEED, 0); break;
				case SDLK_n: mSnapToGrid = !mSnapToGrid; break;
				default: break;	
			}
			
			//StaticObject specific controls
			if (mHeldObject && mHeldObject->mType == ENTITY_STATICOBJECT) 
			{
				StaticObject* so = (StaticObject*)mHeldObject;
				switch (event->key.keysym.sym)
				{
					case SDLK_a: //toggle AA on held object
						so->SetAA( !so->mUseAA );
						break;
					case SDLK_j: //rotate left
						so->Rotozoom(so->mRotation + mRotationStep, so->mScale);
						break;
					case SDLK_k: //rotate right
						so->Rotozoom(so->mRotation - mRotationStep, so->mScale);
						break;
					case SDLK_y: //zoom in
						so->Rotozoom(so->mRotation, so->mScale + mScaleStep);
						break;
					case SDLK_h: //zoom out
						so->Rotozoom(so->mRotation, so->mScale - mScaleStep);
						break;
					default: break;	
				}
			}
		} break;
		case SDL_MOUSEBUTTONUP: 
		{
			if (event->button.button == SDL_BUTTON_LEFT)
			{
				if (mEditSpawn)
				{
					mEditSpawn = false;
				}
				else if (CanMoveHeldObject() && HasMouseFocus())
				{
					if (mHeldObject) //drop it
					{
						mHeldObject = NULL;
					}
					else //if there's an object under the mouse, grab it
					{
						mHeldObject = GetObjectUnderMouse();
						_calculateHeldOffset();
					}
				}
				else
				{
					mHeldObject = GetObjectUnderMouse();
					_calculateHeldOffset();
				}
			}
		} break;
		case SDL_MOUSEMOTION: 
		{
			if (mEditSpawn)
				_adjustSpawn();
			else if (CanMoveHeldObject() && HasMouseFocus() && mHeldObject)
				_adjustHeldObject();
		} break;
		default: break;
	}
}

void EditorMap::Render()
{
	Process(ms); //Here until we have a timer for it

	Image* scr = Screen::Instance();
	rect r = GetScreenPosition();
	rect rr;
	
	scr->SetClip(r); //should this be here, or in Map::PreRender(), Map::PostRender()

	//fill our map rect
	scr->DrawRect(r, mBackground);
	
	_renderEntities(ms);
	
	if (mShowGrid)
		RenderGrid(r);
	
	if (mSpawnFlagIcon)
	{
		rr = ToScreenPosition( rect(mSpawnPoint.x, mSpawnPoint.y, 16, 16) );
		//rr.x -= 8;
		//rr.y -= 16;
		mSpawnFlagIcon->Render( scr, 
								rr.x - mSpawnFlagIcon->Width() / 2, 
								rr.y - mSpawnFlagIcon->Height() 
							);
	}
	
	string msg;
	msg = its(GetCameraPosition().x) + "," + its(GetCameraPosition().y);

	//draw top left coordinate numbers
	mFont->Render(scr, r.x, r.y, msg, color(255,255,255));
	r.x += mFont->GetWidth(msg) + 5;

	if (mCanMoveObjects)
	{
		msg = "Move Enabled";
		mFont->Render(scr, r.x, r.y, msg, color(255,0,0));
		r.x += mFont->GetWidth(msg) + 5;
	}

	if (mHeldObject)
	{
		msg = "Holding " + mHeldObject->mId;
		mFont->Render(scr, r.x, r.y, msg, color(0,255,0));
		r.x += mFont->GetWidth(msg) + 5;
	}

	r = GetScreenPosition();
	r.w = 640;
	r.h = 480;
	//Render a bounding rect the size of our default ingame screen
	scr->DrawRect(r, color(255,0,255), false);
	
	scr->SetClip();
		
	Map::Render();
}

void EditorMap::RenderGrid(rect r)
{
	Image* scr = Screen::Instance();

	// Create a cached copy of the grid for less cpu intensive rendering
	if (!mGridImage)
	{
		color c(80,80,80);
		
		mGridImage = resman->NewImage(32*16, 32*16, color(), false);
		
		rect rr(0,0,16,16);
		for (rr.y = 0; rr.y < mGridImage->Height(); rr.y+=16)
		{
			for (rr.x = 0; rr.x < mGridImage->Width(); rr.x+=16)
			{
				mGridImage->DrawRect( rr, c, false );
			}
		}
	}
	
	//Render our grid image to the editor, repeated to fill
	mGridImage->RenderPattern(scr, rect(0, 0, mGridImage->Width(), mGridImage->Height()), r);
}

void EditorMap::_renderEntities(uLong ms)
{
	//for all VISIBLE objects, in order of their layer and y position (like ingame)
	ResortEntities();
	
	Image* scr = Screen::Instance();

	uShort i, ii, u;
	rect r, rr;
	string msg;
	color c;
	Entity* e;
	for (i = 0; i < ENTITYLEVEL_COUNT; i++) //Iterate all lists
	{
		for (ii = 0; ii < entityLevel[i].size(); ii++)
		{
			e = entityLevel[i].at(ii);
			
			if (e->IsVisible() && IsRectInCamera(e->GetBoundingRect()))
			{
				e->Render(ms); //draw entity
				
				r = ToScreenPosition( e->GetBoundingRect() );
				
				if (mHeldObject == e)
					c = color(0,255,0);
				else
					c = color(0,0,255);
				
				//EXTRA INFORMATION
				
				if (mAmountOfObjectInfo > 0)
				{
					scr->DrawRect(r, c, false); //draw bounding rect
		
					//draw collision rects
					for (uShort u = 0; u < e->mCollisionRects.size(); u++)
					{
						rr.x = e->mCollisionRects.at(u).x + e->mPosition.x - e->mOrigin.x;
						rr.y = e->mCollisionRects.at(u).y + e->mPosition.y - e->mOrigin.y;
						rr.w = e->mCollisionRects.at(u).w;
						rr.h = e->mCollisionRects.at(u).h;
						scr->DrawRect( ToScreenPosition(rr), color(255,0,0), false );
					}
				}
				
				if (mAmountOfObjectInfo > 1)
				{
					//Draw stats on this entity
					msg = its(e->mPosition.x) + "," + its(e->mPosition.y) + " L" + its(i);
					
					r.w = mFont->GetWidth(msg);
					r.h = mFont->GetHeight();
					r.y -= r.h;
					scr->DrawRect(r, color(0,0,0));
					mFont->Render(scr, r.x, r.y, msg, color(255,255,255));
					
					r = ToScreenPosition( e->GetBoundingRect() );	
						
					//draw circle around origin point
					r.x = r.x + e->mOrigin.x - 2;
					r.y = r.y + e->mOrigin.y - 2;
					r.w = 5;
					r.h = 5;
					scr->DrawRound(r, 2, color(0,0,255));
									
					r = ToScreenPosition( e->GetBoundingRect() );
	
					//if this object is position locked, draw a lock symbol around it
					if (e->mLocked && mLockIcon)
					{
						mLockIcon->Render( scr, r.x + (r.w / 2 - mLockIcon->Width() /  2), 
												r.y + (r.h / 2 - mLockIcon->Height() / 2) );	
					}
				} //if show object info
			} //if visible & in camera
		} //for all entities in layer
	} //for each layer
}

void EditorMap::Clone()
{
	Entity* clone;
	if (mHeldObject)
	{
		//clone = mHeldObject->Clone();
		//TODO: add clone to entity manager
		mHeldObject = clone; //will drop currently held where it is, and keep a hold on the clone
	}
}

Entity* EditorMap::GetObjectUnderMouse()
{
	if (!HasMouseFocus())
		return NULL;
		
	rect r(gui->GetMouseX(), gui->GetMouseY(), 1, 1);
	r = ToCameraPosition(r);
	
	sShort i, ii;
	Entity* e;
	for (i = ENTITYLEVEL_COUNT-1; i > -1; i--) //Iterate all lists from render top to bottom
	{
		for (ii = entityLevel[i].size()-1; ii > -1; ii--)
		{
			e = entityLevel[i].at(ii);
			
			if ( e->IsVisible() && IsRectInCamera(e->GetBoundingRect()) 
				&& areRectsIntersecting(e->GetBoundingRect(), r) && mHeldObject != e )
			{
				return e;
			}
		}
	}
	return NULL;
}

bool EditorMap::CanMoveHeldObject()
{
	return mCanMoveObjects && mHeldObject && !mHeldObject->mLocked;
}

void EditorMap::_adjustHeldObject()
{
	rect r = ToCameraPosition( gui->GetMouseRect() );
	
	point2d p;
	
	
	if (mSnapToGrid)
	{
		p.x = (r.x - mHeldOffset.x) / 16;
		p.x *= 16;
		//p.x += 8; //small adjustment to get it to center on the tile
		p.y = (r.y - mHeldOffset.y) / 16;
		p.y *= 16;	
	}
	else
	{
		p.x = r.x - mHeldOffset.x;
		p.y = r.y - mHeldOffset.y;
	}
	
	mHeldObject->SetPosition(p);
}

void EditorMap::_adjustSpawn()
{
	rect r = ToCameraPosition( gui->GetMouseRect() );
	
	//Force spawn point on a 16x16 grid
	mSpawnPoint.x = r.x / 16;
	mSpawnPoint.x *= 16;
	mSpawnPoint.x += 8; //small adjustment to get it to center on the tile
	mSpawnPoint.y = r.y / 16;
	mSpawnPoint.y *= 16;
}

void EditorMap::_calculateHeldOffset()
{
	if (!mHeldObject)
		return;
		
	rect r = ToCameraPosition( gui->GetMouseRect() );
	
	mHeldOffset.x = r.x - mHeldObject->mPosition.x;
	mHeldOffset.y = r.y - mHeldObject->mPosition.y;
}

void EditorMap::_deleteHeldObject()
{
	if (mHeldObject && !mHeldObject->mLocked)
	{
		RemoveEntity(mHeldObject);
		mHeldObject = NULL;
	}
}

void EditorMap::_cloneHeldObject()
{
	if (mHeldObject)
	{
		AddUniqueEntity(mHeldObject->mId, mHeldObject->mPosition);
	}	
}

void EditorMap::_lockHeldObject()
{
	if (mHeldObject)
	{
		mHeldObject->mLocked = !mHeldObject->mLocked;
	}	
}

Entity* EditorMap::AddEntityFromResources(string id, point2d pos)
{
/*	Entity* e = NULL;
	
	//search for existing unique entity, if it exists, return a copy of it.
	for (uShort i = 0; i < mUniqueEntities.size(); i++)
	{
		if (mUniqueEntities.at(i)->mId == id)
		{
			e = mUniqueEntities.at(i)->Clone();
			e->SetPosition(pos);
			AddEntity(e, e->GetLayer());
			return e;
		}
	}

	e = _loadUniqueEntity(id);
	if (e)
	{
		e = e->Clone();
		if (e)
		{
			e->SetPosition(pos);
			AddEntity(e, e->GetLayer());
		}
	}

	return e;*/
	FATAL("TODO: EditorMap::AddEntityFromResources");
}

Entity* EditorMap::_loadUniqueEntity(string id)
{
	XmlFile xf;

	string file = string(DIR_EDITOR) + DIR_ENTITIES + id + ".xml";

	if (!xf.LoadFromFile( file ))
		return NULL;
	
	TiXmlElement* e = xf.mDoc.FirstChildElement();
	
	string type = e->Value();
	Entity* ent = NULL;
	
	//create entity based on root element
	if (type == "object")
	{
		ent = new StaticObject();
	}
	else if (type == "actor")
	{
		ent = new SceneActor;	
	}
	
	if (ent)
	{
		/*if (id != xf.GetParamString(e, "id"))
		{
			console->AddMessage("Internal ID does not match " + id);
			delete ent;
			return NULL;
		}
		*/
		ent->mMap = this;
		ent->mId = xf.GetParamString(e, "id");

		//pass all the children to the readers
		e = e->FirstChildElement();
		while (e)
		{
			if (ent->ReadXml(&xf, e, false) == XMLPARSE_CANCEL)
			{
				console->AddMessage("Issue while loading: " + ent->mId);
				delete ent;
				return NULL;
			}
			e = e->NextSiblingElement(); //go to next element
		}
		
		mUniqueEntities.push_back(ent);	
	}
	else
	{
		console->AddMessage(id + " is unknown entity type <" + type + ">");	
	}
	
	return ent;
}

bool EditorMap::LoadScript(string id)
{
	XmlFile xf;

	string file = string(DIR_EDITOR) + DIR_SCRIPTS + id + ".xml";

	if (!xf.LoadFromFile( file ))
	{
		console->AddMessage(file + " Xml Load Error: " + xf.GetError());
		return false;
	}
	
	for (uShort i = 0; i < mScriptList.size(); i++)
	{
		if (mScriptList.at(i) == id)
		{
			console->AddMessage("Script " + id + " already loaded");
			return false;
		}
	}
	
	TiXmlElement* e = xf.mDoc.FirstChildElement();
	
	string type = e->Value();

	//create entity based on root element
	if (type != "script")
	{
		console->AddMessage("Invalid <script> " + id);
		return false;
	}
	
	/*if (id != xf.GetParamString(e, "id"))
	{
		console->AddMessage("Internal ID does not match " + id);
		return false;
	}*/
	
	mScriptList.push_back(xf.GetParamString(e, "id"));	
	return true;
}

/*	Read DIR_MAPS/id.res into array, exploded via \n
	For each item, _loadUniqueEntity(id)
	return false if the resource list is not found, or malformed
	id is provided as a parameter so that we can load multiple resource files in. 
*/
bool EditorMap::ReadResourceList(string id)
{
	string file = string(DIR_EDITOR) + DIR_MAPS + id + ".res";
	vString v;

	if (!fileTovString(v, file, "\n")) //vString&, delimiter
	{
		console->AddMessage("Resource file not found: " + file);
		return false;
	}
	
	console->AddMessage("Total Resources: " + its(v.size()));
	
	Entity* e;
	for (uShort i = 0; i < v.size(); i++)
	{
		if (!v.at(i).empty())
		{
			e = _loadUniqueEntity(v.at(i));
			if (!e)
			{
				//Not an entity, try to load as a script
				if (!LoadScript(v.at(i)))
				{
					console->AddMessage("Failed to load object: " + v.at(i));
					return false;
				}
				else
				{
					console->AddMessage("Loaded Script: " + v.at(i));
				}
			}
			else
			{
				console->AddMessage("Loaded Entity: " + v.at(i));
			}
		}
	}
	
	return true;
}

int EditorMap::ParseXmlElement(XmlFile* xf, TiXmlElement* e)
{
	string id = e->Value();

	if (id == "layout")
	{
		return xf->ParseSiblingsOnly(e->FirstChildElement(), this);
	}
	else if (id == "object") //<object class="x" name="x" position="x,y" rotation="1.5" scale="1.75" hue="r,g,b" />
	{
		StaticObject* so = (StaticObject*)AddUniqueEntity( xf->GetParamString(e, "class"), xf->GetParamPoint2d(e, "position") );
		if (so)
		{
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
	}
	else if (id == "actor") //<actor class="mId" name="mName" position="x,y" />
	{
		SceneActor* a = (SceneActor*)AddUniqueEntity( xf->GetParamString(e, "class"), xf->GetParamPoint2d(e, "position") );
		if (a)
		{
			a->mName = xf->GetParamString(e, "name");
		}
	}
	else if (id == "size") //<size w="#" h="#"/>
	{
		//This is pretty much ignored for the map editor, so don't set it so the camera can move without bounds.
		//mWidth = mXml->GetParamInt(e, "w");
		//mHeight = mXml->GetParamInt(e, "h");
	}
	else if (id == "spawn") //<spawn position="x,y"/>
	{
		mSpawnPoint = mXml->GetParamPoint2d(e, "position");
	}
	
	return XMLPARSE_SUCCESS;
}

bool EditorMap::LoadXml()
{
	XmlFile xf;
	xf.SetParser(callback_EditorMapParseElement);

	if (!xf.LoadFromFile( string(DIR_EDITOR) + DIR_MAPS + mId + ".xml" ))
	{
		console->AddMessage(xf.GetError());
		return false;
	}
	
	return (xf.ParseSiblingsOnly(xf.mDoc.FirstChildElement(), this) != XMLPARSE_CANCEL);
}

/*	Loads resource list and layout */
bool EditorMap::Load(string id)
{
	FlushEntities();
	mScriptList.clear();
	
	mId = id;
	
	if (!ReadResourceList(id))
		return false;
	
	console->AddMessage("Unique Entities: " + its(mUniqueEntities.size()));
	
	if (!LoadXml())
		return false;

	return true;
}

/*	for each unique entity, output id\n */
bool EditorMap::SaveResourceList()
{
	string file = string(DIR_EDITOR) + DIR_MAPS + mId + ".res";
	uShort i;
	
	FILE* f = fopen(file.c_str(), "w");
	if (f)
	{
		for (i = 0; i < mUniqueEntities.size(); i++)
		{
			//only save the entities that have actually been used in the map
			if ( FindEntityById(mUniqueEntities.at(i)->mId) )
				fprintf(f, "%s\n", mUniqueEntities.at(i)->mId.c_str());
		}
		
		//Write loaded scripts
		for (i = 0; i < mScriptList.size(); i++)
		{
			fprintf(f, "%s\n", mScriptList.at(i).c_str());
		}

		fclose(f);
		return true;
	}
	
	return false;
}

/*	write out <layout>, for each entity, write out associated
	xml. (Type, properties, etc)
*/
bool EditorMap::SaveXml()
{
	uShort i, ii;
	TiXmlElement* root;
	TiXmlElement* e;
	Entity* ent;
	XmlFile xf;

	root = new TiXmlElement("map");
	xf.mDoc.LinkEndChild(root);

	// Add <resources>RESMARKER</resources> to have an insertion point for master.php
	// to work with, Without resorting to real xml parsing
	e = new TiXmlElement("resources");
	xf.SetText(e, "RESMARKER");
	root->LinkEndChild(e);
	
	e = new TiXmlElement("layout");
	root->LinkEndChild(e);
	
	root = e; //working with just layout 

	//print dimensions and spawn point
	
	//<size w="#" h="#"/>
	_calculateDimensions();
	e = xf.AddChildElement(root, "size");
	xf.SetParamInt(e, "w", mWidth);
	xf.SetParamInt(e, "h", mHeight);

	//<spawn position="x,y"/>
	e = xf.AddChildElement(root, "spawn");
	xf.SetParamPoint2d(e, "position", mSpawnPoint);

	//Print out all entities
	for (i = 0; i < ENTITYLEVEL_COUNT; i++)
	{
		for (ii = 0; ii < entityLevel[i].size(); ii++)
		{
			ent = entityLevel[i].at(ii);

			//TODO: Really REALLY should be a function in entity...
			switch (ent->mType)
			{
				// <object class="x" name="x" layer="0" position="x,y" lock="1" rotation="1.5" scale="1.75" hue="r,g,b" />
				case ENTITY_STATICOBJECT: 
				{
					StaticObject* so = (StaticObject*)ent;
					
					e = xf.AddChildElement(root, "object");
					xf.SetParamString(e, "class", so->mId);
					xf.SetParamString(e, "name", so->mName);
					xf.SetParamPoint2d(e, "position", so->mPosition);
					
					if (so->mRotation != 0.0)
						xf.SetParamDouble(e, "rotation", so->mRotation);
					
					if (so->mScale != 1.0)
						xf.SetParamDouble(e, "scale", so->mScale);
					
					if (so->mUseAA)
						xf.SetParamInt(e, "aa", 1);
					
					if (so->mLocked)
						xf.SetParamInt(e, "lock", so->mLocked);
					
					//if (!so->mWarpDestination.empty())
				//		xf.SetParamString(e, "destination", so->mWarpDestination);
						
					//xf.SetParamDouble(e, "rotation", ent->mRotation);
					//xf.SetParamDouble(e, "scale", ent->mScale);
					//xf.SetParamColor(e, "hue", ent->mHue);
				} break;
				case ENTITY_SCENEACTOR:
				{
					SceneActor* sa = (SceneActor*)ent;
					
					e = xf.AddChildElement(root, "actor");
					
					xf.SetParamString(e, "class", sa->mId);
					xf.SetParamString(e, "name", sa->mName);
					xf.SetParamPoint2d(e, "position", sa->mPosition);
				} break;
				default: break;
			}
		}
	}

	string file = string(DIR_EDITOR) + DIR_MAPS + mId + ".xml";
	
	xf.SaveToFile(file);
	return true;
}

bool EditorMap::Save()
{
	return ( SaveResourceList() && SaveXml() );
}

/*	Runs through every DIR_ENTITIES/*.xml and loads into the unique objects list. 
	Used for when we want easy access to every entity available to us.
*/
bool EditorMap::LoadAllResources()
{
	string searchPattern = string(DIR_EDITOR) + DIR_ENTITIES + string("*.xml");
	vString files;
	if (!getFilesMatchingPattern(files, searchPattern))
		return false;
	
	size_t pos;
	uShort i, ii;
	bool found;
	for (i = 0; i < files.size(); i++)
	{
		files.at(i).erase(files.at(i).length() - 4, 4); //erase .xml
			
		if (!files.at(i).empty())
		{
			found = false;
			//make sure it doesn't exist already
			for (ii = 0; ii < mUniqueEntities.size(); ii++)
			{
				if (mUniqueEntities.at(ii)->mId == files.at(i))
				{	
					found = true;
					break;
				}
			}
			if (!found)
				_loadUniqueEntity(files.at(i));
		}
	}
}

void EditorMap::_calculateDimensions()
{
	uShort i, ii;
	Entity* ent;
	rect r;
	mWidth = mHeight = 0;
	
	for (i = 0; i < ENTITYLEVEL_COUNT; i++)
	{
		for (ii = 0; ii < entityLevel[i].size(); ii++)
		{
			ent = entityLevel[i].at(ii);
			
			r = ent->GetBoundingRect();
			if (r.x + r.w > mWidth)
				mWidth = r.x + r.w;
			if (r.y + r.h > mHeight)
				mHeight = r.y + r.h;
		}
	}
}




