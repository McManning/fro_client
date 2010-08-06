
#include "Combatant.h"

Combatant::Combatant()
{
	m_bLevel = 0;
	m_bGene = 0;

	m_bType1 = 0;
	m_bType2 = 0;
	m_bType3 = 0;
	
	m_bBaseAttack = 0;
	m_bBaseDefense = 0;
	m_bBaseSpeed = 0;
	m_bBaseHealth = 0;

	m_iAttack = 0;
	m_iDefense = 0;
	m_iSpeed = 0;
	m_iMaxHealth = 1;
	m_iCurrentHealth = 1;
	m_iExp = 0;
	m_iMaxExp = 1;

	m_bDisplayStats = false;
}

void Combatant::SetLevel(char level)
{
	m_bLevel = level;
	m_iExp = 0;
	RecalculateStats();
}

void Combatant::SetGene(char gene)
{
	m_bGene = gene;
	//RecalculateStats();
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

