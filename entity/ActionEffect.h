
#ifndef _ACTIONEFFECT_H_
#define _ACTIONEFFECT_H_

#include "StaticObject.h"

/*	Comic-like action bubbles that appear when something happens. Such as an 
	explosion, or an attack.
	These will appear at their designated positions, and float upward (or shake, 
	or grow, depending on what type of modifier), and vanish after so long. 
	
	Scriptable via:
		Entity.NewActionIcon(x, y, id, effect, timetolive, fade (1|0))
		Should also be able to include these as resources in the map, storable in the cache. 
		(Custom ones and whatnot)
*/
class ActionEffect : public StaticObject
{
  public:
	ActionEffect();
	~ActionEffect();

	/*	Returns a new ActionEffect cloned from this one. This isn't an exact clone. It'll have the same initial state as set by Create() */
	ActionEffect* Clone();
	
	enum effectType
	{
		RISE = 0, //float upward until death
		SHAKE, //shake until death
		HARD_SHAKE, //shake VIOLENTLY until death
		GROW, //grow larger and larger until death
		SPIN_OFF, //spin off toward a random arching velocity
	};
	
	void Create(point2d pos, sShort h, string id, effectType effect, uShort ms);

	/*	Return false if it's time to delete this effect */
	bool Process(uLong ms);

	bool mFade; //will this fade away?
	uShort mDisplayMs; //how long will this appear?
	uLong mStartMs; //when this was created
	point2d mStartPosition;
	effectType mEffect;
	string mEffectId; //ID code of this particular effect
};

#endif //_ACTIONEFFECT_H_
