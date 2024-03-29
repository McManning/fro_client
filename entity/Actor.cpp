
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


#include <lua.hpp>
#include "Actor.h"
#include "StaticObject.h"
#include "TextObject.h"
#include "LocalActor.h"
#include "ChatBubble.h"
#include "../map/Map.h"
#include "../avatar/Avatar.h"

uShort timer_processMovement(timer* t, uLong ms)
{
	Actor* m = (Actor*)t->userData;
	if (m)
	{
		m->ProcessMovement();
		return TIMER_CONTINUE;
	}
	else
	{
		WARNING("No actor assigned to timer " + pts(t));
	}

	return TIMER_DESTROY;
}

uShort timer_ActorAnimate(timer* t, uLong ms)
{
	Actor* a = (Actor*)t->userData;

	if (!a)
	{
		WARNING("No actor assigned to timer " + pts(t));
		return TIMER_DESTROY;
	}

	return (a->_animate()) ? TIMER_CONTINUE : TIMER_DESTROY;
}

uShort timer_checkLoadingAvatar(timer* t, uLong ms)
{
    Actor* a = (Actor*)t->userData;

    if (a)
	{
        // keep the timer alive until the avatar stops loading
        if (a->CheckLoadingAvatar())
            return TIMER_CONTINUE;

        a->mCheckLoadingAvatarTimer = NULL;
	}

    return TIMER_DESTROY;
}


Actor::Actor()
	: Entity()
{
	mAvatar = NULL;
	mLoadingAvatar = NULL;
	mAnimationTimer = NULL;
	mCheckLoadingAvatarTimer = NULL;
	mStep = 0;
	mAction = IDLE;
	mDirection = SOUTH;
	mSpeed = SPEED_WALK;
	mType = ENTITY_ACTOR;
	mLimitedAvatarSize = false;
	mJumpHeight = 0;
	mFalling = true;
	SetIgnoreSolids(false);

	mMovementTimer = timers->Add("movproc",
								PROCESS_MOVE_INTERVAL, false,
								timer_processMovement,
								NULL,
								this);
}

Actor::~Actor()
{
	SAFEDELETE(mAvatar);
	SAFEDELETE(mLoadingAvatar);
}

rect Actor::GetBoundingRect()
{
	rect r;

	if (mAvatar && mAvatar->GetImage())
	{
		r.w = mAvatar->GetImage()->Width();
		r.h = mAvatar->GetImage()->Height();
	}
	else
	{
		r.w = 16;
		r.h = 16;
	}

	r.x = mPosition.x - (r.w / 2);
	r.y = mPosition.y - r.h - mJumpHeight;
	r.h += mJumpHeight;

	return r;
}

Image* Actor::GetImage()
{
	Avatar* a = GetAvatar();

	if (a)
		return a->GetImage();

	return NULL;
}

void Actor::SetName(string name)
{
	mName = name;
}

void Actor::Move(direction dir, sShort distance, byte speed)
{
	if (speed == 0)
		speed = mSpeed;
	else
		SetSpeed(speed);

	SetDirection(dir);
	SetAction(IDLE);

	rect r;
	offsetRectByDirection(&r, dir, distance);

	//Set our destination
	mDestination.x += r.x;
	mDestination.y += r.y;

	if (!IsJumping())
		mPreviousPosition = mPosition;

	//WARNING("SETTINGDST " + its(mPosition.x) + "," + its(mPosition.y) + "->"
	//		 + its(mDestination.x) + "," + its(mDestination.y));

	if (mAvatar && mAvatar->GetImage())
	{
		_syncAvatarFrameset();
		StopAnimation();
	}
	mStep = 0;
}

void Actor::MoveTo(point2d destination, byte speed)
{
	//FATAL("TODO: Redesign");

	SetAction(IDLE);

	mPreviousPosition = mPosition;
	mDestination = destination;

	if (speed != 0) //0 = use mSpeed
		SetSpeed(speed);

	if (mAvatar && mAvatar->GetImage())
	{
		_syncAvatarFrameset();
		StopAnimation();
	}
	mStep = 0;
}

