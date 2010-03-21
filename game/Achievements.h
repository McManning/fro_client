
#ifndef _ACHIEVEMENTS_H_
#define _ACHIEVEMENTS_H_

const int ACH_FASHIONADDICT_COUNT = 5;
const int ACH_FASHIONADDICT_MS = 5000;
const int ACH_STICKYKEYS_COUNT = 5;
const int ACH_STICKYKEYS_MS = 4000;
const int ACH_TREASUREHUNTER_MAX = 30;
const int ACH_WASTENOT_MAX = 50;
const int ACH_OVERACHIEVER_MAX = 20;
const int ACH_NEEDASPAMBLOCKER_COUNT = 10;
const int ACH_NEEDASPAMBLOCKER_MS = 2000;

//each time user changes their avatar
void achievement_FashionAddict();

//whenever user hits shift
void achievement_StickyKeys();

//each time user gains an item he doesn't already have
void achievement_TreasureHunter();

//each time user deletes an item. Input the number of items deleted
void achievement_WasteNot(int count);

//Each time an achievement is earned. 
void achievement_OverAchiever();

//whenever the user speaks
void achievement_NeedASpamBlocker();

#endif //_ACHIEVEMENTS_H_
