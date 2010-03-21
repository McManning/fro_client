
#include "Shortcuts.h"

#include "../core/widgets/Button.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/Input.h"

#include "../map/Map.h"
#include "../game/GameManager.h"

Shortcuts* shortcuts;

void callback_ShortcutsSave(Button* b)
{
	shortcuts->Save();
	shortcuts->SetVisible(false);
}

void callback_ShortcutsClose(Button* b) //override current close system
{
	b->GetParent()->SetVisible(false);
}

Shortcuts::Shortcuts() :
	Frame(gui, "shortcuts", rect(), "Edit Shortcuts", true, false, true, true)
{
	ASSERT(!shortcuts); //there can be only one
	
	uShort y = 30;

	Input* i;
	Button* b;
	Label* l;
	
	Load();
	
	rect r(40, 30, 170, 20);

	//Read values from mShortcut array into our inputs for editing
	for (uShort u = 0; u < 8; u++)
	{
		new Label(this, "", rect(10,r.y), "F" + its(u + 1));
		i = new Input(this, "f" + its(u + 1), r, "", 32, true, NULL);
			i->SetText(mShortcut[u]);
		r.y += 25;
	}

	b = new Button(this, "save",rect(r.w + r.x - 25,r.y,20,20), "", callback_ShortcutsSave);
		b->mHoverText = "Save";
		makeImage(b, "", "assets/button20.png", rect(60,0,20,20),
				rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	r.y += 25;
		
	if (mClose) //override current close method
		mClose->onClickCallback = callback_ShortcutsClose;
	
	shortcuts = this;

	SetSize(r.w + r.x + 5, r.y);
	Center();
	ResizeChildren();
}	

Shortcuts::~Shortcuts()
{
	shortcuts = NULL;
}

bool Shortcuts::Load()
{
	TiXmlElement* root;
	TiXmlElement* e;
	XmlFile xf;
	if (!xf.LoadFromFile(SHORTCUTS_FILENAME))
		return false;
		
	root = xf.mDoc.FirstChildElement("shortcuts");
	if (!root)
		return false;
		
	string id;
	for (uShort u = 0; u < 8; u++)
	{
		id = "f" + its(u+1);
		e = root->FirstChildElement(id.c_str());
		if (e)
			mShortcut[u] = xf.GetText(e);
		else
			mShortcut[u].clear();
	}
	
	return true;
}
	
bool Shortcuts::Save()
{
	TiXmlElement* root;
	TiXmlElement* e;
	XmlFile xf;

	root = new TiXmlElement("shortcuts");
	xf.mDoc.LinkEndChild(root);

	//Dump back into mShortcut array
	Input* i;
	for (int u = 0; u < 8; u++)
	{
		i = (Input*)Get("f" + its(u + 1));
		mShortcut[u] = i->GetText();
		
		e = xf.AddChildElement(root, "f" + its(u+1));
		xf.AddChildText( e, mShortcut[u] );
	}

	return xf.SaveToFile(SHORTCUTS_FILENAME);
}

void Shortcuts::Run(int num)
{
	if (!game || num < 0 || num > 7 || mShortcut[num].empty()) 
		return;

	timer* t = timers->Find("shortwait");
	if (t)
	{
		int seconds = (t->lastMs + t->interval - gui->GetTick()) / 1000;
		game->mChat->AddMessage("\\c900 * You must wait " + its(seconds) + " seconds.");
		return;
	}

	game->mChat->mInput->PushInput( mShortcut[num] );
	
	//limit the number of times they can use shortcuts
	timers->Add("shortwait", SHORTCUT_INTERVAL_MS, false, NULL, NULL, NULL);
}


