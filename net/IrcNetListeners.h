
#ifndef _IRCNET_LISTENERS_H_
#define _IRCNET_LISTENERS_H_

#include "../core/Core.h"

string decompressActionBuffer(string buffer);

void stampMapText(int x, int y, int rotation, string& text);

void hookNetListeners();
void unhookNetListeners();


#endif //_IRCNET_LISTENERS_H_
