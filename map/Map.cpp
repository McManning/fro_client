
#include "Map.h"
#include "../entity/RemoteActor.h"
#include "../entity/LocalActor.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/OpenUrl.h"
#include "../core/widgets/RightClickMenu.h"
#include "../core/io/Crypt.h"
#include "../game/GameManager.h"
#include "../game/ChatCommands.h"
#include "../game/WorldLoader.h"
#include "../lua/MapLib.h"

#include "../interface/AvatarFavorites.h"
#include "../interface/UserList.h"
#include "../interface/OptionsDialog.h"
#include "../interface/AvatarCreator.h"
#include "../interface/WorldViewer.h"

/*	TODO: Relocate these callbacks for the remote player menu */

/*void callback_playerMenuBeat(RightClickMenu* m, void* userdata)
{
	RemoteActor* ra = (RemoteActor*)userdata;
	
	//get random inventory item, beat r->mLinked->mName with it if item != NULL
	if (!ra || !inventory) return;
	
	//Make sure we're not waiting for a beat timer to finish up
	timer* t = timers->Find("rpbeat");
	if (t)
	{
		int seconds = t->lastMs + t->interval - gui->GetTick();
		if (seconds < 0)
			seconds = 0;
		else
			seconds /= 1000;

		game->GetChat()->AddMessage("\\c900 * You must wait " + its(seconds+1) + " seconds.");
		return;
	}

	//if too far, tell them!
	if (getDistance(game->mPlayer->GetPosition(), ra->GetPosition()) > 100)
	{
		game->GetChat()->AddMessage("\\c900 * You're too far to hit your target!");	
		return;
	}

	itemProperties* i = inventory->GetRandom();
	if (i)
	{
		netSendBeat(ra, i->id);
	}
	else
	{
		game->GetChat()->AddMessage("\\c900 * You don't have any items to use!");
	}
	
	//limit the number of times they can use the beat command
	timers->Add("rpbeat", 10 * 1000, false, NULL, NULL, NULL);
}
*/

void callback_playerMenuPrivmsg(RightClickMenu* m, void* userdata)
{
	RemoteActor* ra = (RemoteActor*)userdata;
	
	if (ra)
		game->GetPrivateChat(ra->GetName());
}

/*
void callback_playerMenuTrade(RightClickMenu* m, void* userdata)
{
	RemoteActor* ra = (RemoteActor*)userdata;
	
	if (ra)
		handleOutboundTradeRequest(ra->mName);
}
*/

void callback_playerMenuToggleBlock(RightClickMenu* m, void* userdata)
{
	RemoteActor* ra = (RemoteActor*)userdata;
	
	if (ra)
		ra->SetBlocked(!ra->IsBlocked());
}

void callback_showChatbox(Button* b)
{
	if (game->mMap)
        game->mMap->ShowChat();
}

void callback_hideChatbox(Button* b)
{
    if (game->mMap)
        game->mMap->HideChat();  
}

Map::Map() 
	: Frame(NULL, "", rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT))
{
	mWidth = mHeight = 0; 
	mFollowedEntity = NULL;
	mStopCameraAtMapEdge = true;
	mConstrainChildrenToRect = true;
	mLuaState = NULL;
	mChat = NULL;
	mHud = NULL;
	mEditorMode = false;
	mShowDebug = false;

	CreateChatbox();
	CreateHud();

	mShowPlayerNames = sti(game->mUserData.GetValue("MapSettings", "ShowNames"));
		
	mGravity = 1;
	mCameraSpeed = 4;
	
	if (game->mLoader && game->mLoader->m_bTestMode)
		mWorkingDir = DIR_DEV;
	else
		mWorkingDir = DIR_CACHE;
		
	// TODO: Why is this here???
	gui->RemoveGlobalEventHandler(this);
}

Map::~Map()
{

	//in case this wasn't deleted gracefully
	if (mLuaState)
		mapLib_CloseLuaState(mLuaState);
}

void Map::Render()
{
    Image* scr = Screen::Instance();
	rect r = GetScreenPosition();

	scr->PushClip(r);
	
	//fill with background color
	scr->DrawRect(r, mBackground);

	_renderEntities();

	Frame::Render();
		
	scr->PopClip();
}

