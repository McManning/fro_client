
#include "ChatBubble.h"
#include "../core/SDL/SDL_rotozoom.h"
#include "../map/Map.h"

const int MAX_CHAT_BUBBLE_WIDTH = 300;

uShort timer_destroyChatBubble(timer* t, uLong ms)
{
	ChatBubble* cb = (ChatBubble*)t->userData;
	
	if (cb)
		cb->mMap->RemoveEntity(cb);

	return TIMER_DESTROY;
}

uShort timer_updateChatBubble(timer* t, uLong ms)
{
	ChatBubble* cb = (ChatBubble*)t->userData;
	
	if (cb)
	{
		cb->UpdatePosition();
		return TIMER_CONTINUE;
	}
	
	return TIMER_DESTROY;
}

ChatBubble::ChatBubble(Entity* owner, string& msg)
{
	mOwner = owner;
	mImage = NULL;
	
	Create(msg);
	
	if (mOwner)
	{
		mOwner->ClearActiveChatBubble();
		mOwner->mActiveChatBubble = this;
		SetLayer(mOwner->GetLayer() + 1);	
		
		rect r = mOwner->GetBoundingRect();

		r.y -= mImage->Height() - 2;
		r.w = mImage->Width();
		r.h = mImage->Height();
		r.x = mOwner->mPosition.x - (r.w / 2);

		SetPosition(point2d(r.x, r.y));
	}
}

ChatBubble::~ChatBubble()
{
	resman->Unload(mImage);

	if (mOwner)
		mOwner->mActiveChatBubble = NULL;
}

void ChatBubble::Render()
{
	ASSERT(mMap);
	ASSERT(mImage);
	rect r;

	Image* scr = Screen::Instance();
	r = GetBoundingRect();

	if (!IsPositionRelativeToScreen())
		r = mMap->ToScreenPosition( r );

	mImage->Render(scr, r.x, r.y);
}

void ChatBubble::UpdatePosition()
{
	// reposition self relative to owner
	if (mOwner)
	{
		r = mOwner->GetBoundingRect();

		r.y -= mImage->Height() + 2;
		r.w = mImage->Width();
		r.h = mImage->Height();
		r.x = mOwner->mPosition.x - (r.w / 2);

		SetPosition(point2d(r.x, r.y));
	}
}

void ChatBubble::Create(string& msg)
{
	string strippedMsg = stripCodes(msg);
	
	Font* font = fonts->Get();
	Image* bgimg;

	rect r = font->GetTextRect(strippedMsg, false, MAX_CHAT_BUBBLE_WIDTH - 8);
	
	r.w += 8;
	r.h += 8;

	mImage = resman->NewImage(r.w, r.h + 3, color(255,0,255), false); //leave room in height for the knob

	// Don't need to use the slower renderer, because NewImage() created a surface without an alpha channel
	
	bgimg = resman->LoadImg("assets/bubble.png");

	bgimg->RenderBox( mImage, rect(0,0,7,7), r ); 
	bgimg->Render( mImage, (r.w / 2) - 3, r.h-3, rect(0, 21, 7, 7) );
	
	resman->Unload(bgimg);
	
	//Draw the text
	if (r.h == font->GetHeight())
		font->Render( mImage, 4, 4, strippedMsg, color(0, 0, 0));	
	else
		font->Render( mImage, 4, 4, strippedMsg, color(0, 0, 0), r.w - 8);	

	//By converting to alpha here, we can do other special manipulators
	mImage->ConvertToAlphaFormat();

	mImage->Rotate(rnd(-5,5), 1, 1);
	
	timers->Add("", (strippedMsg.length() * 30) + 4000, false, timer_destroyChatBubble, NULL, this);
	timers->Add("", CHAT_BUBBLE_UPDATE_MS, true, timer_updateChatBubble, NULL, this);
	
	
	//Dispatch a say message
	MessageData md("ENTITY_SAY");
	md.WriteUserdata("entity", mOwner);
	md.WriteString("message", msg);
	messenger.Dispatch(md, NULL);
}

rect ChatBubble::GetBoundingRect()
{
	if (!mImage)
		return rect();

	return rect( mPosition.x - mOrigin.x,
				mPosition.y - mOrigin.y,
				mImage->Width(), mImage->Height() );
}


