
#include "ObjectEditor.h"
#include "../core/widgets/Console.h"
#include "../core/io/FileIO.h"
#include "../game/GameManager.h"

string _getActorImage(string id)
{
	string file;
	file = (string)DIR_EDITOR + DIR_ENTITIES + id + ".*"; //wildcard extension
	file = getRandomFileMatchingPattern(file);

	return file;
}

//Unrelated function, but somewhat similar
void callback_CreateActor(Console* c, string s) //create_actor id w h
{
	vString v;
	explode(&v, &s, " ");
	if (v.size() < 4)
	{
		c->AddMessage("Invalid. create_actor <id> <w> <h>");
		return;	
	}
	
/*
	<actor id="test_actor">
	    <base shadow="1" solid="0" layer="1" /> 
		<avatar file="something.png" md5="hash" w="32" h="70" loopstand="1" loopsit="0" delay="1000" />
	</actor>
*/
	
	string imgfile = _getActorImage(v.at(1));
	if (imgfile.empty())
	{
		c->AddMessage((string)DIR_EDITOR + DIR_ENTITIES + v.at(1) + ".(jpg|png|mng|gif|bmp|img) not found.");
		return;	
	}

	string file = (string)DIR_EDITOR + DIR_ENTITIES + v.at(1) + ".xml";
	string hash = md5file((string)DIR_EDITOR + DIR_ENTITIES + imgfile);

	FILE* f = fopen(file.c_str(), "w");
	if (!f)
	{
		c->AddMessage("Could not open " + file);
		return;	
	}
	
	fprintf(f, 	"<actor id=\"%s\">\n"
				"    <base shadow=\"1\" solid=\"0\" layer=\"1\" />\n"
				"    <avatar file=\"%s\" md5=\"%s\" w=\"%i\" h=\"%i\" loopstand=\"1\" loopsit=\"0\" delay=\"1000\"/>\n"
				"</actor>\n", v.at(1).c_str(), imgfile.c_str(), hash.c_str(), sti(v.at(2)), sti(v.at(3))
			);
	fclose(f);

	c->AddMessage(file + " saved."); 
	
}

int callback_ObjectEditorXml(XmlFile* xf, TiXmlElement* e, void* userData)
{
	ObjectEditor* o = (ObjectEditor*)userData;
	
	if (!o)
		return XMLPARSE_CANCEL;
	else
		return o->ParseXml(xf, e);	
}

//TODO: Somehow.. not.. this technique. Given that we can't run multiple OE's at once. 
void callback_ObjectEditorConsoleInput(Console* c, string s)
{
	ObjectEditor* o = (ObjectEditor*)(gui->Get("ObjectEditor"));
	if (o)
		o->OnConsoleInput(c, s);
}

void callback_OELoad(Console* c, string s)
{
	ObjectEditor* o = (ObjectEditor*)(gui->Get("ObjectEditor"));
	if (!o)
	{
		o = new ObjectEditor();
		gui->Add(o);	
	}
	o->OnConsoleInput(c, s);
}

ObjectEditor::ObjectEditor()
	: Frame( gui, "ObjectEditor", rect(0,0,400,300), "Object Editor", true, true, true, true )
{
	Center();

	//TODO: this is temp, to ensure we don't have multiple OEs running.
	ASSERT( gui->Get("ObjectEditor") == this );

	mImage = NULL;
	mGridImage = NULL;
	mEditingRects = false;
	mEditingOrigin = false;
	mShowGrid = false;
	mSolid = false;
	mShadow = false;
	mDelay = 0;
	
	mCamera.x = 0;
	mCamera.y = 0;

	console->HookCommand("oe_load", callback_OELoad);
	console->HookCommand("oe_save", callback_ObjectEditorConsoleInput);
	console->HookCommand("oe_info", callback_ObjectEditorConsoleInput);
	console->HookCommand("oe_resetorigin", callback_ObjectEditorConsoleInput);
	console->HookCommand("oe_resetrects", callback_ObjectEditorConsoleInput);
	console->HookCommand("oe_solid", callback_ObjectEditorConsoleInput);
	console->HookCommand("oe_background", callback_ObjectEditorConsoleInput);
	console->HookCommand("oe_shadow", callback_ObjectEditorConsoleInput);
	console->HookCommand("oe_warp", callback_ObjectEditorConsoleInput);
	console->HookCommand("oe_toanim", callback_ObjectEditorConsoleInput);
	console->HookCommand("oe_layer", callback_ObjectEditorConsoleInput);
	
	ResizeChildren();
}

