
#include <lua.hpp>
#include "Actor.h"
#include "StaticObject.h"
#include "LocalActor.h"
#include "../map/Map.h"
#include "../entity/Avatar.h"
#include "../core/io/FileIO.h"
#include "../game/GameManager.h"

uShort timer_processMovement(timer* t, uLong ms)
{
	Actor* m = (Actor*)t->userData;
	if (m)
	{
		m->ProcessMovement();
		return TIMER_CONTINUE;
	}

	return TIMER_DESTROY;
}

uShort timer_destroyEmote(timer* t, uLong ms)
{
	Actor* a = (Actor*)t->userData;

	if (a)
	{
		resman->Unload(a->mEmoticon);
		a->mEmoticon = NULL;
	}

	return TIMER_DESTROY;
}

uShort timer_moveEmote(timer* t, uLong ms)
{
	Actor* a = (Actor*)t->userData;

	if (!a)
		return TIMER_DESTROY;

	if (a->mEmoteOffset - 10 < 0)
	{
		a->mEmoteOffset = 0;
		return TIMER_DESTROY;
	}
	
	a->mEmoteOffset -= 10;
	return TIMER_CONTINUE;
}

Actor::Actor()
	: Entity()
{
	mAvatar = NULL;
	mLoadingAvatar = NULL;
	mEmoticon = NULL;
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
	resman->Unload(mEmoticon);

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
	r.y = mPosition.y - r.h;

	return r;
}

Image* Actor::GetImage()
{
	Avatar* a = GetAvatar();
	
	if (a)
		return a->GetImage();
		
	return NULL;	
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
		mAvatar->GetImage()->Stop(); //from here on, animation will be manually changed for each step
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
		mAvatar->GetImage()->Stop(); //from here on, animation will be manually changed for each step
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
			mAvatar->GetImage()->Reset();
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

/*	switch (GetAction())
	{
		case IDLE:
			if (mAvatar->mLoopStand)
				mAvatar->GetImage()->Play();
			else
				mAvatar->GetImage()->Stop();
			break;
		case SIT:
			if (mAvatar->mLoopSit)
				mAvatar->GetImage()->Play();
			else
				mAvatar->GetImage()->Stop();
			break;
		default: break;	
	}*/
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
	bool result;

	if (!mLoadingAvatar)
		return false;
	
	result = mLoadingAvatar->Convert();
	
	if (!result 
		|| (mLoadingAvatar->GetImage()->Width() > MAX_AVATAR_WIDTH && mLimitedAvatarSize)
		|| (mLoadingAvatar->GetImage()->Height() > MAX_AVATAR_HEIGHT && mLimitedAvatarSize))
	{
		DEBUGOUT(mName + " BAD SIZE OR " + its(result));
		SAFEDELETE(mLoadingAvatar);
	}
	else
	{
		byte mod = Avatar::MOD_NONE;

		//if our loading avatar decided to set a custom mod, set it
		if (mLoadingAvatar->mModifier != Avatar::MOD_NONE)
			mod = mLoadingAvatar->mModifier;
		else if (mAvatar) //if not, use the previous avatars mod
			mod = mAvatar->mModifier;
			
		SAFEDELETE(mAvatar);
		mAvatar = mLoadingAvatar;
			
		//Carry the modifier over to the new avatar
		if (mod != Avatar::MOD_NONE)
			mAvatar->Modify(mod);

		_syncAvatarFrameset();
		UpdateCollisionAndOrigin();
	}

	mLoadingAvatar = NULL;
	return result;
}

void Actor::UpdateCollisionAndOrigin()
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
	
	r.x = mOrigin.x - 8;
	r.y = mOrigin.y - 16;
	
	mCollisionRects.clear();
	mCollisionRects.push_back(r);	
}

void Actor::_checkLoadingAvatar()
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
			default: { //FAILED or "other"
				SAFEDELETE(mLoadingAvatar);
			} break;
		}
	}
}

bool Actor::LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, 
								bool loopStand, bool loopSit)
{
	DEBUGOUT("\\c139* Loading avatar: " + file);
		
	SAFEDELETE(mLoadingAvatar);
	mLoadingAvatar = new Avatar();
	
	mLoadingAvatar->mUrl = file;
	mLoadingAvatar->mPass = pass;
	mLoadingAvatar->mWidth = w;
	mLoadingAvatar->mHeight = h;
	mLoadingAvatar->mDelay = delay;
	mLoadingAvatar->mLoopStand = loopStand;
	mLoadingAvatar->mLoopSit = loopSit;
	
	int oldcap = downloader->GetByteCap();

	//rig it so that there's a tighter limit on avatar filesizes
	if (mLimitedAvatarSize)
		downloader->SetByteCap(MAX_AVATAR_FILESIZE);

	mLoadingAvatar->Load();
	downloader->SetByteCap(oldcap);

	bool result = (mLoadingAvatar != NULL);
	
	//TODO: If it's a map actor, and the file is from disk, just load it.
	
	// If it loads from disk (avy://, etc), we'll swap here.
	// Screws up LocalActor::LoadAvatar()	 _checkLoadingAvatar();

	return result;
}