/*	Offset mPosition toward the specified direction, check if it collides, then reset mPosition and return true if collided with something */
bool Actor::CanMove(direction d, sShort distance)
{
	ASSERT(mMap);

	if (IgnoreSolids())
		return true;

	point2d temp = mPosition;

	//Continuously offset our position in the specified direction and distance, checking
	//for collision along the way
/*TODO:	for (int totalDistance = 0; totalDistance <= distance; totalDistance += 8)
	{

	}
*/
	rect r;
	r.x = mPosition.x;
	r.y = mPosition.y;
	offsetRectByDirection(&r, d, distance);

	mPosition.x = r.x;
	mPosition.y = r.y;

	bool result = true;

	if ((r.x >= mMap->mWidth && mMap->mWidth > 0) || r.x < 0)
		result = false;

	if ((r.y >= mMap->mHeight && mMap->mHeight > 0) || r.y < 0)
		result = false;

	//if we're still on the map, check for entity collisions if we actually collide with others
	if (result)
		result = !IsCollidingWithSolid();

	mPosition = temp; //reset mPosition to it's original

	return result;
}

void Actor::_dispatchEntityMove()
{
	//don't dispatch if we actually haven't moved
	if (mPreviousPosition.x == mDestination.x && mPreviousPosition.y == mDestination.y)
		return;

	//DEBUGOUT("ENTITY_MOVE " + mName + ": " + p2dts(mPreviousPosition) + " " + p2dts(mDestination));
	MessageData md("ENTITY_MOVE");
	md.WriteUserdata("entity", this);
	md.WriteInt("oldx", mPreviousPosition.x);
	md.WriteInt("oldy", mPreviousPosition.y);
	md.WriteInt("newx", mDestination.x);
	md.WriteInt("newy", mDestination.y);
	md.WriteInt("direction", GetDirection());
	md.WriteInt("speed", GetSpeed());
	messenger.Dispatch(md, this);

	mPreviousPosition = mPosition;
}

bool Actor::ProcessMovement()
{
	if (!mMap)
		return false;

	if (!IsMoving()) //already there, don't need to move
	{
		if (mActionBuffer.empty())
			return false;

		_checkActionBuffer(); //could have steps to perform
	}

	AddPositionRectForUpdate();

	_stepTowardDestination();

	_recalculateDirection();

	_recalculateStep();

	if (mStep == 4 && mMap) //Run these messages, but not too fast. Only when we get halfway through a step
	{
		mMap->QueueEntityResort();

		//don't send until we land, and have actually changed coordinate position
		if (!IsJumping() && (mPreviousPosition.x != mPosition.x || mPreviousPosition.y != mPosition.y) )
		{
			_dispatchEntityMove();
		}
	}

	_processJump();

	AddPositionRectForUpdate();

	/*	If they just reached the target position,
		do whatever is necessary after we're finished	*/
	if (!IsMoving())
	{
		PostMovement();
	}

	return true;
}

void Actor::PostMovement()
{
	_checkActionBuffer();

	if (!IsMoving())
	{
		if (mAvatar && mAvatar->GetImage())// && (GetAction() != IDLE || mAvatar->mLoopStand))
		{
			_syncAvatarFrameset();
			PlayAnimation();
		}
	}
}

