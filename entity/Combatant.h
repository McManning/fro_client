
#ifndef _COMBATANT_H_
#define _COMBATANT_H_

#include "../core/Core.h"

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
	
	// Type information slots
	char m_bType1;
	char m_bType2;
	char m_bType3;
	
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

	skill m_sSkills[5];

	bool m_bDisplayStats; // if true, an overhead display of stats will be shown
};

#endif //_COMBATANT_H_


