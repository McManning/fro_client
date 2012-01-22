
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


#include "CheckIn.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/io/FileIO.h"
#include "../core/io/Crypt.h"
#include "../game/GameManager.h"
#include "../map/Map.h"
#include "../entity/LocalActor.h"
#include "../core/net/IrcNet2.h"
#include "../lua/MapLib.h"

#ifdef DEBUG
const int CHECK_IN_TIMER_INTERVAL_MS = 60*1000; // 1min
#else
const int CHECK_IN_TIMER_INTERVAL_MS = 15*60*1000; //15min
#endif

/*	Reply from the server after a check in. With this lua code insertion (and proper security),
	we can do several things, including event notification, forum news/update/private message
	notification, random item drops, additional data requests, etc.
*/
void dlCallback_CheckInSuccess(downloadData* data)
{
    if (game->mMap && game->mMap->mLuaState)
        mapLib_CallCheckin(game->mMap->mLuaState, data->filename);

	removeFile(data->filename);
}

void dlCallback_CheckInFailure(downloadData* data)
{
	string error = "Checkin Reply Error: " + its(data->errorCode);
	console->AddMessage(error);
	removeFile(data->filename);
}

uShort timer_CheckInWithServer(timer* t, uLong ms)
{
	//send http get: login.php?act=ping&nick=test&id=test&pass=test&map=library
	string query;
	string s;

	// if we're not on a server, don't check in
	if (!game->mNet->IsConnected())
		return TIMER_CONTINUE;

	query = "http://sybolt.com/drm-svr/";
	query += "checkin.php?v=";

	// Note: Don't crypt it all together as one giant B64.
	// They could always stick &blah=blah as a password and falsify keys

	s = VER_STRING;
	CPHP_Encrypt(s, URL_CRYPT_KEY);
	query += s;

    s = game->mUsername;
    CPHP_Encrypt(s, URL_CRYPT_KEY);
    query += "&u=" + s;

    s = game->mPassword;
    CPHP_Encrypt(s, URL_CRYPT_KEY);
    query += "&k=" + s;

	if (game->mPlayer)
	{
	    s = game->mPlayer->GetName();
        CPHP_Encrypt(s, URL_CRYPT_KEY);
		query += "&n=" + s;
	}

	if (game->mMap)
	{
	    s = game->mMap->mId;
        CPHP_Encrypt(s, URL_CRYPT_KEY);
		query += "&m=" + s;
	}

	downloader->QueueDownload(query, getTemporaryCacheFilename(),
								NULL, dlCallback_CheckInSuccess,
								dlCallback_CheckInFailure, true);

	DEBUGOUT("Checking in with master");

	return TIMER_CONTINUE;
}

void startCheckInTimer()
{
	timers->Add("", CHECK_IN_TIMER_INTERVAL_MS, false, timer_CheckInWithServer);
}


