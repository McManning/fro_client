
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


#ifndef _IRCNET_SENDERS_H_
#define _IRCNET_SENDERS_H_

#include "../core/Core.h"

class Entity;
class Avatar;

void netSendBeat(Entity* target, string item); //act 2 target item
void netSendMusic(string song); //act 1 song
void netSendMe(string text); //act $text
void netSendStamp(string text); //stp x y rotation color $text
void netSendSay(string text); //say $message || act $message
void netSendAvatar(Avatar* a, string nick = ""); //avy $url #w #h ...
void netSendEmote(uShort num); //emo #id
void netSendRequestAvatar(string nick); // reqAvy
void netSendAchievement(string title); // ern $title (earn, get it, get it!?)
void netSendState(string targetNick, string header); //header $chan $name #x #y #dir #action Avatar Stuff
void netSendActionBuffer(); //mov #x #y $buffer
void netSendAvatarMod(); // mod #id

#endif //_IRCNET_SENDERS_H_
