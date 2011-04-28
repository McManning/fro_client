
#include "LoginDialog.h"
#include "AutoUpdater.h"

#include "../game/GameManager.h"
#include "../net/CheckIn.h"

#include "../core/widgets/Label.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/Button.h"
#include "../core/widgets/Checkbox.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/Scrollbar.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/io/FileIO.h"
#include "../core/net/IrcNet2.h"

LoginDialog* loginDialog;

void callback_doUpdate(MessagePopup* m)
{
#ifdef WIN32	
	int result = (int)ShellExecute(NULL, "open", "updater.exe", "", NULL, SW_SHOWNORMAL);
	if (result <= 32)
	{
		systemErrorMessage("Error!", "Encountered error code " + its(result) + " while trying to run updater.exe!\n\nPlease complain at http://sybolt.com/community/");
	}	
#endif

	appState = APPSTATE_CLOSING;
}

/* Welcome, sent after login
<welcome>
	<msg title="News">
		Message
	</msg>
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
		new MessagePopup("", xf->GetParamString(e, "title"), xf->GetText(e), xf->GetParamInt(e, "long"));	
	}
	else if (id == "error") // <error title="blah" long="1">TEXT</error>
	{
		new MessagePopup("", xf->GetParamString(e, "title"), xf->GetText(e), xf->GetParamInt(e, "long"));	
		//game->syboltId.clear();
		//game->syboltPass.clear();
		game->mUsername.clear();
		game->mPassword.clear();
		
		if (loginDialog)
			loginDialog->SetControlState(true);
	}
	else if (id == "update") // <update title="??">TEXT</update>
	{
		MessagePopup* m = new MessagePopup("", xf->GetParamString(e, "title"), xf->GetText(e), true);	
			m->onCloseCallback = callback_doUpdate;
			m->DemandFocus();
	}
	else if (id == "server")
	{
		game->mNet->mServerList.push_back(xf->GetText(e));	
	}
	else if (id == "start")
	{
		//loginDialog->Die();

		game->mUsername = loginDialog->mUsername;
		game->mPassword = loginDialog->mPassword;

		if (!game->mUsername.empty())
			game->mNet->mRealname = game->mUsername;
			
		game->mStartingWorldId = xf->GetText(e);
		game->mNet->TryNextServer();
		
		startCheckInTimer();
		
		//loginDialog = NULL;
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
		
		if (loginDialog)
			loginDialog->SetControlState(true);
	}
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
	Frame(gui, "login", rect(50,50), "Login to Sybolt", true, false, false, true)
{
	mSortable = false;
	
	ASSERT(!loginDialog); //there can be only one
	
	uShort y = 30;

	Input* i;
	Button* b;
	Checkbox* c;
	Label* l;

	TiXmlElement* e = game->mPlayerData.mDoc.FirstChildElement("data")->FirstChildElement("login");
	
	new Label(this, "", rect(10,y), "ID");
	i = new Input(this, "id", rect(60, y, 150, 20), "", 32, true, NULL);
		i->SetText( game->mPlayerData.GetParamString(e, "id") );
		i->SetKeyFocus();
	y += 25;	
	
	new Label(this, "", rect(10,y), "Pass");
	i = new Input(this, "pass", rect(60, y, 150, 20), "", 32, true, NULL);
		i->SetText( game->mPlayerData.GetParamString(e, "pass") );
		i->mIsPassword = true;
	y += 25;

	//checkboxes to the left
	c = new Checkbox(this, "remember", rect(10,y), "Remember Me", 0);
		c->SetState( game->mPlayerData.GetParamInt(e, "remember") );
	y += 25;
	
	
	//bottom button set
	b = new Button(this, "register",rect(10,y,20,20), "", callback_LoginDialogRegister);
		b->mHoverText = "Register";
		b->SetImage("assets/buttons/login_register.png");
	
	b = new Button(this, "login",rect(160,y,20,20), "", callback_LoginDialogSendLogin);
		b->mHoverText = "Send Login";
		b->SetImage("assets/buttons/login_send.png");
	
	b = new Button(this, "skip",rect(190,y,20,20), "", callback_LoginDialogSkip);
		b->mHoverText = "Skip Login";
		b->SetImage("assets/buttons/login_skip.png");

	l = new Label(this, "status", rect(10,y), "Getting Server Verification...");
		l->SetVisible(false);

	y += 25;
	
	mText = new Multiline(gui, "", rect(10,SCREEN_HEIGHT - 150,350,120));
		mText->mHideScrollbar = true;
		resman->Unload(mText->mImage);
		mText->mImage = NULL;
		mText->mFont = fonts->Get("", 0, TTF_STYLE_BOLD); //load default font, but bold.
		mText->mFontColor = color(255,255,255);

	SetSize(220, y);
	ResizeChildren();

	if (game)
		game->SetVisible(false);
	
	loginDialog = this;
	
	mBackgroundImage = resman->LoadImg("assets/login.jpg");
	
	// match the first pixel in our background image
	gui->ColorizeGui( mBackgroundImage->GetPixel(0, 0) );
	
	AutoUpdater* au = new AutoUpdater();
	au->SendRequestForManifest();
}

LoginDialog::~LoginDialog()
{
	loginDialog = NULL;
	
	if (mText)
		mText->Die();
	
	if (game)
		game->SetVisible(true);
		
	resman->Unload(mBackgroundImage);
}

void LoginDialog::Render()
{	
	Image* scr = Screen::Instance();
	
	if (mBackgroundImage)
		mBackgroundImage->Render(scr, 0, 0);

	Frame::Render();
}

void LoginDialog::Skip()
{
	SendLoginQuery(true);
}

void LoginDialog::SendLogin()
{
	Input* i;
	Checkbox* c;
	
	i = (Input*)Get("id");
	if (i && i->GetText().empty())
	{
		new MessagePopup("", "Invalid Login", "You must input an ID");
		return;
	}

	c = (Checkbox*)Get("remember");
	if (c)
	{
		TiXmlElement* e = game->mPlayerData.mDoc.FirstChildElement("data")->FirstChildElement("login");
		
		game->mPlayerData.SetParamInt(e, "remember", c->GetState());
		
		if (c->GetState() == 1)
		{
			if (i)
				game->mPlayerData.SetParamString(e, "id", i->GetText());
			
			i = (Input*)Get("pass");
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
	string query;
	
	//send http get: login.php?ver=1.1.0&id=test&pass=test

	query = "http://sybolt.com/drm-svr/";
	query += "login.php?ver=";
	query += APP_VERSION;

	if (!skip)
	{
		i = (Input*)Get("id");
		mUsername = i->GetText();
		
		i = (Input*)Get("pass");
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
	
	//e = game->mPlayerData.mDoc.FirstChildElement("data")->FirstChildElement("map");
	//query += "&lm=" + game->mPlayerData.GetParamString(e, "lastid");
	
	downloader->QueueDownload(query, getTemporaryCacheFilename(),
									NULL, dlCallback_welcomeXmlSuccess,
									dlCallback_welcomeXmlFailure, true);
	SetControlState(false);
}

void LoginDialog::SetControlState(bool enabled)
{
	Get("login")->SetVisible(enabled);
	Get("skip")->SetVisible(enabled);
	Get("register")->SetVisible(enabled);
	Get("remember")->SetVisible(enabled);
	//Get("id")->SetActive(enabled);
	//Get("pass")->SetActive(enabled);
	
	Button* b = (Button*)Get("frameclose");
	if (b)
		b->SetActive(enabled);
		
	Get("status")->SetVisible(!enabled);
}