ObjectEditor::~ObjectEditor()
{
	resman->Unload(mImage);
	resman->Unload(mGridImage);

	console->UnhookCommand("oe_load");
	console->UnhookCommand("oe_save");
	console->UnhookCommand("oe_info");
	console->UnhookCommand("oe_resetorigin");
	console->UnhookCommand("oe_resetrects");
	console->UnhookCommand("oe_solid");
	console->UnhookCommand("oe_background");
	console->UnhookCommand("oe_shadow");
	console->UnhookCommand("oe_warp");
	console->UnhookCommand("oe_toanim");
	console->UnhookCommand("oe_layer");
}

bool ObjectEditor::Load(string id)
{
	SetCaption("ObjectEditor - " + id);
	mId = id;

	SetDefaults();
	
	if (!LoadXml())
	{
		console->AddMessage(string(DIR_EDITOR) + DIR_ENTITIES + mId 
							+ ".xml not found. Loading default object properties.");
		if (!_tryToLoadImage())
		{
			console->AddMessage(string(DIR_EDITOR) + DIR_ENTITIES + mId 
								+ ".(jpg|png|mng|gif|bmp|img) not found. Assuming this is intentional.");
		}
		
		SetDefaults();
		return false;
	}

	console->AddMessage(mId + " loaded.");

	return true;
}

bool ObjectEditor::LoadXml()
{
	XmlFile xf;
	xf.SetParser(callback_ObjectEditorXml);

	if (xf.LoadFromFile( string(DIR_EDITOR) + DIR_ENTITIES + mId + ".xml" ))
	{
		if (xf.ParseSiblingsOnly(xf.mDoc.FirstChildElement(), this) != XMLPARSE_CANCEL)
		{
			return true;
		}
	}

	WARNING("[ObjectEditor::LoadXml] " + xf.GetError());

	return false;
}

int ObjectEditor::ParseXml(XmlFile* xf, TiXmlElement* e)
{
	string id = e->Value();
	
	if (id == "object") //type identifier
	{
		return xf->ParseSiblingsOnly(e->FirstChildElement(), this);
	}
	else if (id == "actor")
	{
		console->AddMessage("\\c900Entity is an Actor. Cannot edit.");
		return XMLPARSE_CANCEL;	
	}
	else if (id == "base") //<base shadow="0" solid="1" layer="1" />
	{
		mShadow = xf->GetParamInt(e, "shadow");
		mSolid = xf->GetParamInt(e, "solid");
		mLayer = xf->GetParamInt(e, "layer");
	}
	else if (id == "warp") // <warp id="library" name="wonderland_book" />
	{
		mWarpDestinationId = xf->GetParamString(e, "id");
		mWarpDestinationObject = xf->GetParamString(e, "name");
	}
	else if (id == "image") //<image file="blah.png" ver="1" />
	{
		mImageFile = xf->GetParamString(e, "file");
		//mImageVersion = xf->GetParamString(e, "ver");

		string file = string(DIR_EDITOR) + DIR_ENTITIES + mImageFile;
		mImage = resman->LoadImg(file);
		
		if (!mImage)
		{
			console->AddMessage("Image not found: " + file);
			return XMLPARSE_CANCEL;
		}
	}
	else if (id == "rect") // <rect>serialized collision rect</rect>
	{
		rect r = deserializeRect(xf->GetText(e));
		
		if (!isDefaultRect(r))
			mCollisionRects.push_back(r);
	}
	else if (id == "origin") // <origin x="#" y="#" />
	{
		mOrigin = xf->GetParamPoint2d(e, "position");
	}
	else if (id == "animation") // <animation w="#" delay="#"/> - Converts image to a horizontal animation
	{
		if (mImage && mImage->Width() != xf->GetParamInt(e, "w"))
		{
			if (!mImage->ConvertToHorizontalAnimation( rect(0, 0, xf->GetParamInt(e, "w"), 
															mImage->Height()), xf->GetParamInt(e, "delay") ))
			{
				return XMLPARSE_CANCEL;
			}
		}
	}
	
	return XMLPARSE_SUCCESS;
}

