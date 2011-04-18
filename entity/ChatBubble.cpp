
#include "ChatBubble.h"
#include "../core/SDL/SDL_rotozoom.h"
#include "../core/io/FileIO.h"
#include "../map/Map.h"

const int MAX_CHAT_BUBBLE_WIDTH = 300;
const int CHAT_BUBBLE_UPDATE_MS = 10;

const int EMOTE_DISPLAY_MS = 7000;
const int EMOTE_MOVE_DELAY = 100;

uShort timer_destroyChatBubble(timer* t, uLong ms)
{
	ChatBubble* cb = (ChatBubble*)t->userData;
	
	if (cb)
		cb->mMap->RemoveEntity(cb);

	return TIMER_DESTROY;
}

uShort timer_updateChatBubbleText(timer* t, uLong ms)
{
	ChatBubble* cb = (ChatBubble*)t->userData;
	
	if (cb)
	{
		cb->UpdatePosition();
		return TIMER_CONTINUE;
	}
	
	return TIMER_DESTROY;
}

uShort timer_updateChatBubbleEmote(timer* t, uLong ms)
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
	mRiseHeight = 0;

	if (mOwner)
	{
		mOwner->ClearActiveChatBubble();
		mOwner->mActiveChatBubble = this;
		SetLayer(mOwner->GetLayer() + 1);	
	}
    
    CreateText(msg);
}

ChatBubble::ChatBubble(Entity* owner, int emote)
{
    mOwner = owner;
	mImage = NULL;
	mRiseHeight = 0;

	if (mOwner)
	{
		mOwner->ClearActiveChatBubble();
		mOwner->mActiveChatBubble = this;
		SetLayer(mOwner->GetLayer() + 1);
	}   
    
    CreateEmote(emote);
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
	rect clip;
	
	if (mRiseHeight > 0)
	{
		Image* scr = Screen::Instance();
		r = GetBoundingRect();
	
		if (!IsPositionRelativeToScreen())
			r = mMap->ToScreenPosition( r );
	
		clip.h = mRiseHeight;
	
		mImage->Render(scr, r.x, r.y, clip);
	}
}

void ChatBubble::UpdatePosition()
{
	rect r;
	
	// reposition self relative to owner
	if (mOwner)
	{
		r = mOwner->GetBoundingRect();

		r.y -= mRiseHeight + 2;
		r.x = mOwner->mPosition.x - (mImage->Width() / 2);

		SetPosition(point2d(r.x, r.y));
		
		if (mRiseHeight < mImage->Height())
		{	
			int speed = mImage->Height() / 20;
			mRiseHeight += (speed > 0) ? speed : 1;
		}
		
		if (mRiseHeight > mImage->Height())
			mRiseHeight = mImage->Height();
	}
}

void ChatBubble::CreateText(string& msg)
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
	timers->Add("", CHAT_BUBBLE_UPDATE_MS, true, timer_updateChatBubbleText, NULL, this);
	
	
	//Dispatch a say message
	MessageData md("ENTITY_SAY");
	md.WriteUserdata("entity", mOwner);
	md.WriteString("message", msg);
	messenger.Dispatch(md, NULL);
}

void ChatBubble::CreateEmote(int id)
{
    string file = "assets/emoticons/" + its(id) + ".*"; //wildcard extension
	file = getRandomFileMatchingPattern(file);

	if (file.empty()) //check for invalid emote
		return;

	file = "assets/emoticons/" + file;

	mImage = resman->LoadImg(file);
	//mImage->Rotate(rnd(-10,10), 1, 1);

	timers->Add("", EMOTE_DISPLAY_MS, false, timer_destroyChatBubble, NULL, this);
	timers->Add("", CHAT_BUBBLE_UPDATE_MS, false, timer_updateChatBubbleEmote, NULL, this);

	//mEmoteOffset = (mImage) ? mImage->Height() : 0;
}

rect ChatBubble::GetBoundingRect()
{
	if (!mImage)
		return rect();

	return rect( mPosition.x - mOrigin.x,
				mPosition.y - mOrigin.y,
				mImage->Width(), mRiseHeight );
}


