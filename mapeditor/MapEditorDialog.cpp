
#include "MapEditorDialog.h"
#include "EditorMap.h"
#include "../core/widgets/Console.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Input.h"

const int LISTWIDTH = 150;

void callback_MEMultilineDblLeft(Multiline* m)
{
	((MapEditorDialog*)m->GetParent())->CopySelectedObjectInList();
}

void callback_MEAdd(Console* c, string s)
{
	MapEditorDialog* m = (MapEditorDialog*)gui->Get("MapEditor");
	ASSERT(m);

	vString v;
	explode(&v, &s, " ");
	
	rect r = m->mMap->ToCameraPosition( gui->GetMouseRect() );
	Entity* e;
	
	if (v.size() > 1)
	{
		e = m->mMap->AddEntityFromResources(v.at(1), point2d(r.x, r.y));
		m->UpdateUniqueObjectsList();
		if (!e)
			c->AddMessage("Failed to load Object: " + v.at(1));
		else
			c->AddMessage("Loaded Object: " + v.at(1));
	}
	else
	{
		c->AddMessage("Invalid. me_add <ObjectId>");
	}
}

void callback_MEAddScript(Console* c, string s)
{
	MapEditorDialog* m = (MapEditorDialog*)gui->Get("MapEditor");
	ASSERT(m);

	vString v;
	explode(&v, &s, " ");

	if (v.size() > 1)
	{
		if ( !m->mMap->LoadScript(v.at(1)) )
			c->AddMessage("Failed to load Script: " + v.at(1));
		else
			c->AddMessage("Loaded Script: " + v.at(1));
	}
	else
	{
		c->AddMessage("Invalid. me_addscript <ScriptId>");
	}
}

void callback_MERemoveScript(Console* c, string s)
{
	MapEditorDialog* m = (MapEditorDialog*)gui->Get("MapEditor");
	ASSERT(m);

	vString v;
	explode(&v, &s, " ");

	if (v.size() > 1)
	{
		for (uShort i = 0; i < m->mMap->mScriptList.size(); i++)
		{
			if (m->mMap->mScriptList.at(i) == v.at(1))
			{
				m->mMap->mScriptList.erase(m->mMap->mScriptList.begin() + i);
				c->AddMessage("Unloaded Script: " + v.at(1));
				break;
			}
		}
	}
	else
	{
		c->AddMessage("Invalid. me_removescript <ScriptId>");
	}
}

void callback_MELoad(Console* c, string s)
{
	MapEditorDialog* m = (MapEditorDialog*)gui->Get("MapEditor");
	
	if (!m)
	{
		m = new MapEditorDialog();
		gui->Add(m);	
	}

	vString v;
	explode(&v, &s, " ");
	
	if (v.size() > 1)
	{
		if (!m->mMap->Load(v.at(1)))
			c->AddMessage("A blank map has been created");
		else
			c->AddMessage("Loaded: " + v.at(1));
		m->SetCaption("MapEditor - " + v.at(1));
		m->UpdateUniqueObjectsList();
	}
	else
	{
		c->AddMessage("Invalid. me_load <MapId>");	
	}
}

void callback_MESave(Console* c, string s)
{
	MapEditorDialog* m = (MapEditorDialog*)gui->Get("MapEditor");
	ASSERT(m);
	if (m->mMap->Save())
		c->AddMessage("Map Saved: " + m->mMap->mId);
	else
		c->AddMessage("Failed to save: " + m->mMap->mId);
}

void callback_MEHelp(Console* c, string s)
{
	c->AddMessage("\\c990Keyboard Controls:");
	c->AddMessage(" * c -\\c777 Clone held object");
	c->AddMessage(" * i -\\c777 Toggle amount of object info");
	c->AddMessage(" * l -\\c777 Toggle object locking");
	c->AddMessage(" * m -\\c777 Toggle object move mode");
	c->AddMessage(" * s -\\c777 Toggle spawn move mode");
	c->AddMessage(" * n -\\c777 Toggle grid snapping mode");
	c->AddMessage(" * g -\\c777 Toggle grid");
	c->AddMessage(" * a -\\c777 Toggle AA on object");
	c->AddMessage(" * j -\\c777 Rotate object left");
	c->AddMessage(" * k -\\c777 Rotate object right");
	c->AddMessage(" * y -\\c777 Scale up object");
	c->AddMessage(" * h -\\c777 Scale down object");
	c->AddMessage(" * Page Up -\\c777 Raise held object's layer");
	c->AddMessage(" * Page Down -\\c777 Lower held object's layer");
	c->AddMessage(" * Delete -\\c777 Delete held object");
	c->AddMessage(" * Arrow Keys -\\c777 Move camera");
	c->AddMessage("\\c990Mouse Controls:");
	c->AddMessage(" * Left click an object to hold it or let go");
	c->AddMessage(" * If move is NOT enabled (Or held object is locked), left click again on object will select next under it");
}

void callback_MEListAllResources(Button* b)
{
	MapEditorDialog* m = (MapEditorDialog*)b->GetParent();
	if (m)
	{
		m->mMap->LoadAllResources();
		m->UpdateUniqueObjectsList();
	}
}

