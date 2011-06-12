
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

void dlCallback_welcomeDataSuccess(downloadData* data)
{
	vString lines;
	string hash;
	bool isDone = false;
	
	if (loginDialog)
	{
		fileTovString(lines, data->filename, "\n");
		
		for (int i = 0; i < lines.size(); ++i)
		{
			replace(&lines.at(i), "\r", "");
			if (lines.at(i).find("error:", 0) == 0)
			{
				new MessagePopup("", "Login Error", lines.at(i).substr(6), false);	
				game->mUsername.clear();
				game->mPassword.clear();

				loginDialog->SetControlState(true);
				isDone = true;
			}
			else if (lines.at(i).find("manifest:", 0) == 0)
			{
#ifndef DEBUG
				hash = md5file(DIR_CACHE "manifest.res");
				// if our hashes don't match, trigger an update
				if (hash != lines.at(i).substr(9))
				{
					printf("Hash %s doesn't match\n", hash.c_str());
					loginDialog->SetControlState(false);
					AutoUpdater* au = new AutoUpdater();
					au->SendRequestForManifest();
					isDone = true;
				}
#endif
			}
			else if (lines.at(i).find("server:", 0) == 0)
			{
				game->mNet->mServerList.push_back(lines.at(i).substr(7));
			}
			else if (lines.at(i) == "OK" && !isDone) // good to go!
			{				
				game->mUsername = loginDialog->mUsername;
				game->mPassword = loginDialog->mPassword;
		
				if (!game->mUsername.empty())
					game->mNet->mRealname = game->mUsername;
					
				startCheckInTimer();
					
				if (!isDone)
				{
					game->mNet->TryNextServer();
					isDone = true;	
				}
			}
		}
		
		if (!isDone)
		{
			new MessagePopup("", "Connection Error", "Server sent invalid login data.", false);	
			loginDialog->SetControlState(true);
		}	
	}
	
	removeFile(data->filename);
}

void dlCallback_welcomeDataFailure(downloadData* data)
{
	string error;
	
	switch (data->errorCode)
	{
		case DEC_BADHOST:
			error = "Server address is invalid.";
			break;
		case DEC_CONNECTFAIL:
			error = "Could not connect to login server.";
			break;
		case DEC_FILENOTFOUND:
			error = "Could not cache login data.";
			break;
		default:
			error = "The gnomes broke something.";
			break;
	}

	console->AddMessage(error);
	new MessagePopup("", "Connection Error", error);
	
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
				"\tTo register, or read about the benefits of registration, right click the following link: \n\n" 		
				"\\c008http://sybolt.com/drm/register.php";

	new MessagePopup("register", "Register", msg, true);	
}

LoginDialog::LoginDialog() :
	Frame(gui, "LoginDialog", rect(50,50), "Login to Sybolt", true, false, false, true)
{
	mSortable = false;
	
	ASSERT(!loginDialog); //there can be only one
	
	uShort y = 30;

	Input* i;
	Button* b;
	Checkbox* c;
	Label* l;

	// TODO: Pull login creds out somewhere else, locally. That WON'T get uploaded to sybolt

	new Label(this, "", rect(10,y), "ID");
	i = new Input(this, "id", rect(60, y, 150, 20), "", 32, true, NULL);
		i->SetText( game->mUserData.GetValue("Login", "ID") );
		i->SetKeyFocus();
	y += 25;	
	
	new Label(this, "", rect(10,y), "Pass");
	i = new Input(this, "pass", rect(60, y, 150, 20), "", 32, true, NULL);
		i->SetText( game->mUserData.GetValue("Login", "Password") );
		i->mIsPassword = true;
	y += 25;

	//checkboxes to the left
	c = new Checkbox(this, "remember", rect(10,y), "Remember Me", 0);
		c->SetState( sti(game->mUserData.GetValue("Login", "Remember")) );
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
		if (c->GetState() == 1)
		{
			
			game->mUserData.SetValue("Login", "Remember", "1");
			game->mUserData.SetValue("Login", "ID", i->GetText());
			
			i = (Input*)Get("pass");
			game->mUserData.SetValue("Login", "Password", i->GetText());
		}
		else
		{
			game->mUserData.SetValue("Login", "Remember", "0");
			game->mUserData.SetValue("Login", "ID", "");
			game->mUserData.SetValue("Login", "Password", "");
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
	query += VER_STRING;

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
									NULL, dlCallback_welcomeDataSuccess,
									dlCallback_welcomeDataFailure, true);
	SetControlState(false);
}

void LoginDialog::SetControlState(bool enabled)
{
	Get("login")->SetVisible(enabled);
	Get("skip")->SetVisible(enabled);
	Get("register")->SetVisible(enabled);
	
	Widget* w;
	
	w = Get("remember");
	if (w)
		w->SetVisible(enabled);
	//Get("id")->SetActive(enabled);
	//Get("pass")->SetActive(enabled);
	
	Button* b = (Button*)Get("frameclose");
	if (b)
		b->SetActive(enabled);
		
	Get("status")->SetVisible(!enabled);
}