// Called by the timer
bool Actor::_animate()
{
	Image* img = mAvatar->GetImage();

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

void Actor::PlayAnimation()
{
	Image* img = mAvatar->GetImage();
	ASSERT(img);

	img->Reset();

	SDL_Frame* f = img->Frame();
	ASSERT(f);

	if (!mAnimationTimer)
	{
		// only add if there's a reason to animate
		if (img->mImage->CountFrames() > 1)
			mAnimationTimer = timers->Add("", f->delay, false, timer_ActorAnimate, NULL, this);
	}
	else
	{
		mAnimationTimer->interval = f->delay;
	}

	AddPositionRectForUpdate();
}

void Actor::StopAnimation()
{
	if (mAnimationTimer)
	{
		timers->Remove(mAnimationTimer);
		mAnimationTimer = NULL;
	}

	AddPositionRectForUpdate();
}

/* 	Based on the current action of our character, change the frameset we are currently playing.
	Will also reset the current frame we're on, so only call when it needs to be changed.
*/
void Actor::_syncAvatarFrameset()
{
	if (!mAvatar || !mAvatar->GetImage()) return;

	string dir;

	//TODO: Directional downgrading. For now, just support N,E,S,W
	if (mDirection == NORTH || mDirection == NORTHEAST || mDirection == NORTHWEST)
		dir = "8";
	else if (mDirection == EAST)
		dir = "6";
	else if (mDirection == WEST)
		dir = "4";
	else //s, sw, se, or (server forbid) invalid.
		dir = "2";

	Image* img = mAvatar->GetImage();

	if (mAction == SIT)
	{
		if ( !img->SetFrameset("_sit_" + dir) )
		{
			//Check for backwards compatability with kyat-mngs.
			//TODO: This looks HORRIBLE. Please fix.
			if (dir == "8" && img->SetFrameset("_jump_2")) { }
			else if (dir == "4" && img->SetFrameset("_left_2")) { }
			else if (dir == "6" && img->SetFrameset("_right_2")) { }
			else if ( !img->SetFrameset("_stop_" + dir) )
			{
				if ( !img->SetFrameset("_move_" + dir) )
				{
					img->SetFrameset("_move_2");
				}
			}
		}
	}
	else
	{
		if (IsMoving() && !IsJumping())
		{
			if ( !img->SetFrameset("_move_" + dir) )
			{
				img->SetFrameset("_move_2");
			}
		}
		else
		{
			//Look for a stop frame. If we don't have one, try a _stop_2,
			//if don't have that, try a move frame. If not that, _move_2.
			if ( !img->SetFrameset("_stop_" + dir) )
			{
				if ( !img->SetFrameset("_stop_2") && !img->SetFrameset("_move_" + dir) )
				{
					img->SetFrameset("_move_2");
				}
			}
		}
	}

	//TODO: Detect run, check for _RUN_dir, Detect jump, check for _JUMP_dir

	AddPositionRectForUpdate();
}

void Actor::AddToActionBuffer(string data)
{
	mActionBuffer += data;
}

void Actor::_checkActionBuffer()
{
	if (mActionBuffer.empty()) return;

	char c = mActionBuffer.at(0);
	byte b;

PRINTF("Checking Buffer [%s]: %c\n", mId.c_str(), c);
	bool recheck = false; //set to true if the buffer should be scanned again after processing

	if (c >= '1' && c <= '9') //Movement
	{
		SetAction(IDLE);
		Move(charToDirection(c));
	}
	else if (c == 'w')
	{
		SetSpeed(SPEED_WALK);
		recheck = true;
	}
	else if (c == 'r')
	{
		SetSpeed(SPEED_RUN);
		recheck = true;
	}
	else if (c == 's') //Sit + 1 char for dir
	{
		if (mActionBuffer.size() > 1)
		{
			SetDirection(charToDirection(mActionBuffer.at(1)));
			mActionBuffer.erase(mActionBuffer.begin());
		}
		SetAction(SIT);
	}
	else if (c == 't') //Stand/Turn + 1 char for dir
	{
		if (mActionBuffer.size() > 1)
		{
			SetDirection(charToDirection(mActionBuffer.at(1)));
			mActionBuffer.erase(mActionBuffer.begin());
		}
		SetAction(IDLE);
	}
	else if (c == 'j') //Jump + 1 char for type (s,w,r)
	{
		b = STANDING_JUMP;
		if (mActionBuffer.size() > 1)
		{
			switch (mActionBuffer.at(1))
			{
				case 's': b = STANDING_JUMP; break;
				case 'w': b = WALKING_JUMP; break;
				case 'r': b = RUNNING_JUMP; break;
				default: break; //malformed!
			}
			mActionBuffer.erase(mActionBuffer.begin());
		}
		Jump(b);
	}
	else if (c == 'c') //Compare supplied x/y with position. format: cXXXX.YYYY.
	{
		size_t xend = mActionBuffer.find(".", 0);
		size_t yend = mActionBuffer.find(".", xend+1);

		if (yend == string::npos || xend == string::npos)
		{
			WARNING("c:npos : " + mActionBuffer);
		}

		sShort x = sti(mActionBuffer.substr(1, xend - 1));
		sShort y = sti(mActionBuffer.substr(xend + 1, yend - xend - 1));

		//If we need to correct position, do so.
		if (mPosition.x != x || mPosition.y != y)
		{
			if (x == 0 && y == 0)
			{
				//DEBUGOUT("\\c700 MANTICORE Double Zero Ignore " + mId);
			}
			else
			{
				//DEBUGOUT("\\c700 MANTICORE Correcting Position " + mId
				//		+ " " + its(mPosition.x) + "," + its(mPosition.y) + " -> "
				//		+ its(x) + "," + its(y));
				SetPosition(point2d(x, y));
			}
		}

		//Erase our entire coordinate chunk.
		mActionBuffer.erase(0, yend);

		recheck = true;
	}
//	else ignore
//		console->AddMessage("\\c700 Unknown in buffer " + c);

	mActionBuffer.erase(mActionBuffer.begin());
PRINTF("Check done. Recheck? %i\n", recheck);
	if (recheck)
		_checkActionBuffer();
}

bool Actor::IsMoving() const
{
	if (IsJumping()) return true;
	return !(mDestination.x == mPosition.x && mDestination.y == mPosition.y);
}

void Actor::SetPosition(point2d position)
{
	Entity::SetPosition(position);
	mDestination = mPosition; //so it won't try to walk to mDestination

//	AddToActionBuffer('c' + its(position.x) + "." + its(position.y) + ".");
}

void Actor::SetDirection(direction newDir)
{
    if (newDir != mDirection)
	{
		mDirection = newDir;
		_syncAvatarFrameset();
    }
}

void Actor::SetSpeed(byte newSpeed)
{
	if (mSpeed != newSpeed)
	{
		mSpeed = newSpeed;
		_syncAvatarFrameset();
	}
}

void Actor::SetAction(byte newAction)
{
	if (mAction != newAction)
	{
		mAction = newAction;
		_syncAvatarFrameset();
	}
}

void Actor::_recalculateDirection()
{
	if (mPosition.x == mDestination.x && mPosition.y == mDestination.y)
		return;

	direction dir;

	//Recalculate direction
	if (mPosition.y < mDestination.y) //moving south
	{
		if (mPosition.x > mDestination.x)
			dir = SOUTHWEST;
		else if (mPosition.x < mDestination.x)
			dir = SOUTHEAST;
		else
			dir = SOUTH;
	}
	else if (mPosition.y > mDestination.y) //north
	{
		if (mPosition.x > mDestination.x)
			dir = NORTHWEST;
		else if (mPosition.x < mDestination.x)
			dir = NORTHEAST;
		else
			dir = NORTH;
	}
	else //moving on x only
	{
		if (mPosition.x > mDestination.x)
			dir = WEST;
		else if (mPosition.x < mDestination.x)
			dir = EAST;
	}

	SetDirection(dir);
}

void Actor::_recalculateStep()
{
	if (mSpeed <= SPEED_WALK)
		mStep += 1;
	else
		mStep += 2;

	if (mStep == 4)
	{
		if (mAvatar && mAvatar->GetImage())
			mAvatar->GetImage()->ForwardCurrentFrameset(true);
	}
	else if (mStep == 8)
	{
		if (mAvatar && mAvatar->GetImage())
			mAvatar->GetImage()->ForwardCurrentFrameset(true);
		mStep = 0; //reset step
	}
}

void Actor::_stepTowardDestination()
{
	//mPreviousPosition = mPosition;

	point2d p = mPosition;

	if (mPosition.x < mDestination.x)
	{
		if (mPosition.x + mSpeed > mDestination.x)
			mPosition.x = mDestination.x;
		else
			mPosition.x += mSpeed;
	}
	else if (mPosition.x > mDestination.x)
	{
		if (mPosition.x - mSpeed < mDestination.x)
			mPosition.x = mDestination.x;
		else
			mPosition.x -= mSpeed;
	}

	if (mPosition.y < mDestination.y)
	{
		if (mPosition.y + mSpeed > mDestination.y)
			mPosition.y = mDestination.y;
		else
			mPosition.y += mSpeed;
	}
	else if (mPosition.y > mDestination.y)
	{
		if (mPosition.y - mSpeed < mDestination.y)
			mPosition.y = mDestination.y;
		else
			mPosition.y -= mSpeed;
	}

	//Make sure we can move!
	bool canMove = true;

	if (!IgnoreSolids())
	{
		if ((mPosition.x >= mMap->mWidth && mMap->mWidth > 0) || mPosition.x < 0)
			canMove = false;

		if ((mPosition.y >= mMap->mHeight && mMap->mHeight > 0) || mPosition.y < 0)
			canMove = false;

		//if we're still on the map, check for entity collisions
		if (canMove)
			canMove = !IsCollidingWithSolid();
	}

	if (!canMove) //revert mPosition back to original, and cancel destination. We hit a block.
	{
		mPosition = p;
		mDestination = p;
	}
}

bool Actor::SwapAvatars()
{
    if (mLoadingAvatar)
    {
    	if (!mLoadingAvatar->Convert())
    	{
    		AvatarError(AVYERR_CONVERT);
    	}
    	else if ((mLoadingAvatar->GetImage()->Width() > MAX_AVATAR_WIDTH && mLimitedAvatarSize)
    		|| (mLoadingAvatar->GetImage()->Height() > MAX_AVATAR_HEIGHT && mLimitedAvatarSize))
    	{
    		AvatarError(AVYERR_SIZE);
    	}
    	else // All post-load checks are fine, load it.
    	{
    		byte mod = Avatar::MOD_NONE;

    		//if our loading avatar decided to set a custom mod, set it
    		if (mLoadingAvatar->mModifier != Avatar::MOD_NONE)
    			mod = mLoadingAvatar->mModifier;
    		else if (mAvatar) //if not, use the previous avatars mod
    			mod = mAvatar->mModifier;

    		// If the new avy is smaller, this'll make sure the old one cleans up
    		AddPositionRectForUpdate();

    		SAFEDELETE(mAvatar);
    		mAvatar = mLoadingAvatar;

    		//Carry the modifier over to the new avatar
    		if (mod != Avatar::MOD_NONE)
    			SetAvatarModifier(mod);

    		_syncAvatarFrameset();
    		UpdateCollisionAndOrigin();
    		PlayAnimation();

    		mLoadingAvatar = NULL;

    	    MessageData md("ENTITY_SWAPAVATAR");
        	md.WriteUserdata("entity", this);
        	messenger.Dispatch(md);

    		return true;
    	}
    }

	return false;
}

void Actor::UpdateCollisionAndOrigin()
{
	rect r;
	r.w = 12;
	r.h = 8;
	if (mAvatar)
	{
		mOrigin.x =	mAvatar->GetImage()->Width() / 2;
		mOrigin.y = mAvatar->GetImage()->Height();
	}
	else
	{
		mOrigin.x = 0;
		mOrigin.y = 0;
	}

	r.x = mOrigin.x - 6;
	r.y = mOrigin.y - 10;

	mCollisionRects.clear();
	mCollisionRects.push_back(r);

	AddPositionRectForUpdate();

	// OPTIMIZETODO: Reload shadow image if we got one
}

bool Actor::CheckLoadingAvatar()
{
	if (mLoadingAvatar)
	{
		switch (mLoadingAvatar->mState)
		{
			case Avatar::LOADED: {
				SwapAvatars();
			} break;
			case Avatar::LOADING: {
				//Don't do anything
			} break;
			case Avatar::BADIMAGE: {
				AvatarError(AVYERR_BADIMAGE);
			} break;
			default: { //FAILED or other
				AvatarError(AVYERR_LOADFAIL);
			} break;
		}
	}

	return (mLoadingAvatar && mLoadingAvatar->mState == Avatar::LOADING);
}

bool Actor::LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, uShort flags)
{
	DEBUGOUT("\\c139* Loading avatar: " + file);

	bool wasLoadingAvatar = (mLoadingAvatar != NULL);

	SAFEDELETE(mLoadingAvatar);

	mLoadingAvatar = new Avatar();

	mLoadingAvatar->mUrl = file;
	mLoadingAvatar->mPass = pass;
	mLoadingAvatar->mWidth = w;
	mLoadingAvatar->mHeight = h;
	mLoadingAvatar->mDelay = delay;
	mLoadingAvatar->mFlags = flags;

	mLoadingAvatar->Load();

    if (!mCheckLoadingAvatarTimer)
        mCheckLoadingAvatarTimer = timers->Add("avacheck",
                    					500, false,
                    					timer_checkLoadingAvatar,
                    					NULL,
                    					this);

    MessageData md("ENTITY_LOADAVATAR");
	md.WriteUserdata("entity", this);
	md.WriteString("file", file);
	md.WriteInt("replaced", wasLoadingAvatar);
	messenger.Dispatch(md);

	return true;
}