bool ObjectEditor::_tryToLoadImage()
{
	resman->Unload(mImage);

	//If it's in subdirs, filter those out
	string subdirs = getFileDirectory(mId);

	mImageFile = string(DIR_EDITOR) + DIR_ENTITIES + mId + ".*"; //wildcard extension

	mImageFile = getRandomFileMatchingPattern(mImageFile);

	if (mImageFile.empty()) //if none were found matching pattern
		return false;
		
	//append the subdirs to the beginning, since getRandomFile.. only returns the filename itself
	mImageFile = subdirs + mImageFile;

	//append directory since getRandomFile doesn't do that
	string file = string(DIR_EDITOR) + DIR_ENTITIES + mImageFile;

	mImage = resman->LoadImg(file);

	if (!mImage)
	{
		mImageFile.clear();
		return false;
	}

	return true;
}

void ObjectEditor::SetDefaults()
{
	RemoveAllRects();
	ResetOrigin();
	mEditingRects = false;
	mEditingOrigin = false;
	mShadow = false;
	mSolid = true;
	mDelay = 0;
	//mImageVersion = "1";
}

bool ObjectEditor::Save()
{
	uShort i;
	TiXmlElement* root;
	TiXmlElement* e;
	XmlFile xf;

	root = new TiXmlElement("object");
	xf.mDoc.LinkEndChild(root);

	// <object id="id">
	xf.SetParamString(root, "id", mId);

	// <base shadow="0" solid="1" layer="1" />
	e = xf.AddChildElement(root, "base");
	xf.SetParamInt(e, "shadow", mShadow);
	xf.SetParamInt(e, "solid", mSolid);
	xf.SetParamInt(e, "layer", mLayer);

	// <origin position="x,y" />
	e = xf.AddChildElement(root, "origin");
	xf.SetParamPoint2d(e, "position", mOrigin);
	
	// <warp id="library" name="wonderland_book" />
	if (!mWarpDestinationId.empty())
	{
		e = xf.AddChildElement(root, "warp");
		xf.SetParamString(e, "id", mWarpDestinationId);
		xf.SetParamString(e, "name", mWarpDestinationObject);
	}
	
	// <image file="something.png" md5="$$$" />
	if (!mImageFile.empty())
	{
		e = xf.AddChildElement(root, "image");
		xf.SetParamString(e, "file", mImageFile);
		//xf.SetParamString(e, "ver", mImageVersion);
		//xf.SetParamString(e, "md5", md5file(string(DIR_EDITOR) + DIR_ENTITIES + mImageFile));
	}
	
	//TODO: I don't like how avatar/animation is handled. I feel there is a much more
	//optimized technique that can be used. Look it over again.
	if (mImage)
	{
		// <animation w="#" delay="#"/>
		e = xf.AddChildElement(root, "animation");
		xf.SetParamInt(e, "w", mImage->Width());
		xf.SetParamInt(e, "delay", mDelay);
	}
	
	// <rect>Serialized collision rect</rect>
	for (i = 0; i < mCollisionRects.size(); i++)
	{
		e = xf.AddChildElement(root, "rect");
		xf.AddChildText( e, serializeRect(mCollisionRects.at(i)) );
	}

	xf.SaveToFile(string(DIR_EDITOR) + DIR_ENTITIES + mId + ".xml");
	
	console->AddMessage(string(DIR_EDITOR) + DIR_ENTITIES + mId + ".xml saved."); 
}

rect ObjectEditor::GetImagePosition()
{
	rect r = GetScreenPosition();
	r.x += 5;
	r.y += 30;
	if (mImage)
	{
		r.w = mImage->Width();
		r.h = mImage->Height();
	}
	
	r.x -= mCamera.x;
	r.y -= mCamera.y;
	
	return r;
}

bool ObjectEditor::ConvertToHorizontalAnimation(uShort width, uShort delay)
{
	if (!mImage) return false;

	mDelay = delay;
	if (!mImage->ConvertToHorizontalAnimation( rect(0, 0, width, mImage->Height()), delay ))
		return false;
	
	ResetOrigin();
	RemoveAllRects();

}

bool ObjectEditor::RemoveAllRectsIntersectingWith(rect r)
{
	bool result = false;
	for (uShort i = 0; i < mCollisionRects.size(); i++)
	{
		if (areRectsIntersecting(mCollisionRects.at(i), r))
		{
			mCollisionRects.erase(mCollisionRects.begin() + i);
			i--;
			result = true;
		}
	}
	return result;
}