void Map::_renderEntities()
{
	Image* scr = Screen::Instance();

	rect r, rr;
	Entity* e;
	color c;
	
	int i;
	for (i = 0; i < mEntities.size(); ++i)
	{
		e = mEntities.at(i);
		
		if (e && e->IsVisibleInCamera())
		{
			e->Render();
		} //if visible & in camera
	} //for all entities
}

void Map::Event(SDL_Event* event)
{
	switch (event->type)
	{
		case SDL_MOUSEBUTTONDOWN:
			if (HasMouseFocus())
			{
				if (event->button.button == SDL_BUTTON_RIGHT)
				{
					HandleRightClick();
				}
				else if (event->button.button == SDL_BUTTON_LEFT)
				{
					HandleLeftClick();	
				}
			}
			break;
		default: break;	
	}
}

void Map::CreateChatbox()
{
    mChat = new Console("chat", "", "chat_", color(255,0,0), true, true, true);
		mChat->mExit->onClickCallback = callback_hideChatbox;
		mChat->mExit->mHoverText = "Hide Chat";
		mChat->mShowTimestamps = sti(game->mUserData.GetValue("MapSettings", "Timestamps"));
		Add(mChat);

	rect r = deserializeRect( game->mUserData.GetValue("MapSettings", "ChatPosition") );

	if (isDefaultRect(r))
	{
		mChat->SetPosition( rect(mPosition.w - mChat->Width(), 
								mPosition.h - mChat->Height(),
								mChat->Width(), mChat->Height()) 
							);
	}
	else
	{
		mChat->SetPosition(r);
	}
	
    hookChatCommands(mChat);
    
    mChat->ResizeChildren();
}

void callback_hudSubButton(Button* b)
{
	switch (b->mId.at(0))
	{
		case 'a': //avatar favorites
			if (!gui->Get("AvatarFavorites"))
				new AvatarFavorites();
			break;
		case 'o': 
			if (!gui->Get("OptionsDialog"))
			{
				OptionsDialog* o = new OptionsDialog();
				o->DemandFocus(true);
			}
			break;
		/*case 'i': //inventory
			ASSERT(inventory);
			inventory->SetVisible(true);
			inventory->MoveToTop();
			break;
		case 'c': //achievements
			if (!gui->Get("achievements"))
				new MyAchievements();
			break;*/
		case 'u': //userlist
			if (!gui->Get("UserList"))
				new UserList();
			break;
		case 'w': //world viewer
			if (!gui->Get("WorldViewer"))
				new WorldViewer();
			break;
		default: break;
	}
}

void Map::CreateHud()
{
	int x = 0, sx = 0;
	Button* b;

	mHud = new Frame(this, "", rect(12,12,0,0));

	b = new Button(mHud, "o", rect(x,0,35,35), "", callback_hudSubButton);
		b->mHoverText = "Options";
		b->SetImage("assets/hud/options.png");
	x += 40;
	sx += 35;

	b = new Button(mHud, "u", rect(x,0,35,35), "", callback_hudSubButton);
		b->mHoverText = "Userlist";
		b->SetImage("assets/hud/userlist.png");
	x += 40;
	sx += 35;

	/*b = new Button(mHud, "c", rect(x,0,35,35), "", callback_hudSubButton);
		b->mHoverText = "My Achievements";
		b->SetImage("assets/hud/achievements.png");
	x += 40;
	sx += 35;*/

	b = new Button(mHud, "a", rect(x,0,35,35), "", callback_hudSubButton);
		b->mHoverText = "My Avatars";
		b->SetImage("assets/hud/avatars.png");
	x += 40;
	sx += 35;
/*	
	b = new Button(mHud, "i", rect(x,0,35,35), "", callback_hudSubButton);
		b->mHoverText = "My Backpack";
		b->SetImage("assets/hud/inventory.png");
	x += 40;
	sx += 35;
*/	
	b = new Button(mHud, "w", rect(x,0,35,35), "", callback_hudSubButton);
		b->mHoverText = "World Viewer";
		b->SetImage("assets/hud/worldviewer.png");
	x += 40;
	sx += 35;


	mHud->SetSize(x, 35);
}

void Map::HandleLeftClick()
{
	if (game->mGameMode == GameManager::MODE_DUEL)
		return;
		
	Entity* e = GetEntityUnderMouse(!mEditorMode, false);
	
	if (e && (getDistance(game->mPlayer->GetPosition(), e->GetPosition()) <= e->mClickRange
			|| e->IsPositionRelativeToScreen()) && !mEditorMode)
	{
		MessageData md("ENTITY_CLICK");
		md.WriteUserdata("entity", e);
		messenger.Dispatch(md, e);
	}
}

