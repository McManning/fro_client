
#include "LoginDialog.h"

#include "../game/GameManager.h"

#include "../core/widgets/Label.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Checkbox.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/io/FileIO.h"
#include "../core/net/IrcNet2.h"

LoginDialog* loginDialog;

/* Welcome, sent after login
<welcome>
	<msg title="News">
		Message
	</msg>
	<update title="New Shit Yo!" version="1.0.1"> <!-- Would only appear if client version doesn't match -->
		Message
	</update>
	<server>addr:port</server>
	<server>addr:port</server>
	<server>addr:port</server>
	<start>mapid</start>
</welcome>
*/
int callback_welcomeXmlParser(XmlFile* xf, TiXmlElement* e, void* userData)
{
	string id = e->Value();
	Console* c;

	DEBUGOUT("callback_welcomeXmlParser: " + id);
	
	if (id == "msg")
	{
		new MessagePopup("", xf->GetParamString(e, "title"), xf->GetText(e), true);	
	}
	else if (id == "alert")
	{
		new MessagePopup("", xf->GetParamString(e, "title"), xf->GetText(e));	
	}
	else if (id == "error")
	{
		new MessagePopup("", "Error", xf->GetText(e));	
		//game->syboltId.clear();
		//game->syboltPass.clear();
		game->mUsername.clear();
		game->mPassword.clear();
	}
	else if (id == "server")
	{
		game->mNet->mServerList.push_back(xf->GetText(e));	
	}
	else if (id == "start")
	{
		loginDialog->Die();

		game->mUsername = loginDialog->mUsername;
		game->mPassword = loginDialog->mPassword;

		game->mQueuedMapId = xf->GetText(e);
		game->mNet->TryNextServer();
		
		loginDialog = NULL;
	}
	
	return XMLPARSE_SUCCESS;
}

void dlCallback_welcomeXmlSuccess(downloadData* data)
{
	string error;
	
	XmlFile xf;
	xf.SetParser(callback_welcomeXmlParser);

	if (!xf.LoadFromFile(data->filename))
	{
		error = "Invalid welcome xml file.";
	}
	else
	{
		xf.SetEntryPoint("welcome");
		if (xf.Parse(NULL) != XMLPARSE_SUCCESS)
			error = "Error while parsing welcome Xml.";
	}

	removeFile(data->filename);
	
	if (!error.empty())
	{
		console->AddMessage(error);
		new MessagePopup("loginerror", "Login Error", error);
	}

	if (loginDialog)
		loginDialog->SetControlState(true);
}

void dlCallback_welcomeXmlFailure(downloadData* data)
{
	string error = "Could not retrieve login information. Error was that the ";

	switch (data->errorCode)
	{
		case DEC_BADHOST:
			error += "server address was invalid.";
			break;
		case DEC_CONNECTFAIL:
			error += "network could not connect to the server.";
			break;
		case DEC_FILENOTFOUND:
			error += "xml could not be located.";
			break;
		default:
			error += "downloader experienced an unknown error.";
			break;
	}
	
	//error += " We suggest connecting to a world manually.";

	console->AddMessage(error);
	new MessagePopup("loginerror", "Login Error", error);
	
	if (loginDialog)
		loginDialog->SetControlState(true);
}

void callback_LoginDialogSendLogin(Button* b)
{
	loginDialog->SendLogin();
}

void callback_LoginDialogSkip(Button* b)
{
	loginDialog->Skip();
}

void callback_LoginDialogRegister(Button* b)
{
	string msg = "Registration is \\c300optional\\c000. If you choose not to use an account, "
				" you can still access the worlds by hitting the \\c300Skip Login\\c000 button.\\n\\n"
				"To register, or read about the benefits of registration, right click the following link: \\n\\n" 		
				"\\c008http://sybolt.com/drm/register.php";
		
	new MessagePopup("register", "Register", msg, true);	
}