void Actor::Render()
{
	ASSERT(mMap);

	Font* f = fonts->Get("", 0, TTF_STYLE_BOLD);
	Image* scr = Screen::Instance();
	rect r;
	
	_checkLoadingAvatar();

	//if we're not visible, don't render the below stuff
	if (!mMap->IsRectInCamera( GetBoundingRect() ))
		return;

	//if we're not in a ghost mode, render a shadow
	if (!mAvatar || mAvatar->mModifier != Avatar::MOD_GHOST)
		RenderShadow();	

	//render loading icon if we're loading an avatar
	if (mLoadingAvatar && mMap->mLoadingAvatarIcon)
	{
		r.w = mMap->mLoadingAvatarIcon->Width();
		r.h = mMap->mLoadingAvatarIcon->Height();
		r.x = mPosition.x - (r.w / 2);
		r.y = mPosition.y - (r.h / 2) - 4;
		r = mMap->ToScreenPosition( r );

		mMap->mLoadingAvatarIcon->Render(scr, r.x, r.y);
	}

	//render mAvatar
	if (mAvatar && mAvatar->GetImage())
	{
		r = mMap->ToScreenPosition( GetBoundingRect() );
		r.y -= mJumpHeight; //calculate in jump

		mAvatar->GetImage()->Render(scr, r.x, r.y);
	}
	
	_doDepthRender();

	//render mName
	r = mMap->ToScreenPosition( GetBoundingRect() );

	if ( !mName.empty() && (areRectsIntersecting( r, gui->GetMouseRect() ) || mMap->mShowPlayerNames) )
	{
		f->Render(scr, 
					r.x + r.w / 2 - (f->GetWidth(stripCodes(mName)) / 2), 
					r.y - (f->GetHeight() + 2) - mJumpHeight, 
					mName, color(255,255,255));
	}

	RenderEmote();
}

