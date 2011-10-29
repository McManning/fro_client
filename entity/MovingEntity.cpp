
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


#include "Actor.h"
#include "../map/Map.h"
#include "../entity/Avatar.h"

uShort timer_processMovement(timer* t, uLong ms)
{
	PRINT("movproc");
	Actor* m = (Actor*)t->userData;
	if (m)
	{
		m->ProcessMovement();
		return TIMER_CONTINUE;
	}

	return TIMER_DESTROY;
}

Actor::Actor()
	: Entity()
{
	mAvatar = NULL;
	mLoadingAvatar = NULL;
	mStep = 0;
	mAction = IDLE;
	mDirection = SOUTH;
	mSpeed = SPEED_WALK;
	
	mMovementTimer = timers->Add("movproc", 
								PROCESS_MOVE_INTERVAL, false,
								timer_processMovement,
								NULL,
								this);
}

Actor::~Actor()
{
	timers->Remove(mMovementTimer);
	SAFEDELETE(mAvatar);
	SAFEDELETE(mLoadingAvatar);
}

Actor& Actor::operator=(const Actor& e)
{
	PRINT("Actor::operator=");

	if (this != &e)
	{
		SetSpeed(e.GetSpeed());
		SetDirection(e.GetDirection());
		SetAction(e.GetAction());
		SetPosition(e.GetPosition());
		
		//Don't copy any current movement information.
		mStep = 0;
		mActionBuffer.clear();

		if (e.mAvatar)
		{
			SAFEDELETE(mAvatar);
			mAvatar = new Avatar;
			*mAvatar = *e.mAvatar;
		}
	
		*((Entity*)this) = e; //Have the entity base copy its members over too
	}
	
	return *this;
}

Actor* Actor::Clone()
{
	PRINT("Actor::Clone");
	Actor* e = new Actor;
	*e = *this;
	return e;
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
	r.y = mPosition.y - r.h;

	return r;
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

	mPreviousPosition = mPosition;

	//WARNING("SETTINGDST " + its(mPosition.x) + "," + its(mPosition.y) + "->"
	//		 + its(mDestination.x) + "," + its(mDestination.y));

	if (mAvatar && mAvatar->GetImage()) 
	{
		_syncAvatarFrameset();
		mAvatar->GetImage()->Stop(); //from here on, animation will be manually changed for each step
	}
	mStep = 0;
}

void Actor::MoveTo(point2d destination, byte speed)
{
	mDestination = destination;

	if (speed != 0) //0 = use mSpeed
		SetSpeed(speed);
}

/*	Offset mPosition toward the specified direction, check if it collides, then reset mPosition and return true if collided with something */
bool Actor::CanMove(direction d)
{
	ASSERT(mMap);

	point2d temp = mPosition;

	rect r;
	r.x = mPosition.x;
	r.y = mPosition.y;
	offsetRectByDirection(&r, d, 16);

	mPosition.x = r.x;
	mPosition.y = r.y;
	
	bool result = true;
	
	if ((r.x >= mMap->mWidth && mMap->mWidth > 0) || r.x < 0)
		result = false;
		
	if ((r.y >= mMap->mHeight && mMap->mHeight > 0) || r.y < 0)
		result = false;
	
	//if we're still on the map, check for entity collisions
	if (result)
		result = !IsCollidingWithSolid();
	
	mPosition = temp; //reset mPosition to it's original

	return result;
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

	PRINT("Actor " + mId + " (" + its(mPosition.x) + "," + its(mPosition.y) + ")->("
			+ its(mDestination.x) + "," + its(mDestination.y) + ")");
			
	_stepTowardDestination();

	PRINT("Recalculating Direction");
	_recalculateDirection();
	
	PRINT("Stepping");
	_recalculateStep();

	/*	If they just reached the target position,
		do whatever is necessary after we're finished	*/
	if (!IsMoving())
	{
		PRINT("Post Movement");
		PostMovement();
		
		//movement caused an order change. For optimization, only resort @ 16 grid.
		mMap->QueueEntityResort();
	}

	PRINT("Done");

	return true;
}

void Actor::PostMovement()
{
	_checkActionBuffer();

	if (!IsMoving())
	{
		if (mAvatar && mAvatar->GetImage()) 
		{
			_syncAvatarFrameset();
			mAvatar->GetImage()->Play();
		}
	}
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
		if (IsMoving())
		{
			if ( !img->SetFrameset("_move_" + dir) )
			{
				img->SetFrameset("_move_2");
			}
		}
		else
		{
			if ( !img->SetFrameset("_stop_" + dir) )
			{
				if ( !img->SetFrameset("_stop_2") && !img->SetFrameset("_move_" + dir) )
				{
					img->SetFrameset("_move_2");
				}
			}
		}
	}
	//TODO: Detect run, check for _RUN_dir, Detect jump, check for _JUMP_dir, Detect stop, check for _STOP_dir
}