void Actor::AvatarError(int err)
{
	console->AddMessage(GetName() + " Avatar Error: " + its(err));
	SAFEDELETE(mLoadingAvatar);

	MessageData md("ENTITY_BADAVATAR");
    md.WriteUserdata("entity", this);
    md.WriteInt("id", err);
    messenger.Dispatch(md);
}

void Actor::Render()
{
	ASSERT(mMap);

	Font* f = fonts->Get("", 0, TTF_STYLE_BOLD);
	Image* scr = Screen::Instance();
	rect r;

	//if we're not in a ghost mode, render a shadow
	if (!mAvatar || mAvatar->mModifier != Avatar::MOD_GHOST)
		RenderShadow();

	//render mAvatar
	if (mAvatar && mAvatar->GetImage())
	{
		r = GetBoundingRect();
		if (!IsPositionRelativeToScreen())
			r = mMap->ToScreenPosition( r );

		mAvatar->GetImage()->Render(scr, r.x, r.y);
	}

	//_doDepthRender();

	//render mName
	/*r = GetBoundingRect();
	if (!IsPositionRelativeToScreen())
		r = mMap->ToScreenPosition( r );

	if ( !mName.empty() && (areRectsIntersecting( r, gui->GetMouseRect() ) || mMap->mShowPlayerNames) )
	{
		f->Render(scr,
					r.x + r.w / 2 - (f->GetWidth(stripCodes(mName)) / 2),
					r.y - (f->GetHeight() + 2),
					mName, color(255,255,255));
	}
	*/

	Entity::Render();
}