void Actor::_doDepthRender()
{
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

void Actor::RenderEmote()
{
	if (!mEmoticon || mEmoteOffset == mEmoticon->Height())
		return;
		
	Image* scr = Screen::Instance();
	rect r;
	
	r = mMap->ToScreenPosition( GetBoundingRect() );
	r.y -= mJumpHeight; //calculate in jump
	
	r.x = r.x + r.w / 2 - (mEmoticon->Width() / 2);
	r.y -= mEmoticon->Height() - mEmoteOffset;

	rect clip(0, 0, mEmoticon->Width(), mEmoticon->Height() - mEmoteOffset);
	
	mEmoticon->Render(scr, r.x, r.y, clip);
}

void Actor::Emote(uShort num)
{
	string file = "assets/emoticons/" + its(num) + ".*"; //wildcard extension
	file = getRandomFileMatchingPattern(file);

	if (file.empty()) //check for invalid emote
		return;

	file = "assets/emoticons/" + file; //append directory since getRandomFile doesn't do that

	if (mEmoticon)
	{
		resman->Unload(mEmoticon);
		timers->Remove("emote", this);
		timers->Remove("emoMove", this);
	}
	
	mEmoticon = resman->LoadImg(file);
	if (mEmoticon)
	{
		ASSERT(mMap);
		
		//kill the bubble so it won't get in the way
		mMap->mBubbles.PopBubble(this);
		
		timers->Add("emote", EMOTE_DISPLAY_MS, false, timer_destroyEmote, NULL, this);
		timers->Add("emoMove", EMOTE_MOVE_DELAY, false, timer_moveEmote, NULL, this);
		mEmoteOffset = mEmoticon->Height();
	}
}

void Actor::Jump(byte type)
{
	SetAction(IDLE);
	mJumpType = type;
	mJumpHeight = 0;
	mFalling = false;
	
	mPreviousPosition = mPosition;
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

	uShort yVelocity;
	uShort maxHeight;

	if (mMap->GetGravity() == 0) //not 0g. Just.. don't jump.
	{
		mFalling = true;
		mJumpHeight = 0;
		return; 
	}
	
	switch (mJumpType)
	{
		case STANDING_JUMP:
			yVelocity = STANDING_JUMP_VELOCITY;
			maxHeight = STANDING_JUMP_HEIGHT * mMap->GetGravity();
			break;
		case WALKING_JUMP:
			yVelocity = WALKING_JUMP_VELOCITY;
			maxHeight = WALKING_JUMP_HEIGHT * mMap->GetGravity();
			break;
		case RUNNING_JUMP: 
			yVelocity = RUNNING_JUMP_VELOCITY;
			maxHeight = RUNNING_JUMP_HEIGHT * mMap->GetGravity();
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
	
	if (mJumpType != STANDING_JUMP) //no moving while standing jump
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
/*
	for (direction dir = SOUTHWEST; dir <= NORTHEAST; dir++)
	{
		if (dir == 5) //invalid direction
			continue;
			
		theta = directionToAngle(dir);
		
		if (isPointInPie(pp, 0, p.x, p.y, theta - 45, theta + 45))
		{
			SetDirection(dir);
			return;
		}
	}
	*/
	//Not working, so let's use the ghetto technique. TODO: Fix

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
	else if (prop == "speed") SetSpeed( (byte)lua_tonumber(ls, index) );
	else if (prop == "action") SetAction( (byte)lua_tonumber(ls, index) );
	else if (prop == "noclip") SetIgnoreSolids( lua_toboolean(ls, index) );
	else if (prop == "mod" && GetAvatar())
	{
		if ( GetAvatar()->Modify( (byte)lua_tonumber(ls, index) ) )
		{
			//if we modified our local players avatar, we need to send this mod to the network
			if (this == (Actor*)game->mPlayer)
				game->mPlayer->NetSendAvatarMod();
		}
	}
	
	// Combatant properties
	else if (prop == "level") SetLevel((int)lua_tonumber(ls, index));
	else if (prop == "gene") SetGene((int)lua_tonumber(ls, index));
	else if (prop == "attack") m_iAttack = (int)lua_tonumber(ls, index);
	else if (prop == "defense") m_iDefense = (int)lua_tonumber(ls, index);
	else if (prop == "speed") m_iSpeed = (int)lua_tonumber(ls, index);
	else if (prop == "maxhealth") m_iMaxHealth = (int)lua_tonumber(ls, index);
	else if (prop == "health") m_iCurrentHealth = (int)lua_tonumber(ls, index);
	//else if (prop == "exp") m_iExp = (int)lua_tonumber(ls, index);
	else if (prop == "maxexp") m_iMaxExp = (int)lua_tonumber(ls, index);
	
	else return Entity::LuaSetProp(ls, prop, index);

	return 1;
}

int Actor::LuaGetProp(lua_State* ls, string& prop)
{
	if (prop == "direction") lua_pushnumber( ls, GetDirection() );
	else if (prop == "speed") lua_pushnumber( ls, GetSpeed() );
	else if (prop == "action") lua_pushnumber( ls, GetAction() );
	else if (prop == "noclip") lua_pushboolean( ls, IgnoreSolids() );
	else if (prop == "mod" && GetAvatar()) lua_pushnumber( ls, GetAvatar()->mModifier );
	
	// Combatant properties
	else if (prop == "level") lua_pushnumber(ls, m_iLevel);
	else if (prop == "gene") lua_pushnumber(ls, m_iGene);
	else if (prop == "attack") lua_pushnumber( ls, m_iAttack );
	else if (prop == "defense") lua_pushnumber( ls, m_iDefense );
	else if (prop == "speed") lua_pushnumber( ls, m_iSpeed );
	else if (prop == "maxhealth") lua_pushnumber(ls, m_iMaxHealth);
	else if (prop == "health") lua_pushnumber(ls, m_iCurrentHealth);
	else if (prop == "exp") lua_pushnumber(ls, m_iExp);
	else if (prop == "maxexp") lua_pushnumber(ls, m_iMaxExp);
	
	else return Entity::LuaGetProp(ls, prop);

	return 1;
}

void Actor::TakeDamage(Combatant* attacker, int damage)
{
	m_iCurrentHealth -= damage;
	
	// We died! Trigger an event!
	if (m_iCurrentHealth <= 0)
	{
		m_iCurrentHealth = 0;
		
		MessageData md("ENTITY_DEATH");
		md.WriteUserdata("entity", this);
		md.WriteUserdata("attacker", attacker);
		md.WriteInt("damage", damage);
		messenger.Dispatch(md);
	}
	else 
	{
		MessageData md("ENTITY_HURT");
		md.WriteUserdata("entity", this);
		md.WriteUserdata("attacker", attacker);
		md.WriteInt("damage", damage);
		messenger.Dispatch(md);
	}
}

void Actor::RecalculateStats()
{
	// Send out a request for SOMEONE to recalculate our stats (hopefully picked up by lua)
	// TODO: Should we use this particular method to calculate? Couldn't this be dangerous? 
	//		Yet, still don't want to hard code it.
	MessageData md("ENTITY_RECALC");
	md.WriteUserdata("entity", this);
	messenger.Dispatch(md);
}

void Actor::LevelUp()
{
	++m_iLevel;
	RecalculateStats();
	
	MessageData md("ENTITY_LEVEL");
	md.WriteUserdata("entity", this);
	messenger.Dispatch(md);
}


