
#include "CheckIn.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/io/FileIO.h"
#include "GameManager.h"
#include "../map/Map.h"
#include "../entity/LocalActor.h"

const int CHECK_IN_TIMER_INTERVAL_MS = 15*60*1000; //15min

/*	Reply from the server after a check in. With this xml (and proper security), 
	we can do several things, including event notification, forum news/update/private message
	notification, random item drops, additional data requests, etc. 
<checkin>
	<msg type="#">text</msg>
</checkin>
*/
int callback_CheckInXmlParser(XmlFile* xf, TiXmlElement* e, void* userData)
{
	string id = e->Value();
	
	if (id == "msg")
	{	
		// TODO: type.. blah blah
		if (game && game->mChat)
			game->mChat->AddMessage( xf->GetText(e) );
	}
	else if (id == "alert") // <alert title="error">TEXT</alert>
	{
		new MessagePopup("", xf->GetParamString(e, "title"), xf->GetText(e));
	}
	
	return XMLPARSE_SUCCESS;
}

void dlCallback_CheckInXmlSuccess(downloadData* data)
{
	string error;
	
	XmlFile xf;
	xf.SetParser(callback_CheckInXmlParser);

	if (xf.LoadFromFile(data->filename))
	{
		xf.SetEntryPoint("checkin");
		xf.Parse(NULL);
	}

	removeFile(data->filename);
}

void dlCallback_CheckInXmlFailure(downloadData* data)
{
	string error = "PONG Xml Error: " + its(data->errorCode);
	console->AddMessage(error);
}

uShort timer_CheckInWithServer(timer* t, uLong ms)
{
	//send http get: login.php?act=ping&nick=test&id=test&pass=test&map=library
	
	XmlFile xf;
	string query;
	TiXmlElement* e;
	
	if (!xf.LoadFromFile("assets/connections.cfg"))
	{
		FATAL(xf.GetError());	
	}
	
	e = xf.mDoc.FirstChildElement();
	if (e)
		e = e->FirstChildElement("login");
	
	if (e)
		query = xf.GetText(e);
	
	if (query.empty() || query.find("http://", 0) != 0)
	{
		FATAL("Invalid login address");
	}

	query += "?act=checkin";

	query += "&id=" + htmlSafe(game->mUsername);
	query += "&pass=" + htmlSafe(game->mPassword);
		
	if (game->mPlayer)
		query += "&nick=" + htmlSafe(game->mPlayer->mName);
	
	if (game->mMap)
		query += "&map=" + htmlSafe(game->mMap->mId);
	
	downloader->QueueDownload(query, getTemporaryCacheFilename(),
								NULL, dlCallback_CheckInXmlSuccess,
								dlCallback_CheckInXmlFailure, true);
							
	DEBUGOUT("Checking in with master");
									
	return TIMER_CONTINUE;
}

void StartCheckInTimer()
{
	timers->Add("", CHECK_IN_TIMER_INTERVAL_MS, false, timer_CheckInWithServer);
}


