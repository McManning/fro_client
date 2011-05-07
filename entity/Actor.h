
#ifndef _ACTOR_H_
#define _ACTOR_H_

#include "Entity.h"
//#include "Combatant.h"

#define SPEED_RUN (8)
#define SPEED_WALK (4)

#define PROCESS_MOVE_INTERVAL 40

/*	Actor is a base class for "living" entities. Basically entities that can act, move, 
	have avatars that change based on their actions, etc. 
 */
class Avatar;
class Actor : public Entity /*, public Combatant */
{
  public:

	enum { //action types
		IDLE = 0,
		SIT,
	};	
		
	enum //jump types
	{
		STANDING_JUMP = 0,
		WALKING_JUMP,
		RUNNING_JUMP,
		CUSTOM_JUMP
	};
	
	// Avatar load error definitions
	enum {
		AVYERR_LOADFAIL = 0,
		AVYERR_BADIMAGE,
		AVYERR_SIZE,
		AVYERR_CONVERT,
	};

	Actor();
	virtual ~Actor();

	rect GetBoundingRect();
	
	void Render();
	
	Image* GetImage();
	
	/*	Load an avatar, convert to avatar format, etc
		if MNG/GIF/other animated format: delay, w, and h are ignored. Otherwise it's a constant
		frame delay used when splitting single image avatars into frames.
		Returns true on success, false otherwise.
	*/
	virtual bool LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, uShort flags);
    
    /** 
        Used to trigger a SwapAvatars() when mLoadingAvatar is finished
        @return true if mLoadingAvatar is still loading. False otherwise.
    */
    bool CheckLoadingAvatar();

	/*	Attempts to convert mLoadingAvatar to avatar format and swaps with mAvatar. Returns true on success */
	bool SwapAvatars();
	
	/*	Called when an error occurs while loading an avatar. Lets inherited classes handle it their way */
	virtual void AvatarError(int err);
	
	/*	index - Index of the stack where our new value for the property should be */
	virtual int LuaSetProp(lua_State* ls, string& prop, int index);
	virtual int LuaGetProp(lua_State* ls, string& prop);

	/*	Move in the designated direction/distance at the designated speed */
	virtual void Move(direction dir, sShort distance = 16, byte speed = 0);
	
	/*	Set mDestination and let pathfinding move us to it */
	virtual void MoveTo(point2d destination, byte speed = 0);
	
	/*	Returns true if this actor can move in the specified direction */
	virtual bool CanMove(direction d, sShort distance);

	/*	Moves Actor close to mDestination */
	virtual bool ProcessMovement();
	
	/*	If we still have data in the action buffer, will process the next action */
	virtual void PostMovement();

	/*	Jump in our current direction, using the specified jump type (running, walking, standing) */
	void Jump(byte type);
	
	/*	Fall down using a custom velocity */
	void Fall(int height, int velocity);
	
	/*	Rise with a custom velocity. Will stop at height 1000. No events triggered */
	void Rise(int velocity);
	
	/*	Code to be ran after a jump */
	virtual void Land();

	/* Returns true if we're currently in a jump */
	bool IsJumping() const;

	/*	Returns true if this entity is moving/jumping (durr) */
	bool IsMoving() const;
	
	void SetPosition(point2d position);

	void SetAction(byte newAction);
	byte GetAction() const { return mAction; };
	
	void SetSpeed(byte newSpeed);
	byte GetSpeed() const { return mSpeed; };
	
	void SetDirection(direction newDir);
	direction GetDirection() const { return mDirection; };

	void SetIgnoreSolids(bool b) { mIgnoreSolids = b; };
	bool IgnoreSolids() const { return mIgnoreSolids; };

	virtual void AddToActionBuffer(string data);
	string GetActionBuffer() const { return mActionBuffer; };

	virtual void ClearActionBuffer() { mActionBuffer.clear(); };

	point2d GetDestination() const { return mDestination; };
	
	Avatar* GetAvatar() const { return mAvatar; };
	
	/* if our avatar changes size, this needs to be called */
	void UpdateCollisionAndOrigin();
	
	/* 	Display the specified emote overhead for EMOTE_DISPLAY_MS. 
		Emote is taken from assets/emoticons/NUM.*, where it supports any file extension.
		Due to the wildcard, there can be multiple versions. Such as 5.a.png, 5.b.png, and it will
		randomly pick one. This allows the addition of variation to various emotes.
	*/
	void Emote(int num);
	
	void SetAvatarModifier(int mod);
	
	void RenderEmote();
	
	/*	Change direction to face the other entity */
	void Face(Entity* e);
	
	/* Inherited from Combatant 
	virtual void LevelUp();
	virtual void RecalculateStats();
	virtual void TakeDamage(Combatant* attacker, int damage);
	*/
	
	bool _animate(); // Called by the mAnimationTimer
	void PlayAnimation();
	void StopAnimation();
	
	timer* mAnimationTimer;
	timer* mCheckLoadingAvatarTimer;

 // protected:
  
	/* 	Based on the current action of our character, change the frameset we are currently playing. 
		Will also reset the current frame we're on, so only call when it needs to be changed. 
	*/
	void _syncAvatarFrameset(); 
	
	/*	Reads action buffer and performs actions based on the oldest commands */
	void _checkActionBuffer();
	
	/*	Recalculate our facing direction based on mDestinations relation to mPosition. (Does NOT change the current frame of mAvatar) */
	void _recalculateDirection();
	
	/*	Recalculate avatar animation step based on previous step and speed */
	void _recalculateStep();
	
	/*	Actually move mPosition toward mDestination */
	void _stepTowardDestination();

	/* Changes our jump height and position based on our jump type */
	void _processJump();
	
	void _dispatchEntityMove();
	
	/*	If we're standing on any objects that have depth, render a chunk of them over our avatar */
	void _doDepthRender();
	
	uShort mSpeed;
	direction mDirection;
	byte mAction;
	byte mStep; //Step counter	
	point2d mDestination;
	
	bool mIgnoreSolids; //does this entity ignore collisions while moving?
	
	//Jumping related
	int mJumpHeight;
	byte mJumpType;
	int mCustomVelocity;
	bool mFalling;
	
	bool mLimitedAvatarSize; //does this actor follow size limit guidelines for avatars
	
	string mActionBuffer;

	Avatar* mAvatar;
	Avatar* mLoadingAvatar; //avatar loading in the background
	
	timer* mMovementTimer;

};

#endif //_ACTOR_H_