void callback_MESearchResources(Input* i)
{
	MapEditorDialog* m = (MapEditorDialog*)i->GetParent();
	if (m)
		m->UpdateUniqueObjectsList();
}

MapEditorDialog::MapEditorDialog()
	: Frame( gui, "MapEditor", rect(0,0,gui->Width(),gui->Height()) )
{
	//Center();
	mMap = NULL;
	
	gui->SetAppTitle("fro - Map Editor");

	//TODO: this is temp, to ensure we don't have multiple MEs running.
	ASSERT( gui->Get("MapEditor") == this );

	mMap = new EditorMap();
	mMap->mId = "map_" + timestamp(true);
	Add(mMap);
	
	mUniqueObjectsList = new Multiline(this, "", rect());
	
	//turn it into a list, rather than a multiline text box
	mUniqueObjectsList->mWrap = false;
	mUniqueObjectsList->mHighlightSelected = true;
	mUniqueObjectsList->mHighlightBackground = HIGHLIGHT_COLOR;
	mUniqueObjectsList->onLeftDoubleClickCallback = callback_MEMultilineDblLeft;

	mLoadAllResources = new Button(this, "loadall", rect(), "", callback_MEListAllResources);
		makeImage(mLoadAllResources, "", "assets/me_loadres.png", rect(0,0,20,20), 
					rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	mLoadAllResources->mHoverText = "Load all available resources";
	
	mSearch = new Input(this, "", rect(), "", 0, false, NULL);
	mSearch->onChangeCallback = callback_MESearchResources;
	mSearch->mHoverText = "Search for all matching objects (Wildcards acceptable)";

	ResizeChildren();
	
	//hook console commands
	console->HookCommand("me_load", callback_MELoad);
	console->HookCommand("me_save", callback_MESave);
	console->HookCommand("me_add", callback_MEAdd);
	console->HookCommand("me_addscript", callback_MEAddScript);
	console->HookCommand("me_removescript", callback_MERemoveScript);
	console->HookCommand("me_help", callback_MEHelp);
	
}

MapEditorDialog::~MapEditorDialog()
{
	console->UnhookCommand("me_load");
	console->UnhookCommand("me_save");
	console->UnhookCommand("me_add");
	console->UnhookCommand("me_addscript");
	console->UnhookCommand("me_removescript");
	console->UnhookCommand("me_help");
}

void MapEditorDialog::Render()
{
	Frame::Render();
}

void MapEditorDialog::Event(SDL_Event* event)
{
	switch (event->type)
	{
		case SDL_VIDEORESIZE:
			SetSize(gui->Width(), gui->Height());
			ResizeChildren();
			break;
		default: break;
	}
}

void MapEditorDialog::SetPosition(rect r)
{
	if (r.w > LISTWIDTH + 50 && r.h > 100)
		Frame::SetPosition(r);
}

void MapEditorDialog::ResizeChildren()
{
	if (mMap)
		mMap->SetPosition( rect(5, 5, Width() - 15 - LISTWIDTH, Height() - 10) );
			
	//Update button set
	if (mLoadAllResources)
		mLoadAllResources->SetPosition( rect(Width() - 5 - LISTWIDTH, 5, 20, 20) );
	
	if (mUniqueObjectsList)
		mUniqueObjectsList->SetPosition( rect(Width() - 5 - LISTWIDTH, 30, LISTWIDTH, Height() - 60) );

	if (mSearch)
		mSearch->SetPosition( rect(Width() - 5 - LISTWIDTH, Height() - 25, LISTWIDTH, 20) );
		
	Frame::ResizeChildren();
}

void MapEditorDialog::UpdateUniqueObjectsList()
{
	/*string search = mSearch->GetText();

	mUniqueObjectsList->Clear();
	for (uShort i = 0; i < mMap->mUniqueEntities.size(); i++)
	{
		string& id = mMap->mUniqueEntities.at(i)->mId;
		if ( search.empty() || (id.find(search, 0) == 0)
			|| wildmatch(search.c_str(), id.c_str()) )
		{
			mUniqueObjectsList->AddMessage(id);
		}
	}
	*/
	// Will need to search through all files in editor/, or something... ?
	FATAL("TODO: UpdateUniqueObjectsList");
}

void MapEditorDialog::CopySelectedObjectInList()
{
	string s = mUniqueObjectsList->GetSelectedText();
	rect r = mMap->ToCameraPosition( gui->GetMouseRect() );
	Entity* e;
	
	if (!s.empty())
	{
		e = mMap->AddEntityFromResources(s, point2d(r.x, r.y));
		if (e)
		{
			mMap->mHeldObject = e;
			
			//grab the middle of this entity so we can place it
			r = e->GetBoundingRect();
			mMap->mHeldOffset.x = r.w / 2 - e->mOrigin.x;
			mMap->mHeldOffset.y = r.h / 2 - e->mOrigin.y;
			
			mMap->SetKeyFocus();
			mMap->mCanMoveObjects = true;
		}
	}
}
