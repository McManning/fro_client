
#include "Combatant.h"
#include "Actor.h"

Combatant::Combatant()
{
	m_iLevel = 0;
	m_iGene = 0;

	m_iAttack = 0;
	m_iDefense = 0;
	m_iSpeed = 0;
	m_iMaxHealth = 1;
	m_iCurrentHealth = 1;
	m_iExp = 0;
	m_iMaxExp = 1;

	m_bDisplayStats = false;
	
	m_pActor = NULL;
}

void Combatant::TakeDamage(Combatant* attacker, int damage)
{
	m_iCurrentHealth -= damage;
	
	// We died! Trigger an event!
	if (m_iCurrentHealth <= 0)
	{
		m_iCurrentHealth = 0;
		
		MessageData md("ENTITY_DEATH");
		md.WriteUserdata("entity", m_pActor);
		md.WriteUserdata("attacker", attacker);
		md.WriteInt("damage", damage);
		messenger.Dispatch(md);
	}
	else 
	{
		MessageData md("ENTITY_HURT");
		md.WriteUserdata("entity", m_pActor);
		md.WriteUserdata("attacker", attacker);
		md.WriteInt("damage", damage);
		messenger.Dispatch(md);
	}
}

void Combatant::RecalculateStats()
{
	// Send out a request for SOMEONE to recalculate our stats (hopefully picked up by lua)
	// TODO: Should we use this particular method to calculate? Couldn't this be dangerous? 
	//		Yet, still don't want to hard code it.
	MessageData md("ENTITY_RECALC");
	md.WriteUserdata("entity", m_pActor);
	messenger.Dispatch(md);
}

void Combatant::SetSpecies(combatantSpecies s)
{
	m_species = s;
	RecalculateStats();
}

void Combatant::SetLevel(int level)
{
	m_iLevel = level;
	m_iExp = 0;
	RecalculateStats();
}

void Combatant::SetGene(int gene)
{
	m_iGene = gene;
	RecalculateStats();
}

void Combatant::AddExperience(int exp)
{
	m_iExp += exp;
	
	// Can level up multiple times
	while (m_iExp >= m_iMaxExp)
	{
		m_iExp -= m_iMaxExp;
		if (m_iExp < 0)
			m_iExp = 0;
		LevelUp();
	}
}

void Combatant::LevelUp()
{
	++m_iLevel;
	RecalculateStats();
	
	MessageData md("ENTITY_LEVEL");
	md.WriteUserdata("entity", m_pActor);
	messenger.Dispatch(md);
}