void Actor::_doDepthRender()
{
	if (IsPositionRelativeToScreen()) // Don't worry about for screen relative entities
		return;

	rect dst;
	rect src;
	StaticObject* o;
	for (int i = 0; i < mMap->mEntities.size(); ++i)
	{
		if ( mMap->mEntities.at(i) == this )
			break;

		//If we need to do depth rendering on this object, do it.
		if ( mMap->mEntities.at(i)->mType == ENTITY_STATICOBJECT
			&& areRectsIntersecting(mMap->mEntities.at(i)->GetBoundingRect(), GetBoundingRect()) )
		{
			o = (StaticObject*)mMap->mEntities.at(i);
			if (o->mDepth > 0)
			{
				src = GetBoundingRect();
				dst.x = src.x;
				dst.y = mPosition.y - o->mDepth;

				src.x = dst.x - o->mPosition.x;
				src.y = dst.y - o->mPosition.y;
				src.h = o->mDepth + src.w / 10; //shadow is BoundingRect().w / 8, and only half a shadow below our rect, so / 10.

				dst = mMap->ToScreenPosition(dst);

				o->mImage->Render(Screen::Instance(), dst.x, dst.y, src);
			}
		}
	}
}

void Actor::Jump(byte type)
{
	SetAction(IDLE);
	mJumpType = type;
	mJumpHeight = 0;
	mCustomVelocity = 0;
	mFalling = false;

	mPreviousPosition = mPosition;

	MessageData md("ENTITY_JUMP");
	md.WriteUserdata("entity", this);
	messenger.Dispatch(md);
}