void Map::HandleRightClick()
{
	if (!HasMouseFocus() || game->mGameMode == GameManager::MODE_DUEL)
		return;
		
	Entity* e = GetEntityUnderMouse(false, true);

	if (e)
	{
		MessageData md("RIGHT_CLICK_ACTOR");
		md.WriteUserdata("entity", e);
		md.WriteInt("userlist", 0);
		messenger.Dispatch(md, e);	
	}
}

/*	Will attempt to return the entity directly under the mouse, if it is clickable. */
Entity* Map::GetEntityUnderMouse(bool mustBeClickable, bool actorsOnly)
{
	return GetNextEntityUnderMouse(NULL, mustBeClickable, actorsOnly);
}

// Will return an entity intersecting the screen point, that is lower in the list than the specified entity
// Or NULL, if there are none.
Entity* Map::GetNextEntityUnderMouse(Entity* start, bool mustBeClickable, bool actorsOnly)
{
	int x, y;
	rect r;
	Entity* e;
	Image* img;
	
	bool foundStart = false;
	
	//for all entities, highest to lowest
	for (int i = mEntities.size()-1; i > -1; --i)
	//for (int i = 0; i < mEntities.size(); ++i)
	{
		e = mEntities.at(i);
			
		if (e == start || !start)
		{
			foundStart = true;
		}
		
		if (foundStart && e && e != start && e->IsVisibleInCamera() )
		{
			if (mustBeClickable && e->mClickRange < 1)
				continue;
				
			if (actorsOnly && !(e->mType >= ENTITY_ACTOR && e->mType < ENTITY_END_ACTORS))
				continue;
			
			r = e->GetBoundingRect();
			
			if (!e->IsPositionRelativeToScreen())
				r = ToScreenPosition(r);
			
			if ( areRectsIntersecting(gui->GetMouseRect(), r) )
			{
				//Otherwise, if the mouse is touching a non transparent pixel
				x = gui->GetMouseX() - r.x;
				y = gui->GetMouseY() - r.y;
				
				img = e->GetImage();
				if (img)
				{
					//if non transparent pixel, we found our entity. 
					if (!img->IsPixelTransparent(x, y))
						return e;
				}
			}
		}
	}
	
	return NULL;
}

void Map::Process()
{
	UpdateCamera();
	ResortEntities();
	CheckForClickableEntity();
}

/**
	If the entity under our mouse is clickable, change our system cursor.
	Otherwise, use default.
	Possible modes:
		1. clickable, and in range. 
		2. clickable, but not in range
		3. not clickable (default)
	The reason this is called from Process() is because entities could move 
	around, and screw up the thing. Unfortunately, this is a CPU hog, and 
	there's an obviously more efficient way to do it, but.. laziness.. 
	it'll be the death of this project.
	TODO: Optimize!
*/
void Map::CheckForClickableEntity()
{
	int y = 0;
	
	if (HasMouseFocus())
	{
		Entity* e = GetEntityUnderMouse(true, false);
		if (e)
		{
			if (getDistance(game->mPlayer->GetPosition(), e->GetPosition()) <= e->mClickRange
				|| e->IsPositionRelativeToScreen())
				y = 22;
			else //not in range
				y = 44;
		}
	}
	gui->mCustomCursorSourceY = y;
}

/*	A graceful cleanup was issued. Do some special cleanup */
void Map::Die()
{
	//tell our primary script we're going to die, before we do.
	mapLib_CloseLuaState(mLuaState);
	mLuaState = NULL;
	
	FlushEntities();

	if (mChat)
	{
	   game->mUserData.SetValue("MapSettings", "ChatPosition", serializeRect(mChat->GetPosition()));
    }
    
	Widget::Die();
}

bool Map::IsRectBlocked(rect r)
{
	Entity* e;
	for (int i = 0; i < mEntities.size(); ++i)
	{
		e = mEntities.at(i);
		if ( e && e->IsSolid() && e->CollidesWith(r) && e->mType != ENTITY_LOCALACTOR )
			return true;
	} 
	return false;	
}

void Map::ResizeChildren()
{

}

