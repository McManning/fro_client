
#include "MyAchievements.h"
#include "../game/GameManager.h"

MyAchievements::MyAchievements() :
	Frame(gui, "achievements", rect(0, 0, 500, 400), "My Achievements", true, true, true, true)
{
	Center();
	
	mDefaultIcon = NULL;

	_load();

	mDefaultIcon = resman->LoadImg("assets/ach_notearned.png");
	
	mAchFrame = new Frame(this, "", rect(5, 30, Width()-30, Height()-45));

	int max = mAchievements.size()-1;
	if (max < 0)
		max = 0;
	mScroller = new Scrollbar(this, "", rect(5+mAchFrame->Width(), 30, 20, mAchFrame->Height()), 
								VERTICAL, max, 1, 0, NULL);
	
	ResizeChildren(); //get them into position
}

MyAchievements::~MyAchievements()
{
	for (uShort i = 0; i < mAchievements.size(); i++)
		resman->Unload(mAchievements.at(i).icon);	

	resman->Unload(mDefaultIcon);
}

void MyAchievements::_load()
{
	TiXmlElement* top = game->mPlayerData.mDoc.FirstChildElement();
	TiXmlElement* e;
	
	ASSERT(top);

	e = top->FirstChildElement("achievements");
	achievement a;
	if (e)
	{
		e = e->FirstChildElement();
		while (e)
		{
			a.title = game->mPlayerData.GetParamString(e, "title");
			a.description = game->mPlayerData.GetParamString(e, "desc");
			a.max = game->mPlayerData.GetParamInt(e, "max");
			a.total = game->mPlayerData.GetParamInt(e, "total");

		/*	a.icon = NULL;
			if (a.file.empty())
				a.icon = NULL;
			else
				a.icon = resman->LoadImg(	"cache/achievements/" + a.file + ".png", 
											game->mConfig.GetParamString("connection", "achievements") 
												+ a.file + ".png", "", true
										);
*/
			//TODO: Dynamic loading of achievement icons!
			if (a.max != a.total)
				a.icon = NULL; //use default not-earned icon
			else 
				a.icon = resman->LoadImg("assets/ach_earned.png");

			mAchievements.push_back(a);
			
			e = e->NextSiblingElement();
		}
	}
}

const int ICON_SIZE = 30;

// Returns the y position of where to render the next achievement under this
uShort MyAchievements::_renderSingle(rect r, uShort index)
{
	Image* scr = Screen::Instance();

	//Render background color
	if (index % 2)
		scr->DrawRect( r, color(200,200,200) );
	else
		scr->DrawRect( r, color(220,220,220) );
			
	//render icon at (r.x + 5, r.y + 10, iconSize, iconSize)
//	scr->DrawRect( rect(r.x+5, r.y+10, ICON_SIZE, ICON_SIZE), color() );
	
	if (mAchievements.at(index).icon && mAchievements.at(index).icon->mImage->state == SDL_Image::LOADED)
		mAchievements.at(index).icon->Render( scr, r.x+5, r.y+10, rect(0,0,ICON_SIZE,ICON_SIZE) );
	else if (mDefaultIcon)
		mDefaultIcon->Render( scr, r.x+5, r.y+10, rect(0,0,ICON_SIZE,ICON_SIZE) );	

	sShort y = r.y + 5;
	
	// Render title
	Font* titleFont = fonts->Get("", 18, TTF_STYLE_BOLD);
	color c;
	if (titleFont)
	{
		if (mAchievements.at(index).total < mAchievements.at(index).max)
			c.r = c.g = c.b = 150;
		else
			c.r = c.g = c.b = 0;
		
		titleFont->Render( scr, r.x + ICON_SIZE + 10, y, mAchievements.at(index).title, c );
		y += titleFont->GetHeight() + 3;
	}
	
	// Render description 
	Font* descFont = fonts->Get("", 10);
	if (descFont)
	{
		descFont->Render( scr, r.x + ICON_SIZE + 10, y, mAchievements.at(index).description, 
							color(100,100,100), r.w - (ICON_SIZE + 15) );
		y += descFont->GetHeight(mAchievements.at(index).description, r.w - (ICON_SIZE + 15));
	}
	
	// Render progress bar if it has a max > 1
	if (mAchievements.at(index).max > 1)
	{
		//TODO: a bar! But for now, just text.
		if (descFont)
			descFont->Render( scr, r.x + ICON_SIZE + 10, y, its(mAchievements.at(index).total) + " / " + its(mAchievements.at(index).max),
								color(100, 100, 100) );
		y += descFont->GetHeight();
	}
	
	return y + 5;
}

void MyAchievements::Render()
{
	Frame::Render();
	
	if (mResizing)
		return;
	
	Image* scr = Screen::Instance();
	rect oldclip = scr->GetClip();
	rect r = mAchFrame->GetScreenPosition();

	//contain our achievements listing
	scr->SetClip(r);

	//render each visible one
	for (int i = mScroller->GetValue(); i < mAchievements.size(); ++i)
	{
		if (i < 0) continue;
		
		r.y = _renderSingle(r, i);
		
		//if we rendered them off the available area, we're done.
		if (r.y >= mAchFrame->GetScreenPosition().y + r.h)
			break;
	}

	scr->SetClip(oldclip);
}

void MyAchievements::ResizeChildren()
{
	mAchFrame->SetPosition( rect(5, 30, Width()-30, Height()-45) );
	mScroller->SetPosition( rect(5+mAchFrame->Width(), 30, 20, mAchFrame->Height()) );
	
	Frame::ResizeChildren();
}

void MyAchievements::SetPosition(rect r)
{
	if (r.h >= 200 && r.w >= 300)
		Frame::SetPosition(r);	
}

