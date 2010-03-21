

#ifndef _WEAPON_H_
#define _WEAPON_H_

#include "../core/Core.h"

/*	Weapon - Abstract base class
		-Attached to actors.
		-Renderable
		-Has a Use() command to perform initial use.
		-Has an Idle state that defines how it's rendered when not being used
		-Has an attack state.
		-Rendering will change based on our players actions, directions, etc.
*/
class Actor;
class Weapon
{
  public:
	Weapon() { mOwner = NULL; mState = IDLE; };
	virtual ~Weapon() {};
	
	//Called pre Actor::Render
	virtual void RenderUnder() = 0;
	
	//Called post Actor::Render
	virtual void RenderOver() = 0;
	
	//Called when the actor decides to attack
	virtual void Use() = 0;

	typedef enum
	{
		IDLE = 0,
		ATTACKING
	} weaponState;
	
	weaponState mState;
	Actor* mOwner;
};

/*	A generic sword weapon. Swords will be hidden when mState == IDLE, and 
	will perform a slashing move when Use() is called.
*/
class Sword : public Weapon
{
  public:
	Sword();
	~Sword();
	
	//Called pre Actor::Render
	void RenderUnder();
	
	//Called post Actor::Render
	void RenderOver();
	
	void Use();

	// Continue to swing our sword. If this returns true, the attack has ended.
	bool Update();
	
	//Ran once the attack sequence is complete
	void Finish();

	Image* mRenderedImage; //the rotated/manipulated version of mImage
	Image* mImage;
	
	//animates our swing
	timer* mAttackTimer;
	
	sShort mDegree; //current swing degree
	uShort mSpeed; //speed of the swing
	uShort mDistance; //distance the handle is from the player
	
	//Range of how far our sword will swing
	sShort mStartDegree;
	sShort mEndDegree;
	
  protected:	
	void _renderBlade();
	
};

#endif //_WEAPON_H_

