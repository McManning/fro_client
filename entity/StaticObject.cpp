
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

uShort timer_StaticObjectAnimate(timer* t, uLong ms)
{
	StaticObject* o = (StaticObject*)t->userData;

	if ( o && o->_animate() )
		return TIMER_CONTINUE;

	return TIMER_DESTROY;
}

StaticObject::StaticObject()
	: Entity()
{
	mType = ENTITY_STATICOBJECT;
	mImage = NULL;
	mOriginalImage = NULL;
	mAnimationTimer = NULL;
	mRotation = 0.0;
	mScale = 1.0;
	mDepth = 0;
	mUseAA = false;
	mWarpEntityMoveListener = NULL;
}

StaticObject::~StaticObject()
{
	resman->Unload(mImage);
	resman->Unload(mOriginalImage);

	messenger.RemoveListener(mWarpEntityMoveListener);
}

TimeProfiler soRenderProfiler("StaticObject::Render");

void StaticObject::Render()
{
	ASSERT(mMap);

	soRenderProfiler.Start();

	Image* scr = Screen::Instance();
	rect r;
	
	if (mImage)
	{
		RenderShadow();
		
		r = GetBoundingRect();

		if (!IsPositionRelativeToScreen())
			r = mMap->ToScreenPosition( r );

		mImage->Render(scr, r.x, r.y);
	}
	
	soRenderProfiler.Stop();
	
	Entity::Render();
}

rect StaticObject::GetBoundingRect()
{
	if (!mImage)
		return rect();

	return rect( mPosition.x - mOrigin.x,
				mPosition.y - mOrigin.y,
				mImage->Width(), mImage->Height() );
}

Image* StaticObject::GetImage()
{
	if (mImage)
		return mImage;
	
	return mOriginalImage;
}

void StaticObject::Rotozoom(double degree, double zoom)
{
	mRotation = degree;
	mScale = zoom;
	ASSERT(mOriginalImage);

	Image* img = mOriginalImage->Clone(true); //create a completely unique copy
	
	if (mImage != mOriginalImage)
		resman->Unload(mImage); //unreference the original
		
	img->Rotate(mRotation, mScale, mUseAA);
	mImage = img;
	
	AddPositionRectForUpdate();
}

void StaticObject::LoadImage(string file)
{
	if (mImage != mOriginalImage)
		resman->Unload(mImage);
		
	resman->Unload(mOriginalImage);
	
	mOriginalImage = resman->LoadImg(file);

	if (!mOriginalImage)
	{
		mImage = NULL;
		console->AddMessage(" *\\c900 StaticObject:" + mId + " Failed to load " + file);
	}
	else
		mImage = mOriginalImage->Clone();

	
	PlayAnimation();
	mRotation = 0.0;
	mScale = 1.0;
}

void StaticObject::SetImage(Image* img)
{
	if (mImage != mOriginalImage)
		resman->Unload(mImage);
	
	resman->Unload(mOriginalImage);
	
	mOriginalImage = img;
	
	if (mOriginalImage)
		mImage = mOriginalImage->Clone();

	PlayAnimation();
	mRotation = 0.0;
	mScale = 1.0;	
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

// Called by the timer
bool StaticObject::_animate()
{
	Image* img = GetImage();
	
	if (mAnimationTimer && img) // if the animation is playing
	{
		mAnimationTimer->interval = img->ForwardCurrentFrameset();
		
		AddPositionRectForUpdate();
		
		// if we hit the end of the animation, destroy the timer. 
		if (mAnimationTimer->interval == ULONG_MAX)
		{
			mAnimationTimer = NULL;
			return false;
		}
	}

	return (img != NULL);
}

void StaticObject::PlayAnimation()
{
	Image* img = GetImage();
	ASSERT(img);
	
	img->Reset();
	
	SDL_Frame* f = img->Frame();
	ASSERT(f);
	
	if (!mAnimationTimer)
	{
		// only add if there's a reason to animate
		if (img->mImage->CountFrames() > 1)
			mAnimationTimer = timers->Add("", f->delay, false, timer_StaticObjectAnimate, NULL, this);
	}
	else
	{
		mAnimationTimer->interval = f->delay;
	}
	
	AddPositionRectForUpdate();
}

void StaticObject::StopAnimation()
{
	if (mAnimationTimer)
	{	
		timers->Remove(mAnimationTimer);
		mAnimationTimer = NULL;
	}
	
	AddPositionRectForUpdate();
}

