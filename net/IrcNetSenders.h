
#ifndef _IRCNET_SENDERS_H_
#define _IRCNET_SENDERS_H_

#include "../core/Core.h"

class Entity;
class Avatar;

// Used by LocalActor.cpp (TODO: Relocate?)
string compressActionBuffer(string buffer);

void netSendBeat(Entity* target, string item); //act 2 target item
void netSendMusic(string song); //act 1 song
void netSendMe(string text); //act $text
void netSendStamp(string text); //stp x y rotation color $text
void netSendSay(string text); //say $message || act $message
void netSendAvatar(Avatar* a, string nick = ""); //avy $url #w #h ...
void netSendEmote(uShort num); //emo #id
void netSendRequestAvatar(string nick); // reqAvy
void netSendAchievement(string title); // ern $title (earn, get it, get it!?)

#endif //_IRCNET_SENDERS_H_