void ObjectEditor::RemoveAllRects()
{
	mCollisionRects.clear();
}

void ObjectEditor::OffsetCamera(sShort x, sShort y)
{
	mCamera.x += x;
	mCamera.y += y;
}

void ObjectEditor::Event(SDL_Event* event)
{
	if (!HasKeyFocus())
		return;
		
	switch (event->type)
	{
		case SDL_KEYUP: 
		{
			switch (event->key.keysym.sym)
			{
				case SDLK_r: 
					mEditingRects = !mEditingRects;
					mEditingOrigin = false;
					break;
				case SDLK_o: 
					mEditingOrigin = !mEditingOrigin; 
					mEditingRects = false;
					break;
				case SDLK_g:
					mShowGrid = !mShowGrid;
					break;
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
				default: break;	
			}
		} break;
		case SDL_MOUSEBUTTONUP: 
		{
			if (mEditingRects)
			{
				if (event->button.button == SDL_BUTTON_LEFT)
				{
					if (isDefaultRect(mHoldingRect)) //start a new rect
					{
						mHoldingRect = GetImagePosition();
						mHoldingRect.x = gui->GetMouseX() - mHoldingRect.x;
						mHoldingRect.y = gui->GetMouseY() - mHoldingRect.y;
						mHoldingRect.w = mHoldingRect.h = 1;
					}
					else //Add as a new collision rect and reset our holding rect
					{
						//only add if it isn't a tiny rect
						if (mHoldingRect.w > 3 && mHoldingRect.h > 3)
							mCollisionRects.push_back(mHoldingRect);
						
						mHoldingRect = rect();
					}
				}
				else if (event->button.button == SDL_BUTTON_RIGHT && isDefaultRect(mHoldingRect))
				{
					rect r = GetImagePosition();
					r.x = gui->GetMouseX() - r.x;
					r.y = gui->GetMouseY() - r.y;
					r.w = r.h = 1;
					RemoveAllRectsIntersectingWith( r );
				}
			}
			else if (mEditingOrigin && event->button.button == SDL_BUTTON_LEFT)
			{
				mEditingOrigin = false;
			}
		} break;
		case SDL_MOUSEMOTION: 
		{
			_adjustHoldingRect();
			_adjustOrigin();
		} break;
		default: break;
	}
	
	if (!mEditingOrigin && !mEditingRects)
		Frame::Event(event);
}

void ObjectEditor::RenderGrid(rect r)
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

void ObjectEditor::Render()
{
	Image* scr = Screen::Instance();
	rect oldclip = scr->GetClip();
	uShort i;
	rect r;

	//Frame, mImage in frame, collision rects, origin point, position of cursor relative to our image
	Frame::Render();
	
	if (mResizing)
		return;
	
	rect rr = GetImagePosition();
		
	r = GetScreenPosition();
	r.x += 5;
	r.y += 30;
	r.w = Width() - 10;
	r.h = Height() - 35;

	scr->SetClip(r);

	scr->DrawRect(r, mBackground);

	//draw a containing rect over the entire object
	scr->DrawRect(rr, color(0,255,0), false);
	
	if (mImage)
		mImage->Render(scr, rr.x, rr.y);
		
	if (mShowGrid)
		RenderGrid(r);	
	
	//draw collision rects
	for (i = 0; i < mCollisionRects.size(); i++)
	{
		r = mCollisionRects.at(i);
		r.x += rr.x;
		r.y += rr.y;
		scr->DrawRect(r, color(255,0,0), false);
	}
	
	if (!isDefaultRect(mHoldingRect) && mEditingRects)
	{
		r = mHoldingRect;
		r.x += rr.x;
		r.y += rr.y;
		scr->DrawRect(r, color(255,255,0), false);
	}
	
	//draw circle around origin point
	r.x = rr.x + mOrigin.x - 2;
	r.y = rr.y + mOrigin.y - 2;
	r.w = 5;
	r.h = 5;
	scr->DrawRound(r, 2, color(0,0,255));

	scr->SetClip(oldclip);
}

void ObjectEditor::ResetOrigin()
{
	if (mImage)
	{
		mOrigin.y = mImage->Height();
		mOrigin.x = mImage->Width() / 2;
	}
	else
	{
		mOrigin.x = mOrigin.y = 0;
	}
}

