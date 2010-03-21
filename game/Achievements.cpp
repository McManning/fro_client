
#include "Achievements.h"
#include "GameManager.h"

//each time user changes their avatar
void achievement_FashionAddict()
{
	ASSERT(game && timers);
	
	static int fashionAddictCounter = 0;

	// Check for rapid Avatar change achievement
	if (timers->Find("ach_avychng"))
	{
		++fashionAddictCounter;
		if (fashionAddictCounter == ACH_FASHIONADDICT_COUNT)
		{
			game->EarnAchievement("Fashion Addict", 
				"Rapidly cycle through avatars in an attempt to be the prettiest girl at the ball.", 
				1, "fashion_addict");
		}
	}
	else
	{
		timers->Add("ach_avychng", ACH_FASHIONADDICT_MS, false, NULL, NULL);
		fashionAddictCounter = 1;
	}
}

//whenever user hits shift
void achievement_StickyKeys()
{
	ASSERT(game && timers);
	
	static int stickyKeysCounter = 0;
	
	if (timers->Find("ach_stkykeys"))
	{
		++stickyKeysCounter;
		if (stickyKeysCounter == ACH_STICKYKEYS_COUNT)
		{
			game->EarnAchievement("Sticky Keys", 
				"Congratulations! You have activated Sticky Keys yet again!", 
				1, "sticky_keys");
		}
	}
	else
	{
		timers->Add("ach_stkykeys", ACH_STICKYKEYS_MS, false, NULL, NULL);
		stickyKeysCounter = 1;
	}
}

//each time user gains an item he doesn't already have
void achievement_TreasureHunter()
{
	ASSERT(game);
	
	game->EarnAchievement("Treasure Hunter", 
		"Collect " + its(ACH_TREASUREHUNTER_MAX) + " unique items.", 
		ACH_TREASUREHUNTER_MAX, "treasure_hunter");
}

//each time user deletes an item. Input the number of items deleted
void achievement_WasteNot(int count)
{
	ASSERT(game);
	
	for (int i = 0; i < count; i++)
	{
		if (game->EarnAchievement("Waste Not", 
			"Send " + its(ACH_WASTENOT_MAX) + " items to the incinerator.", 
			ACH_WASTENOT_MAX, "treasure_hunter") == ACH_WASTENOT_MAX)
			break;
	}
}

//Each time an achievement is earned. 
void achievement_OverAchiever()
{
	ASSERT(game);
	
	game->EarnAchievement("Over-Achiever", 
		"Earn " + its(ACH_OVERACHIEVER_MAX) + " achievements. What's the matter, got nothing else to do?", 
		ACH_OVERACHIEVER_MAX, "over_achiever");
}

//whenever the user speaks
void achievement_NeedASpamBlocker()
{
	ASSERT(game && timers);
	
	static int spamCounter = 0;

	// Check for rapid Avatar change achievement
	if (timers->Find("ach_spam"))
	{
		++spamCounter;
		DEBUGOUT("spam++ " + its(spamCounter));
		if (spamCounter == ACH_NEEDASPAMBLOCKER_COUNT)
		{
			game->EarnAchievement("We Need A Spam Blocker", 
				"Someone needs to Ess Tee Eff Yoo once in a while.", 
				1, "spammer");
		}
	}
	else
	{
		DEBUGOUT("Adding spam counter");
		timers->Add("ach_spam", ACH_NEEDASPAMBLOCKER_MS, false, NULL, NULL);
		spamCounter = 1;
	}
}




