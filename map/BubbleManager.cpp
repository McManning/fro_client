
#include "BubbleManager.h"
#include "Map.h"
#include "../entity/Entity.h"
#include "../core/SDL/SDL_rotozoom.h"

BubbleManager::BubbleManager()
{
	mMaxWidth = 300;
	mFont = fonts->Get();
	mImage = resman->LoadImg("assets/bubble.png");
	mMap = NULL;
}

BubbleManager::~BubbleManager()
{
	for (int i = 0; i < mBubbles.size(); i++)
	{
		resman->Unload(mBubbles.at(i)->img);
		delete mBubbles.at(i);
	}
	resman->Unload(mImage);
}

void BubbleManager::Process()
{
	uLong ms = gui->GetTick();
	for (int i = 0; i < mBubbles.size(); i++)
	{
		if (mBubbles.at(i)->killTick < ms)
		{
			resman->Unload(mBubbles.at(i)->img);
			delete mBubbles.at(i);
			mBubbles.erase(mBubbles.begin() + i);
			i--;
		}
	}
}

void BubbleManager::Render()
{
	Image* scr = Screen::Instance();
	Entity* e;
	rect r;
	for (int i = 0; i < mBubbles.size(); i++)
	{
		e = mBubbles.at(i)->owner;
		r = e->GetBoundingRect();
		r.y -= e->mJumpHeight;

		r.y -= mBubbles.at(i)->img->Height() - 2;
		r.w = mBubbles.at(i)->img->Width();
		r.h = mBubbles.at(i)->img->Height();
		r.x = e->mPosition.x - (r.w / 2);

		r = mMap->ToScreenPosition( r );

		mBubbles.at(i)->img->Render(scr, r.x, r.y);
	}
}

bool BubbleManager::CreateBubble(Entity* owner, string msg)
{
	PopBubble(owner);

	if (!mFont || !mImage) return false;
	
	if (msg.find("\\c", 0) != string::npos) 
		msg = stripCodes(msg); //if we got codes, strip em

	uShort width, height;
 
 	vString v;
	mFont->CharacterWrapMessage(v, msg, mMaxWidth - 8);
 
 	if (v.size() < 2)
 	{
		width = mFont->GetWidth(msg) + 8;
		height = mFont->GetHeight() + 8;
	}
	else
	{
		width = mMaxWidth;
		height = v.size() * mFont->GetHeight() + 8;
	}

	Image* img = resman->NewImage(width, height + 3, color(255,0,255), false); //leave room in height for the knob
	//TODO: fix! img->SetGlobalAlpha(220);
	
	//Draw the box and the knob
	mImage->RenderBox( img, rect(0,0,7,7), rect(0, 0, width, height) ); 
	mImage->Render( img, (width / 2) - 3, height-3, rect(0, 21, 7, 7) );

	//Draw the text
	if (v.size() < 2)
		mFont->Render( img, 4, 4, msg, color(0, 0, 0));	
	else
		mFont->Render( img, 4, 4, msg, color(0, 0, 0), width - 8);	

	//build the bubble itself and add
	bubble* b = new bubble;
	b->owner = owner;
	b->img = img;
	b->killTick = gui->GetTick() + (msg.length() * 30) + 4000; //10000;

	//By converting to alpha here, we can do other special manipulators
	b->img->ConvertToAlphaFormat();

	b->img->Rotate(rnd(-5,5), 1, 1);
	
	mBubbles.push_back(b);

	//Dispatch a say message
	MessageData md("ENTITY_SAY");
	md.WriteUserdata("entity", owner);
	md.WriteString("message", msg);
	messenger.Dispatch(md, owner);

	return true;
}

bool BubbleManager::PopBubble(Entity* owner)
{
	for (int i = 0; i < mBubbles.size(); i++)
	{
		if (mBubbles.at(i)->owner == owner)
		{
			resman->Unload(mBubbles.at(i)->img);
			delete mBubbles.at(i);
			mBubbles.erase(mBubbles.begin() + i);
			return true;
		}
	}
	return false;
}

bool BubbleManager::HasBubble(Entity* owner)
{
	for (int i = 0; i < mBubbles.size(); i++)
	{
		if (mBubbles.at(i)->owner == owner)
			return true;
	}
	return false;
}


