
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


#include "ActorNameTag.h"

uShort timer_updateActorNameTag(timer* t, uLong ms)
{
	ActorNameTag* ant = (ActorNameTag*)t->userData;
	
	if (ant)
	{
		ant->Update();
		return TIMER_CONTINUE;
	}
	
	return TIMER_DESTROY;
}


ActorNameTag::ActorNameTag(Actor* owner)
	: TextObject()
{
	timers->Add("", 10, true, timer_updateActorNameTag, NULL, this);
}

ActorNameTag::~ActorNameTag()
{

}

void ActorNameTag::Render()
{
	if (mOwner && !mOwner->mActiveChatBubble)
	{
		TextObject::Render();
	}
}


void ActorNameTag::Update()
{
	rect r;
	rect rr;
	
	// reposition self relative to owner
	if (mOwner)
	{
		if (mText != mOwner->mName)
		{	
			mText = mOwner->mName;
		}
	
		r = mOwner->GetBoundingRect();
		rr = GetBoundingRect();
		
		r.y -= rr.h + 2;
		r.x += r.w / 2 - (rr.w / 2);

		SetPosition(point2d(r.x, r.y));
	}
}