void Map::SetCameraPosition(point2d p, bool centered)
{
	mCameraPosition = p;
	if (centered) //Offset the camera around our center point
	{
		mCameraPosition.x -= (Width() / 2);
		mCameraPosition.y -= (Height() / 2);
	}
}

void Map::SetCameraPosition(rect r, bool centered) 
{
	point2d p(r.x, r.y);	
	if (centered) 
	{
		p.x += (r.w / 2);
		p.y += (r.h / 2);
	}
	
	SetCameraPosition(p, centered);
}

rect Map::GetCameraPosition()
{
	return rect(mCameraPosition.x, mCameraPosition.y, Width(), Height());
}

void Map::OffsetCamera(sShort offsetX, sShort offsetY)
{
	mCameraPosition.x += offsetX;
	mCameraPosition.y += offsetY;
}

void Map::AddCameraRectForUpdate(bool force)
{
	rect r = GetCameraPosition();
	
	//do offsets for camera shaking
	r.x += mCameraFollowOffset.x;
	r.y += mCameraFollowOffset.y;

	if (force
		|| mOldCameraRect.x != r.x 
		|| mOldCameraRect.y != r.y
		|| mOldCameraRect.w != r.w
		|| mOldCameraRect.h != r.h)
	{
		g_screen->AddRect(GetScreenPosition());
		mOldCameraRect = r;
	}
}	

void Map::OffsetCamera(point2d p)
{
	OffsetCamera(p.x, p.y);
}

void Map::UpdateCamera()
{
	//If we have any destinations, pan toward the oldest
	if (!mCameraDestinationStack.empty())
	{
	/*	sShort dx, dy;
		
		dx = mCameraDestinationStack.at(0).x - (mCameraPosition.x );//+ (Width() / 2));
		dy = mCameraDestinationStack.at(0).y - (mCameraPosition.y );//+ (Height() / 2));
	
		double theta = (dx == 0) ? 0 : tan(dy/dx);
		dx = static_cast<sShort>((double)mCameraSpeed * cos(theta));
		dy = static_cast<sShort>((double)mCameraSpeed * sin(theta));
	
		//auto correct once we get close
		if (mCameraPosition.x < mCameraDestinationStack.at(0).x 
			&& mCameraPosition.x + dx >= mCameraDestinationStack.at(0).x)
			mCameraPosition.x = mCameraDestinationStack.at(0).x;
		else
			mCameraPosition.x += dx;
			
		if (mCameraPosition.y < mCameraDestinationStack.at(0).y 
			&& mCameraPosition.y + dy >= mCameraDestinationStack.at(0).y)
			mCameraPosition.y = mCameraDestinationStack.at(0).y;
		else
			mCameraPosition.y += dy;
			
		//re-center it again
	//	mCameraPosition.x -= (Width() / 2);
	//	mCameraPosition.y -= (Height() / 2);

		if (mCameraPosition.x == mCameraDestinationStack.at(0).x 
			&& mCameraPosition.y == mCameraDestinationStack.at(0).y)
		{
			mCameraDestinationStack.erase(mCameraDestinationStack.begin());
			MessageData md("CAMERA_PANNED");
			messenger.Dispatch(md, this);
			
			if (mCameraDestinationStack.empty())
			{
				MessageData md("CAMERA_PANCLEAR");
				messenger.Dispatch(md, this);	
			}
		}*/
	}
	else
	{
		Entity* e = GetCameraFollow();
		
		//If we're following, track their position.
		if (e)
		{
			SetCameraPosition(e->mPosition, true);
		}
			
		if (mStopCameraAtMapEdge)
			_constrainCameraToMap();
	}
	
	AddCameraRectForUpdate(false);
}

bool Map::IsRectInCamera(rect mapRect)
{
	return areRectsIntersecting(mapRect, GetCameraPosition());
}

rect Map::ToCameraPosition(rect screenRect)
{
	rect r = GetScreenPosition();
	return rect(screenRect.x + mCameraPosition.x - r.x, 
				screenRect.y + mCameraPosition.y - r.y,
				screenRect.w, screenRect.h);
}