void Actor::Fall(int height, int velocity)
{
	SetAction(IDLE);
	mJumpType = CUSTOM_JUMP;
	mJumpHeight = height;
	mCustomVelocity = velocity;
	mFalling = true;

	mPreviousPosition = mPosition;
}

/**	Will rise the actor to a height of 1000. Used for a flying-off-map kind of thing,
	and requires a custom Fall() to be done afterwards. No event will be triggered at max rise
*/
void Actor::Rise(int velocity)
{
	SetAction(IDLE);
	mJumpType = CUSTOM_JUMP;
	mJumpHeight = 0;
	mCustomVelocity = velocity;
}

bool Actor::IsJumping() const
{
	//console->AddMessage("Falling: " + its(mFalling) + " Height: " + its(mJumpHeight));
	return (!mFalling || mJumpHeight > 0);
}

void Actor::Land() //code to run after we land from a jump
{
	mFalling = true;
	mJumpHeight = 0;

	MessageData md("ENTITY_LAND");
	md.WriteUserdata("entity", this);
	messenger.Dispatch(md);
}

//Variables to tweak for jumping
const uShort STANDING_JUMP_HEIGHT = 30;
const uShort STANDING_JUMP_VELOCITY = 5;

//Gravity affectors.. Height should change

const uShort WALKING_JUMP_HEIGHT = 35;
const uShort WALKING_JUMP_VELOCITY = 5;

