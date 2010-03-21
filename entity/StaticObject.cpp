
#include "StaticObject.h"
#include "LocalActor.h"
#include "../game/GameManager.h"
#include "../map/Map.h"
#include "../core/io/FileIO.h"

void listener_staticObjectWarperEventMove(MessageListener* listener, MessageData& event, void* sender)
{
	Actor* a = (Actor*)event.ReadUserdata("entity");
	StaticObject* so = (StaticObject*)listener->userdata;
	
	if (a != game->mPlayer || so->GetWarpId().empty())
		return;

	rect r;
	r.w = 8;
	r.h = 8;
	
	r.x = event.ReadInt("oldx") - 4;
	r.y = event.ReadInt("oldy") - 8;
	bool collidesWithOld = so->CollidesWith(r);
	
	r.x = event.ReadInt("newx") - 4;
	r.y = event.ReadInt("newy") - 8;
	bool collidesWithNew = so->CollidesWith(r);
	
	//If neither position collides with us, do nothing.
	//if last position collides and new doesn't, do nothing.
	
	//if new collides, and last doesn't, warp them!
	if (collidesWithNew && !collidesWithOld)
	{
		game->mPlayer->Warp(so->GetWarpId(), point2d(), so->GetWarpObject());
	}
}

StaticObject::StaticObject()
	: Entity()
{
	mType = ENTITY_STATICOBJECT;
	mImage = NULL;
	mOriginalImage = NULL;
	mRotation = 0.0;
	mScale = 1.0;
	mDepth = 0;
	mUseAA = false;
	mWarpEntityMoveListener = NULL;
}

StaticObject::~StaticObject()
{
	PRINT("~StaticObject");
	resman->Unload(mImage);
	resman->Unload(mOriginalImage);

	messenger.RemoveListener(mWarpEntityMoveListener);
}

TimeProfiler soRenderProfiler("StaticObject::Render");

void StaticObject::Render(uLong ms)
{
	ASSERT(mMap);

	soRenderProfiler.Start();

	Image* scr = Screen::Instance();
	rect r;
	
	if (mImage)
	{
		RenderShadow();
		
		r = rect( mPosition.x - mOrigin.x, mPosition.y - mOrigin.y, 
					mImage->Width(), mImage->Height() );

		r = mMap->ToScreenPosition( r );

/*
		rect mapScreenRect = mMap->GetScreenPosition();
		r = rectIntersection(r, mapScreenRect);
		*/
		mImage->Render(scr, r.x, r.y);
	}
	
	soRenderProfiler.Stop();
}

rect StaticObject::GetBoundingRect()
{
	if (!mImage)
		return rect();
		
	//TODO: this is wrong!
	return rect( mPosition.x - mOrigin.x,
				mPosition.y - mOrigin.y,
				mImage->Width(), mImage->Height() );
}

void StaticObject::Rotozoom(double degree, double zoom)
{
	mRotation = degree;
	mScale = zoom;
	ASSERT(mOriginalImage);

	Image* img = mOriginalImage->Clone(true); //create a completely unique copy
	resman->Unload(mImage); //unreference the original
	img->Rotate(mRotation, mScale, mUseAA);
	mImage = img;
}

void StaticObject::LoadImage(string file)
{
	resman->Unload(mImage);
	resman->Unload(mOriginalImage);
	
	mOriginalImage = resman->LoadImg(file);

	if (!mOriginalImage)
		mImage = NULL;
	else
		mImage = mOriginalImage->Clone();

	mRotation = 0.0;
	mScale = 1.0;
}

int StaticObject::ReadXml(XmlFile* xf, TiXmlElement* e, bool online)
{
	string id = e->Value();
	
	PRINT("StaticObject::ReadXml " + id);
	
	string file;
	
	//<image file="blah.png" md5="hash" frameset="id" depth="0" /> tells it load the image and change the frameset if specified
	if (id == "image")
	{
		if (online)
			file = DIR_CACHE;
		else
			file = DIR_EDITOR;	

		file += DIR_ENTITIES + xf->GetParamString(e, "file");
		
		LoadImage(file);
		if (!mImage)
		{
			WARNING("File " + file + " is corrupt!");
			if (online)
				removeFile(file); //kill corrupt cache file

			return XMLPARSE_CANCEL;
		}
			
		if (xf->ParamExists(e, "frameset"))
			mImage->SetFrameset(xf->GetParamString(e, "frameset"));
			
		if (xf->ParamExists(e, "depth"))
			mDepth = xf->GetParamInt(e, "depth");
	}
	else if (id == "warp")
	{
		SetWarp(xf->GetParamString(e, "id"), xf->GetParamString(e, "name"));
	}
	else if (id == "animation") //<animation w="#" delay="#"/> converts image to horiz anim if not already animated 
	{
		if (mImage && mImage->Width() != xf->GetParamInt(e, "w"))
		{
			string sDelay = xf->GetParamString(e, "delay");
			uShort uDelay;
			vString v;
			explode(&v, &sDelay, "-");
			
			if (v.size() < 2)
				uDelay = sti(v.at(0));
			else
				uDelay = rnd2( sti(v.at(0)), sti(v.at(1)) );
			
			if (!mImage->ConvertToHorizontalAnimation( rect(0, 0, xf->GetParamInt(e, "w"), 
															mImage->Height()), uDelay))
			{
				return XMLPARSE_CANCEL;
			}
		}
	}
	else //Unidentified, let the base handle it
	{
		return Entity::ReadXml(xf, e, online);
	}
	
	PRINT("StaticObject::ReadXml " + id + " End");
	
	return XMLPARSE_SUCCESS;
}

void StaticObject::SetAA(bool b)
{
	if (mUseAA != b)
	{
		mUseAA = b;
		if (mRotation != 0.0 || mScale != 1.0)
			Rotozoom(mRotation, mScale);
	}
}

void StaticObject::SetWarp(string id, string objectName)
{
	mWarpDestinationId = id;
	mWarpDestinationObject = objectName;

	messenger.RemoveListener(mWarpEntityMoveListener);
	
	if (!mWarpDestinationId.empty())
		mWarpEntityMoveListener = messenger.AddListener("ENTITY_MOVE", listener_staticObjectWarperEventMove, 
														NULL, this);
}

