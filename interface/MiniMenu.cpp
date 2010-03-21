
#include "MiniMenu.h"
#include "OptionsDialog.h"
#include "LoginDialog.h"
#include "../core/widgets/Button.h"
#include "../core/net/IrcNet2.h"
#include "../game/GameManager.h"

void callback_MiniMenuOptions(Button* b)
{
	b->GetParent()->Die();
	if (!gui->Get("optionsdialog"))
		new OptionsDialog();
}

void callback_MiniMenuToLogin(Button* b)
{
	b->GetParent()->Die();
	if (!loginDialog)
	{
		//game->UnloadWorld();
		//new LoginDialog();
		game->UnloadWorld();
		game->mNet->Quit();
		new LoginDialog();
	}
}

/*void callback_MiniMenuToggleConsole(Button* b)
{
	b->GetParent()->Die();
	console->SetVisible(!console->IsVisible());
}*/

void callback_MiniMenuQuit(Button* b)
{
	b->GetParent()->Die();
	appState = APPSTATE_CLOSING;
}

MiniMenu::MiniMenu()
	: Frame(gui, "MiniMenu", rect(0,0,200,100), "Menu", true, false, true, true)
{
	Button* b;

	rect r(10, 30, Width()-20, 20);
	
	b = new Button(this, "", r, "Options", callback_MiniMenuOptions);
	r.y += 25;

	b = new Button(this, "", r, "Back To Login", callback_MiniMenuToLogin);
	r.y += 25;

	//b = new Button(this, "", r, "Toggle Console", callback_MiniMenuToggleConsole);
	//r.y += 25;
	
	b = new Button(this, "", r, "Quit", callback_MiniMenuQuit);
	r.y += 25;

	r.y += 5;
	
	SetSize(Width(), r.y);
	ResizeChildren();
	Center();
	
	DemandFocus();
}