const uShort RUNNING_JUMP_HEIGHT = 40;
const uShort RUNNING_JUMP_VELOCITY = 5;

void Actor::_processJump()
{
	if (!IsJumping() || !mMap) return;

	int yVelocity;
	int maxHeight;

	/*if (mMap->GetGravity() == 0) //not 0g. Just.. don't jump.
	{
		mFalling = true;
		mJumpHeight = 0;
		return;
	}
	*/

	switch (mJumpType)
	{
		case STANDING_JUMP:
			yVelocity = STANDING_JUMP_VELOCITY;
			maxHeight = STANDING_JUMP_HEIGHT; // * mMap->GetGravity();
			break;
		case WALKING_JUMP:
			yVelocity = WALKING_JUMP_VELOCITY;
			maxHeight = WALKING_JUMP_HEIGHT; // * mMap->GetGravity();
			break;
		case RUNNING_JUMP:
			yVelocity = RUNNING_JUMP_VELOCITY;
			maxHeight = RUNNING_JUMP_HEIGHT; // * mMap->GetGravity();
			break;
		case CUSTOM_JUMP:
			maxHeight = 1000;
			yVelocity = mCustomVelocity;
			break;
		default: break;
	}

	if (!mFalling)
	{
		mJumpHeight += yVelocity;
		if (mJumpHeight >= maxHeight)
		{
			mFalling = true;
		}
	}
	else
	{
		mJumpHeight -= yVelocity;
		if (mJumpHeight <= 0)
		{
			Land();
			return;
		}
	}

	if (mJumpType != STANDING_JUMP && mJumpType != CUSTOM_JUMP) //no moving while standing jump
	{
		//don't offset destination more if we're still moving toward a destination point
		if (mDestination.x != mPosition.x || mDestination.y != mPosition.y)
			return;

		uShort stepDistance = 16; //(WALKING_JUMP) ? WALKING_JUMP_STEP : RUNNING_JUMP_STEP;
		uShort moveSpeed = (mJumpType == WALKING_JUMP) ? SPEED_WALK : SPEED_RUN;

		//we can change our jump height, but if we can't move in this direction, don't do it.

		//TODO: This'll screw up if our client has a solid object on the map where other clients do not. (Moveable map objects and such)
		if (!CanMove(mDirection, stepDistance))
		{
			_dispatchEntityMove();
			return;
		}

		//move our character
		Move(mDirection, stepDistance, moveSpeed);
	}
}

