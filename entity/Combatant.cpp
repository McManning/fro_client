
/*
 * Copyright (c) 2011 Chase McManning
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */


#include "Combatant.h"

Combatant::Combatant()
{
	m_bLevel = 0;
	m_bGene = 0;

	for (int i = 0; i < MAX_COMBATANT_TYPES; ++i)
		m_bType[i] = 0;
	
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