rect Map::ToScreenPosition(rect mapRect)
{
	rect r = GetScreenPosition();
	r.x += mapRect.x - mCameraPosition.x;
	r.y += mapRect.y - mCameraPosition.y;
	r.w = mapRect.w;
	r.h = mapRect.h;
	
	//do offsets for camera shaking
	r.x += mCameraFollowOffset.x;
	r.y += mCameraFollowOffset.y;
	
	return r;
}

void Map::_constrainCameraToMap() 
{
	_constrainCameraX();
	_constrainCameraY();
}

void Map::_constrainCameraX()
{	
	//If the map width is smaller than our camera, center map on camera
	/*if (mWidth < Width() && mWidth != 0)
	{
		mCameraPosition.x = -(Width() - mWidth) / 2;
	}
	else //Constrain X to map edges
	{
		if (mCameraPosition.x + Width() > mWidth && mWidth != 0) 
			mCameraPosition.x = mWidth - Width();
			
		if (mCameraPosition.x < 0) 
			mCameraPosition.x = 0;
	}
	*/
	
	if (mCameraBounds.w > 0)
	{
		if (mCameraPosition.x < mCameraBounds.x)
			mCameraPosition.x = mCameraBounds.x;
		else if (mCameraPosition.x + Width() > mCameraBounds.x + mCameraBounds.w)
			mCameraPosition.x = mCameraBounds.x + mCameraBounds.w - Width();
	}
}

void Map::_constrainCameraY()
{
	//If the map height is smaller than our camera, center map on camera
	/*if (mHeight < Height() &&  mHeight != 0)
	{
		mCameraPosition.y = -(Height() - mHeight) / 2;
	} 
	else //Constrain Y to map edges
	{ 
		if (mCameraPosition.y + Height() > mHeight && mHeight != 0) 
			mCameraPosition.y = mHeight - Height();
	
		if (mCameraPosition.y < 0) 
			mCameraPosition.y = 0;
	}
	*/
	
	if (mCameraBounds.h > 0)
	{
		if (mCameraPosition.y < mCameraBounds.y)
			mCameraPosition.y = mCameraBounds.y;
		else if (mCameraPosition.y + Height() > mCameraBounds.y + mCameraBounds.h)
			mCameraPosition.y = mCameraBounds.y + mCameraBounds.h - Height();
	}
}

void Map::AddCameraDestination(point2d p)
{
	mCameraDestinationStack.push_back(p);
}

void Map::SetFlag(string flag, string value)
{
	flag = base64_encode(flag.c_str(), flag.length());
	value = base64_encode(value.c_str(), value.length());
	mFlags[flag] = value;
	SaveFlags();
}

string Map::GetFlag(string flag)
{
	flag = base64_encode(flag.c_str(), flag.length());
	string value = mFlags[flag];
	
	if (value.empty())
		return value;
	
	return base64_decode(value.c_str(), value.length());
}

// Load our flags from PLAYERSAVE_FILENAME if we're in game mode
void Map::LoadFlags()
{
	if (!game)
		return;
		
	string flags = game->mUserData.GetValue("Map:" + mId, "Flags");
	vString v;
	explode(&v, &flags, ",");
	
	for (int i = 0; i < v.size(); i+=2)
	{
		DEBUGOUT(v.at(i));
		if (v.size() <= i+1)
			break;
		DEBUGOUT("[" + v.at(i) + "] => " + v.at(i+1));
		mFlags[v.at(i)] = v.at(i+1); //since it's encrypted, set directly. Not through SetFlag()
	}
}

// Save our map flags to PLAYERSAVE_FILENAME. If we don't have any, and an element exists, it'll be removed
void Map::SaveFlags()
{
	if (!game)
		return;

	string flags;
	
	//Convert our map to a string
	for (std::map<string, string>::iterator it = mFlags.begin(); it != mFlags.end(); ++it) 
	{
		flags += it->first + ',';
		flags += it->second + ',';
	}	

	game->mUserData.SetValue("Map:" + mId, "Flags", flags);
}

void Map::HideChat()
{
	mChat->SetVisible(false);
	
	if (!Get("ShowChat"))
	{
		Button* b = new Button(this, "ShowChat", rect(Width() - 30, Height() - 20, 30, 20), 
								"", callback_showChatbox);
			b->mHoverText = "Show Chat";
			b->SetImage("assets/buttons/show_chat.png");
	}
}

void Map::ShowChat()
{
	Widget* w = Get("ShowChat");
	if (w) w->Die();
	
	mChat->SetVisible(true);
}