/*	Change direction to face the other entity */
void Actor::Face(Entity* e)
{
	point2d p = GetPosition();
	point2d pp = e->GetPosition();
	direction dir;

	double dx = pp.x - p.x;
	double dy = pp.y - p.y;
	int theta = (int)atan2( dx, dy );

	if (theta == 1)
		dir = EAST;
	else if (theta == -1)
		dir = WEST;
	else if (theta == 0)
		dir = SOUTH;
	else //2, -3, -2
		dir = NORTH;

	SetDirection(dir);
}

/*	index - Index of the stack where our new value for the property should be */
int Actor::LuaSetProp(lua_State* ls, string& prop, int index)
{
	if (prop == "direction") SetDirection( stringToDirection(lua_tostring(ls, index)) );
	else if (prop == "movespeed") SetSpeed( (byte)lua_tonumber(ls, index) );
	else if (prop == "action") SetAction( (byte)lua_tonumber(ls, index) );
	else if (prop == "noclip") SetIgnoreSolids( lua_toboolean(ls, index) );
	else if (prop == "mod" && GetAvatar()) SetAvatarModifier( (int)lua_tonumber(ls, index) );
	else if (prop == "zheight") //forces them to fall from a certain height
	{
		mJumpHeight = (int)lua_tonumber(ls, index);
		mJumpType = STANDING_JUMP;
	}

	else return Entity::LuaSetProp(ls, prop, index);

	return 1;
}

int Actor::LuaGetProp(lua_State* ls, string& prop)
{
	if (prop == "direction") lua_pushnumber( ls, GetDirection() );
	else if (prop == "movespeed") lua_pushnumber( ls, GetSpeed() );
	else if (prop == "action") lua_pushnumber( ls, GetAction() );
	else if (prop == "noclip") lua_pushboolean( ls, IgnoreSolids() );
	else if (prop == "mod" && GetAvatar()) lua_pushnumber( ls, GetAvatar()->mModifier );
	else if (prop == "loadingavatar") lua_pushboolean( ls, mLoadingAvatar != NULL );
	else return Entity::LuaGetProp(ls, prop);

	return 1;
}

void Actor::Emote(int num)
{
	ChatBubble* cb = new ChatBubble(this, num);
	cb->mMap = mMap;
	mMap->AddEntity(cb);
}

void Actor::SetAvatarModifier(int mod)
{
	UpdateCollisionAndOrigin();
	if (GetAvatar())
		GetAvatar()->Modify(mod);
	UpdateCollisionAndOrigin();
}