void ObjectEditor::DisplayInfo()
{
	string info = "Object " + mId;
	if (mImage)
		info += "\\nImage Dimensions: (" + its(mImage->Width()) + "," + its(mImage->Height()) + ")";
	else
		info += "\\nNo Image.";
	info += "\\nIs Solid: " + its(mSolid);
	info += "\\nHas Shadow: " + its(mShadow);
	info += "\\nOrigin: (" + its(mOrigin.x) + "," + its(mOrigin.y) + ")";
	info += "\\nMap Layer: " + its(mLayer);
	info += "\\nCollision Rect Count: " + its(mCollisionRects.size());
	info += "\\nWarp Destination Id: " + mWarpDestinationId + " Object Name: " + mWarpDestinationObject;
	console->AddFormattedMessage(info);
}

void ObjectEditor::_adjustOrigin()
{
	if (mEditingOrigin)
	{
		rect r = GetImagePosition();
		mOrigin = point2d(gui->GetMouseX() - r.x, gui->GetMouseY() - r.y);
		if (mOrigin.x < 0)
			mOrigin.x = 0;
		if (mOrigin.y < 0)
			mOrigin.y = 0;
		if (mOrigin.x > r.w)
			mOrigin.x = r.w;
		if (mOrigin.y > r.h)
			mOrigin.y = r.h;
	}
}

void ObjectEditor::_adjustHoldingRect()
{
	if (!isDefaultRect(mHoldingRect) && mEditingRects)
	{
		rect r = GetImagePosition();
		r.x = gui->GetMouseX() - r.x;
		r.y = gui->GetMouseY() - r.y;
		r.w = r.h = 1;

		if (r.x - mHoldingRect.x > 0)
			mHoldingRect.w = r.x - mHoldingRect.x;

		if (r.y - mHoldingRect.y > 0)
			mHoldingRect.h = r.y - mHoldingRect.y ;
	}
}

void ObjectEditor::OnConsoleInput(Console* c, string s)
{
	vString v;
	explode(&v, &s, " ");
	
	v.at(0) = lowercase(v.at(0));
	if (v.at(0) == "oe_load")
	{
		if (v.size() < 2)
			c->AddMessage("Invalid. oe_load <id>");
		else
			Load(v.at(1));
	}
	else if (v.at(0) == "oe_save")
	{
		Save();
	}
	else if (v.at(0) == "oe_info")
	{
		DisplayInfo();	
	}
	else if (v.at(0) == "oe_resetorigin")
	{
		ResetOrigin();
	}
	else if (v.at(0) == "oe_resetrects")
	{
		RemoveAllRects();
	}
	else if (v.at(0) == "oe_solid")
	{
		if (v.size() < 2)
			c->AddMessage("Invalid. oe_solid <1 or 0>");
		else
			mSolid = sti(v.at(1));
	}
	else if (v.at(0) == "oe_layer")
	{
		if (v.size() < 2)
			c->AddMessage("Invalid. oe_layer <0, 1, or 2>");
		else
			mLayer = sti(v.at(1));
	}
	else if (v.at(0) == "oe_background")
	{
		if ( v.size() < 2 )
			c->AddMessage("Invalid. oe_background <\\\\cRGB>");
		else
			mBackground = slashCtoColor( v.at(1) );
	}
	else if (v.at(0) == "oe_shadow")
	{
		if (v.size() < 2)
			c->AddMessage("Invalid. oe_shadow <1 or 0>");
		else
			mShadow = sti(v.at(1));
	}
	else if (v.at(0) == "oe_warp")
	{
		if (v.size() < 2)
		{
			c->AddMessage("Erased Warp Properties");
			mWarpDestinationId.clear();
			mWarpDestinationObject.clear();
		}
		else
		{
			mWarpDestinationId = v.at(1);
			if (v.size() > 2)
				mWarpDestinationObject = v.at(2);
		}
	}
	else if (v.at(0) == "oe_toanim")
	{
		if (v.size() < 3)
			c->AddMessage("Invalid. oe_toanim <width> <delay>");
		else
			ConvertToHorizontalAnimation( sti(v.at(1)), sti(v.at(2)) );
	}

}

void ObjectEditor::SetPosition(rect r)
{
	if (r.w > 100 && r.h > 100)
		Frame::SetPosition(r);
}

