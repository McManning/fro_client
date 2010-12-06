
#include "PlayerActionMenu.h"
#include "../core/widgets/Button.h"
#include "../entity/Actor.h"
#include "../game/GameManager.h"
//#include "../interface/LunemParty.h"
#include "../interface/Inventory.h"

void callback_Skill(Button* b)
{
	PlayerActionMenu* menu = (PlayerActionMenu*)b->GetParent()->GetParent();
	
	menu->m_pSkillsFrame->SetVisible(true);
	menu->m_pChoicesFrame->SetVisible(false);
}

void callback_BackToActions(Button* b)
{
	PlayerActionMenu* menu = (PlayerActionMenu*)b->GetParent()->GetParent();
	
	menu->m_pSkillsFrame->SetVisible(false);
	menu->m_pChoicesFrame->SetVisible(true);
}

void callback_UseSkill(Button* b)
{
	PlayerActionMenu* menu = (PlayerActionMenu*)b->GetParent()->GetParent();
	int slot = sti(b->mId);

	// Send use skill event
	MessageData md("DUEL_USE_SKILL");
	md.WriteInt("slot", slot);
	messenger.Dispatch(md);
	
	game->EndPlayersDuelTurn();
}

void callback_Defend(Button* b)
{
	PlayerActionMenu* menu = (PlayerActionMenu*)b->GetParent()->GetParent();
	
	// Send defend event
	MessageData md("DUEL_USE_DEFEND");
	messenger.Dispatch(md);
	
	game->EndPlayersDuelTurn();
}

void callback_RunAway(Button* b)
{
	PlayerActionMenu* menu = (PlayerActionMenu*)b->GetParent()->GetParent();
	
	// Send run away event
	MessageData md("DUEL_USE_RUN");
	messenger.Dispatch(md);
	
	game->EndPlayersDuelTurn();
}

void callback_TagOut(Button* b)
{
	PlayerActionMenu* menu = (PlayerActionMenu*)b->GetParent()->GetParent();
	
	game->mParty->SetVisible(true);
	game->mParty->SetMenuMode(ActorStats::DUEL_SWAP_MENU);
	game->mParty->MoveToTop();
}

void callback_Item(Button* b)
{
	PlayerActionMenu* menu = (PlayerActionMenu*)b->GetParent()->GetParent();
	
	inventory->SetVisible(true);
	inventory->MoveToTop();
}

uShort timer_checkForTimeout(timer* t, uLong ms)
{
	PlayerActionMenu* menu = (PlayerActionMenu*)t->userData;
	
	if (!menu)
		return TIMER_DESTROY;

	if (--menu->m_iCountdown < 1)
	{
		// Alert the dueling sys that we timed out
		MessageData md("DUEL_TURN_TIMEOUT");
		messenger.Dispatch(md);
		
		game->EndPlayersDuelTurn();
		
		return TIMER_DESTROY;
	}

	menu->SetCaption("Select Action (" + its(menu->m_iCountdown) + " sec)");
	return TIMER_CONTINUE;
}


PlayerActionMenu::PlayerActionMenu(int timeout, Actor* controlled)
	: Frame(NULL, "PlayerActionMenu", rect(50,220,200,100), "Select Action (" + its(timeout) + " sec)", true, false, false, true)
{
	m_pControlled = controlled;
	m_iCountdown = timeout;
	mConstrainChildrenToRect = false;
	
	Button* b;
	rect r;

	r.x = 10;
	r.y = 30;
	r.w = Width() - 20;
	r.h = 20;

	m_pChoicesFrame = new Frame(this, "", r);
		m_pChoicesFrame->mConstrainChildrenToRect = false;
	m_pSkillsFrame = new Frame(this, "", r);
		m_pSkillsFrame->mConstrainChildrenToRect = false;
		
		r.x = 0;
		r.y = 0;
			
		if (controlled)
		{
			b = new Button(m_pChoicesFrame, "", r, "Use Skill", callback_Skill);
			r.y += 25;
	
			b = new Button(m_pChoicesFrame, "", r, "Use Item", callback_Item);
				b->SetActive(false);
			r.y += 25;
	
			b = new Button(m_pChoicesFrame, "", r, "Defend", callback_Defend);
			r.y += 25;
		}
		
		b = new Button(m_pChoicesFrame, "", r, "Tag Out", callback_TagOut);
		r.y += 25;

		b = new Button(m_pChoicesFrame, "", r, "Run Away", callback_RunAway);
		r.y += 25;

		r.y += 5;
		
		m_pChoicesFrame->SetSize(Width()-20, r.y);
		
	SetSize(Width(), r.y+30);

		r.y = 0;
			
		if (controlled)
		{
			for (int i = 0; i < 5; ++i)
			{
				if (!controlled->m_sSkills[i].id.empty())
				{
					b = new Button(m_pSkillsFrame, its(i), r, controlled->m_sSkills[i].id, callback_UseSkill);
					r.y += 25;	
				}
			}
		}
			
		m_pSkillsFrame->SetSize(Width()-20, r.y);
		m_pSkillsFrame->SetVisible(false);
		
	ResizeChildren();

	timers->Add("actiontimeout", 1000, false, timer_checkForTimeout, NULL, this);
}

PlayerActionMenu::~PlayerActionMenu()
{
	timers->RemoveMatchingUserData(this);
}


