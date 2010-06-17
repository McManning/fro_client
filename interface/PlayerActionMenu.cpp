
#include "PlayerActionMenu.h"
#include "../core/widgets/Button.h"
#include "../entity/Actor.h"

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
	
	menu->Die();
}

void callback_Defend(Button* b)
{
	PlayerActionMenu* menu = (PlayerActionMenu*)b->GetParent()->GetParent();
	
	// Send defend event
	MessageData md("DUEL_USE_DEFEND");
	messenger.Dispatch(md);
	
	menu->Die();
}

void callback_RunAway(Button* b)
{
	PlayerActionMenu* menu = (PlayerActionMenu*)b->GetParent()->GetParent();
	
	// Send run away event
	MessageData md("DUEL_USE_RUN");
	messenger.Dispatch(md);
	
	menu->Die();
}

void callback_Item(Button* b)
{
	PlayerActionMenu* menu = (PlayerActionMenu*)b->GetParent()->GetParent();
	
	//inventory->SetVisible(true);
	//inventory->
	
	/*
		TODO: Rewrite the callback for the inventory close & use buttons.
		Close button should hide inventory & redisplay the main menu of this, and replace itself with the 
			old inventory close button callback.
		Use button should call a thing here so it can pass a new event, and kill player action menu
			then replace itself with the old inventory close button callback.
	*/
	
	//menu->Die();
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
		
		menu->Die();
		return TIMER_DESTROY;
	}

	menu->SetCaption("Select Action (" + its(menu->m_iCountdown) + " sec)");
	return TIMER_CONTINUE;
}


PlayerActionMenu::PlayerActionMenu(int timeout, Actor* controlled)
	: Frame(gui, "PlayerActionMenu", rect(0,0,200,100), "Select Action (" + its(timeout) + " sec)", true, false, false, true)
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
		
		b = new Button(m_pChoicesFrame, "", r, "Use Skill", callback_Skill);
		r.y += 25;

		b = new Button(m_pChoicesFrame, "", r, "Use Item", callback_Item);
			b->SetActive(false);
		r.y += 25;

		b = new Button(m_pChoicesFrame, "", r, "Tag Out", NULL);
			b->SetActive(false);
		r.y += 25;

		b = new Button(m_pChoicesFrame, "", r, "Defend", callback_Defend);
		r.y += 25;
		
		b = new Button(m_pChoicesFrame, "", r, "Run Away", callback_RunAway);
		r.y += 25;

		r.y += 5;
		
		m_pChoicesFrame->SetSize(Width()-20, r.y);
		
	SetSize(Width(), r.y+30);

		r.y = 0;
		
		b = new Button(m_pSkillsFrame, "1", r, "SKILL 1", callback_UseSkill);
		r.y += 25;
		
		b = new Button(m_pSkillsFrame, "2", r, "SKILL 2", callback_UseSkill);
		r.y += 25;
		
		b = new Button(m_pSkillsFrame, "3", r, "SKILL 3", callback_UseSkill);
		r.y += 25;
		
		b = new Button(m_pSkillsFrame, "4", r, "SKILL 4", callback_UseSkill);
		r.y += 25;

		b = new Button(m_pSkillsFrame, "4", r, "Back to actions", callback_BackToActions);
		r.y += 25;
		
		m_pSkillsFrame->SetSize(Width()-20, r.y);
		m_pSkillsFrame->SetVisible(false);
		
	ResizeChildren();
	Center();
	
	DemandFocus();

	timers->Add("actiontimeout", 1000, false, timer_checkForTimeout, NULL, this);
}

PlayerActionMenu::~PlayerActionMenu()
{
	timers->RemoveMatchingUserData(this);
}