void Actor::AddToActionBuffer(string data)
{
	mActionBuffer += data;
}

void Actor::_checkActionBuffer()
{
	if (mActionBuffer.empty()) return;

	char c = mActionBuffer.at(0);
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
	else if (c == 'c') //Compare supplied x/y with position. format: cXXXX.YYYY.
	{
		size_t xend = mActionBuffer.find(".", 0);
		size_t yend = mActionBuffer.find(".", xend+1);

		if (yend == string::npos || xend == string::npos)
		{
			WARNING("npos " + its(xend) + its(yend) + " : " + mActionBuffer);
		}

		sShort x = sti(mActionBuffer.substr(1, xend - 1));
		sShort y = sti(mActionBuffer.substr(xend + 1, yend - xend - 1));

		//If we need to correct position, do so.
		if (mPosition.x != x || mPosition.y != y)
		{
			if (x == 0 && y == 0)
			{
				//PRINT("\\c700 MANTICORE Double Zero Ignore " + mId);
			}
			else
			{
				//PRINT("\\c700 MANTICORE Correcting Position " + mId
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
	return (mDestination.x != mPosition.x || mDestination.y != mPosition.y);	
}

void Actor::SetPosition(point2d position)
{
	Entity::SetPosition(position);
	mDestination = mPosition; //so it won't try to walk to mDestination
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

void Actor::_recalculateDirection() //TODO: Shouldn't this call SetDirection so we can _syncAvatarFrameset() ?
{
	//Recalculate direction
	if (mPosition.y < mDestination.y) //moving south
	{
		if (mPosition.x > mDestination.x)
			mDirection = SOUTHWEST;
		else if (mPosition.x < mDestination.x)
			mDirection = SOUTHEAST;
		else
			mDirection = SOUTH;
	}
	else if (mPosition.y > mDestination.y) //north
	{
		if (mPosition.x > mDestination.x)
			mDirection = NORTHWEST;
		else if (mPosition.x < mDestination.x)
			mDirection = NORTHEAST;
		else
			mDirection = NORTH;
	}
	else //moving on x only
	{
		if (mPosition.x > mDestination.x)
			mDirection = WEST;
		else if (mPosition.x < mDestination.x)
			mDirection = EAST;
	}
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
			mAvatar->GetImage()->Forward(true);
	}
	else if (mStep == 8)
	{
		if (mAvatar && mAvatar->GetImage())
			mAvatar->GetImage()->Forward(true);
		mStep = 0; //reset step
	}
}

void Actor::_stepTowardDestination()
{
	mPreviousPosition = mPosition;

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
}

bool Actor::SwapAvatars()
{
	bool result;
	
	PRINT("Actor::SwapAvatars");
	
	if (!mLoadingAvatar)
		return false;
	
	result = mLoadingAvatar->Convert();
	
	if (!result)
	{
		SAFEDELETE(mLoadingAvatar);
	}
	else
	{
		SAFEDELETE(mAvatar);
		mAvatar = mLoadingAvatar;
		_syncAvatarFrameset();
		_updateCollisionAndOrigin();
	}
	
	PRINT("Actor::SwapAvatars end");
	
	mLoadingAvatar = NULL;
	return result;
}

void Actor::_updateCollisionAndOrigin()
{
	rect r;
	r.w = 16;
	r.h = 16;
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
	
	//r.x = mOrigin.x;
	//r.y = mOrigin.y;
	r.x -= 8;
	r.y -= 16;
	
	mCollisionRects.clear();
	mCollisionRects.push_back(r);	
}

void Actor::_checkLoadingAvatar()
{
	if (mLoadingAvatar)
	{
		ASSERT(mLoadingAvatar->GetImage());
		switch (mLoadingAvatar->GetImage()->mImage->state)
		{
			case SDL_Image::LOADED: {
				SwapAvatars();
			} break;
			case SDL_Image::LOADING: {
				//Don't do anything
			} break;
			case SDL_Image::BADIMAGE: {
				SAFEDELETE(mLoadingAvatar);
			} break;
			default: break;
		}
	}
}

bool Actor::LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, 
								bool loopStand, bool loopSit)
{
	PRINT("Actor::LoadAvatar");
	
	console->AddMessage("\\c009* Loading avatar: " + file);
		
	SAFEDELETE(mLoadingAvatar);
	mLoadingAvatar = new Avatar();
	
	mLoadingAvatar->mUrl = file;
	mLoadingAvatar->mPass = pass;
	mLoadingAvatar->mWidth = w;
	mLoadingAvatar->mHeight = h;
	mLoadingAvatar->mDelay = delay;
	mLoadingAvatar->mLoopStand = loopStand;
	mLoadingAvatar->mLoopSit = loopSit;
	
	mLoadingAvatar->mImage = resman->LoadImg(file, pass);

	PRINT("Actor::LoadAvatar end");
	
	return (mLoadingAvatar != NULL);
}

