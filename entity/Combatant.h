
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


#ifndef _COMBATANT_H_
#define _COMBATANT_H_

#include "../core/Core.h"

#define MAX_COMBATANT_SKILLS 5
#define MAX_COMBATANT_TYPES 5

/**	
	A base class that Actors will inherit from to give them RPG-like statistics used for combat. 
	Will manage things like stats, health, getting hurt, gaining exp, etc. Will mostly use messenger
	events for its various inner workings, as it will rely mostly on working with Lua, rather than 
	having hardcoded methods. 
*/
class Combatant
{
  public:
	Combatant();
	virtual ~Combatant() {};

	/**	
		Using the base stats, it will attempt to recalculate our current stats
		(things such as health, attack, defense, etc) based on our current level, 
		DNA, and other factors.
	*/	
	virtual void RecalculateStats() = 0;

	/**
		Set to a specific level. Do not use for leveling up calculations,
		as the current experience points will not roll over to the next level. 
	*/
	void SetLevel(char level);
	
	/**
		Gene is a special value that will factor in when calculating stats
		to make this particular entity unique, despite having shared species
		with others
	*/
	void SetGene(char gene);

	/**
		Add experience points to m_iExp. If m_iExp > m_iMaxExp,
		will call LevelUp(). This function may call LevelUp() multiple times
		if a large amount is inputted.
	*/
	void AddExperience(int exp);
	
	/**
		Increases m_iLevel by one, recalculates stats, and sends
		out ENTITY_LEVEL for anyone to pick up and mess with
	*/
	virtual void LevelUp() = 0;
	
	/**	
		Will reduce mCurrentHealth, and indicate as such. Will either
		trigger ENTITY_HURT or ENTITY_DEATH (if m_iCurrentHealth < 1) events.
		@param attacker If null, will be considered to be world-damage. 
		@param damage The amount of damage taken
	*/
	virtual void TakeDamage(Combatant* attacker, int damage) = 0;

	struct skill
	{
		string id;	
	};

	char m_bLevel;
	char m_bGene;
	
	// Type information slots.
	char m_bType[MAX_COMBATANT_TYPES];
	
	// current stats
	int m_iAttack; // read-only
	int m_iDefense; // read-only
	int m_iSpeed; //read-only
	int m_iMaxHealth; //read-only
	int m_iCurrentHealth;
	int m_iExp; //experience points
	int m_iMaxExp; //read-only. exp needed to reach the next level
	
	// base stats
	char m_bBaseAttack;
	char m_bBaseDefense;
	char m_bBaseSpeed;
	char m_bBaseHealth;

	skill m_sSkills[MAX_COMBATANT_SKILLS];

	bool m_bDisplayStats; // if true, an overhead display of stats will be shown
};

#endif //_COMBATANT_H_