LoginDialog::LoginDialog() :
	Frame(gui, "login", gui->GetScreenPosition())
{
	ASSERT(!loginDialog); //there can be only one
	
	uShort y = 30;

	Input* i;
	Button* b;
	Checkbox* c;
	Label* l;
	
	mLoginFrame = new Frame(this, "", rect(50,50), "Login to Sybolt", true, false, false, true);
	
	TiXmlElement* e = game->mPlayerData.mDoc.FirstChildElement("data")->FirstChildElement("login");
	
	new Label(mLoginFrame, "", rect(10,y), "ID");
	i = new Input(mLoginFrame, "id", rect(60, y, 150, 20), "", 32, true, NULL);
		i->SetText( game->mPlayerData.GetParamString(e, "id") );
		i->SetKeyFocus();
	y += 25;	
	
	new Label(mLoginFrame, "", rect(10,y), "Pass");
	i = new Input(mLoginFrame, "pass", rect(60, y, 150, 20), "", 32, true, NULL);
		i->SetText( game->mPlayerData.GetParamString(e, "pass") );
	y += 25;

	//checkboxes to the left
	c = new Checkbox(mLoginFrame, "remember", rect(10,y), "Remember Me", 0);
		c->SetState( game->mPlayerData.GetParamInt(e, "remember") );
	y += 25;
	
	
	//bottom button set
	b = new Button(mLoginFrame, "register",rect(10,y,20,20), "", callback_LoginDialogRegister);
		b->mHoverText = "Register";
		makeImage(b, "", "assets/login.png", rect(0,0,20,20),
				rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	
	b = new Button(mLoginFrame, "login",rect(160,y,20,20), "", callback_LoginDialogSendLogin);
		b->mHoverText = "Send Login";
		makeImage(b, "", "assets/login.png", rect(20,0,20,20),
				rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);
	
	b = new Button(mLoginFrame, "skip",rect(190,y,20,20), "", callback_LoginDialogSkip);
		b->mHoverText = "Skip Login";
		makeImage(b, "", "assets/login.png", rect(40,0,20,20),
				rect(0,0,20,20), WIDGETIMAGE_FULL, true, false);

	l = new Label(mLoginFrame, "status", rect(10,y), "Getting Server Verification...");
		l->SetVisible(false);

	y += 25;

	mLoginFrame->SetSize(220, y);
	mLoginFrame->ResizeChildren();
	//mLoginFrame->Center();
//	DemandFocus();
	
	if (game)
		game->SetVisible(false);
	
	loginDialog = this;
}

LoginDialog::~LoginDialog()
{
	loginDialog = NULL;
	
	if (game)
		game->SetVisible(true);
}

void LoginDialog::Render(uLong ms)
{	
	//if (gui->GetDemandsFocus() == this)
	//	gui->RenderDarkOverlay();

	Frame::Render(ms);
}

void LoginDialog::Skip()
{
	SendLoginQuery(true);
}

void LoginDialog::SendLogin()
{
	Input* i;
	Checkbox* c;
	
	i = (Input*)mLoginFrame->Get("id");
	if (i && i->GetText().empty())
	{
		new MessagePopup("", "Invalid Login", "You must input an ID");
		return;
	}

	c = (Checkbox*)mLoginFrame->Get("remember");
	if (c)
	{
		TiXmlElement* e = game->mPlayerData.mDoc.FirstChildElement("data")->FirstChildElement("chat");
		
		game->mPlayerData.SetParamInt(e, "remember", c->GetState());
		
		if (c->GetState() == 1)
		{
			if (i)
				game->mPlayerData.SetParamString(e, "id", i->GetText());
			
			i = (Input*)mLoginFrame->Get("pass");
			if (i)
				game->mPlayerData.SetParamString(e, "pass", i->GetText());
		}
	}

	SendLoginQuery(false);
}

void LoginDialog::SendLoginQuery(bool skip)
{
	Checkbox* c;
	Input* i;	
	
	//send http get: login.php?ver=1.1.0&id=test&pass=test
	
	FATAL("do this");
	string query = ""; //game->mConfig.GetParamString("connection", "login") + "?";

	query += "ver=";
	query += APP_VERSION;

	if (!skip)
	{
		i = (Input*)mLoginFrame->Get("id");
		mUsername = i->GetText();
		
		i = (Input*)mLoginFrame->Get("pass");
		mPassword = i->GetText();
		
		if (!mUsername.empty())
		{
			query += "&id=" + htmlSafe(mUsername);
			
			if (!mPassword.empty())
			{
				//mPassword = md5(mPassword.c_str(), mPassword.length());	
				query += "&pass=" + htmlSafe(mPassword);
			}
		}
	}
	
	downloader->QueueDownload(query, getTemporaryCacheFilename(),
									NULL, dlCallback_welcomeXmlSuccess,
									dlCallback_welcomeXmlFailure, true);
	SetControlState(false);
}

void LoginDialog::SetControlState(bool enabled)
{
	mLoginFrame->Get("login")->SetVisible(enabled);
	mLoginFrame->Get("skip")->SetVisible(enabled);
	mLoginFrame->Get("register")->SetVisible(enabled);
	mLoginFrame->Get("remember")->SetVisible(enabled);
	mLoginFrame->Get("id")->SetActive(enabled);
	mLoginFrame->Get("pass")->SetActive(enabled);
	
	Button* b = (Button*)mLoginFrame->Get("frameclose");
	if (b)
		b->SetActive(enabled);
		
	mLoginFrame->Get("status")->SetVisible(!enabled);
}
