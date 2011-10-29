
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
#include "../game/GameManager.h"
#include "../map/Map.h"
#include "../entity/LocalActor.h"
#include "../core/net/IrcNet2.h"
#include "../core/io/tinyxml/tinyxml.h"

const int CHECK_IN_TIMER_INTERVAL_MS = 15*60*1000; //15min

/*	Reply from the server after a check in. With this xml (and proper security), 
	we can do several things, including event notification, forum news/update/private message
	notification, random item drops, additional data requests, etc. 
<checkin>
	blahblahblah
</checkin>
*/
void dlCallback_CheckInXmlSuccess(downloadData* data)
{
	TiXmlDocument doc;
	TiXmlElement* e;
	TiXmlElement* e2;
	const char* c;
	string id;
	
	if (doc.LoadFile(data->filename.c_str()) == false)
	{
		WARNING(doc.ErrorDesc());
		return;
	}
	
	removeFile(data->filename);
	
	e = doc.FirstChildElement("checkin");
	
	for (e; e; e = e->NextSiblingElement())
	{
		id = e->Value();
	
		if (id == "msg") // <msg>Stuff to add to chatbox</msg>
		{
			if (game && game->GetChat() && !e->NoChildren())
				game->GetChat()->AddMessage( e->FirstChild()->Value() );
		}
		else if (id == "alert") // <alert title="error">Alert message</alert>
		{
			c = e->Attribute("title");
			new MessagePopup("", (c) ? c : "Alert", e->FirstChild()->Value());
		}
		else if (id == "event")
		{
			/*
				<event id="SOME_EVENT">
					<key id="some key">data</key>
					<key ...></key>
				</event>
			*/
			c = e->Attribute("id");
			if (c)
			{
				MessageData md(c);
				
				e2 = e->FirstChildElement();
				for (e2; e2; e2 = e2->NextSiblingElement())
				{
					c = e2->Attribute("id");
					if (c && !e2->NoChildren())
					{
						md.WriteString(c, e2->FirstChild()->Value());
					}
				}
				messenger.Dispatch(md, NULL);
			}
		}
	}

}

void dlCallback_CheckInXmlFailure(downloadData* data)
{
	string error = "PONG Xml Error: " + its(data->errorCode);
	console->AddMessage(error);
}

uShort timer_CheckInWithServer(timer* t, uLong ms)
{
	//send http get: login.php?act=ping&nick=test&id=test&pass=test&map=library
	string query;

	// if we're not on a server, don't check in
	if (!game->mNet->IsConnected())
		return TIMER_CONTINUE;

	query = "http://sybolt.com/drm-svr/";
	query += "checkin.php?ver=";
	query += VER_STRING;

	query += "&id=" + htmlSafe(game->mUsername);
	query += "&pass=" + htmlSafe(game->mPassword);
		
	if (game->mPlayer)
		query += "&nick=" + htmlSafe(game->mPlayer->GetName());
	
	if (game->mMap)
		query += "&map=" + htmlSafe(game->mMap->mId);
	
	downloader->QueueDownload(query, getTemporaryCacheFilename(),
								NULL, dlCallback_CheckInXmlSuccess,
								dlCallback_CheckInXmlFailure, true);
							
	DEBUGOUT("Checking in with master");
									
	return TIMER_CONTINUE;
}

void startCheckInTimer()
{
	timers->Add("", CHECK_IN_TIMER_INTERVAL_MS, false, timer_CheckInWithServer);
}


