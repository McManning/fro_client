
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


#include "ActionEffect.h"
#include "../map/Map.h"

uShort timer_ActionEffect(timer* t, uLong ms)
{
	ActionEffect* a = (ActionEffect*)t->userData;
	if (!a)
		return TIMER_DESTROY;

	if ( !a->Process(ms) ) //we're done, cleanup
	{
		Map* m = a->mMap;
		t->userData = NULL;	
		m->RemoveEntity(a);
		return TIMER_DESTROY;
	}
	
	return TIMER_CONTINUE;
}

ActionEffect::ActionEffect()
	: StaticObject()
{
	mType = ENTITY_EFFECT;
	
	mFade = false;
	mDisplayMs = 0;	
	SetAA(true);
}

ActionEffect::~ActionEffect()
{
	timers->RemoveMatchingUserData(this);
}

ActionEffect* ActionEffect::Clone()
{
	ASSERT(mMap);
	
	ActionEffect* a = new ActionEffect;

	//Will create another effect that started how we did. NOT an exact clone. 
	a->Create(mStartPosition, mOrigin.y, mEffectId, mEffect, mDisplayMs);
	a->mMap = mMap;
	a->SetLayer(GetLayer());
	
	mMap->AddEntity(a);
	
	return a;
}

void ActionEffect::Create(point2d pos, sShort h, string id, effectType effect, uShort ms)
{
	mStartPosition = pos;
	SetPosition(pos);
	
	LoadImage("assets/actions/" + id + ".png");
	
	if (mImage)
	{
		mOrigin.x = mImage->Width() / 2;
		mOrigin.y = mImage->Height();
	}
	
	mOrigin.y += h;
	mEffectId = id;
	mEffect = effect;
	mDisplayMs = ms;

	uShort interval = 50;
	
	mStartMs = gui->GetTick();
	
	timers->Add("", interval, false, timer_ActionEffect, NULL, this);
}

/*	Return false if it's time to delete this effect */
bool ActionEffect::Process(uLong ms)
{
	//if it's our time to die, do so.
	if (ms - mStartMs > mDisplayMs)
		return false;
	
	switch (mEffect)
	{
		case RISE:
			mOrigin.y += 3;
			break;
		case GROW:
			Rotozoom(0.0, mScale + 0.1);
			break;
		default: break;
	}
	
	return true;
}
