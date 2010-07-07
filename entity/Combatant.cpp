
#include "Combatant.h"

Combatant::Combatant()
{
	m_iLevel = 0;
	m_iGene = 0;

	m_iType1 = 0;
	m_iType2 = 0;
	m_iType3 = 0;
	
	m_iBaseAttack = 0;
	m_iBaseDefense = 0;
	m_iBaseSpeed = 0;
	m_iBaseHealth = 0;

	m_iAttack = 0;
	m_iDefense = 0;
	m_iSpeed = 0;
	m_iMaxHealth = 1;
	m_iCurrentHealth = 1;
	m_iExp = 0;
	m_iMaxExp = 1;

	m_bDisplayStats = false;
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

