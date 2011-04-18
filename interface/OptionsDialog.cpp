
#include "OptionsDialog.h"

#include "../core/widgets/Button.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/Checkbox.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/Scrollbar.h"
#include "../core/widgets/SmallSelect.h"

#include "../core/net/IrcNet2.h"
#include "../core/sound/SoundManager.h"
#include "../game/GameManager.h"
#include "../entity/LocalActor.h"
#include "../map/Map.h"

#include "../interface/UserList.h"

void callback_optionsDialogSave(Button* b)
{
	OptionsDialog* o = (OptionsDialog*)b->GetParent();
	o->Save();
	o->Die();
}

void callback_optionsDialogTab(Button* b)
{
	OptionsDialog* o = (OptionsDialog*)b->GetParent();
	o->Toggle(b->mId);
}

OptionsDialog::OptionsDialog() :
	Frame(gui, "optionsdialog", rect(), "Options", true, false, true, true)
{	
	Button* b;
	int x;
	
	SetSize(265, 185);
	Center();

//Create section frames
	rect r(10,55,mPosition.w - 20, mPosition.h - 55);
	
	mFrameUser = new Frame(this, "User", r);
	mFrameNetwork = new Frame(this, "Network", r);
	mFrameAudio = new Frame(this, "Audio", r);
	mFrameGraphics = new Frame(this, "Graphics", r);
	
	//in case something else screwed with it
	game->mPlayerData.mXmlPos = game->mPlayerData.mDoc.FirstChildElement("data");
	
	_buildFrameUser();
	_buildFrameNetwork();
	_buildFrameAudio();
	_buildFrameGraphics();

	Toggle("User");
	
//Create toggle buttons
	x = 10;
	b = new Button(this, "User",rect(x,30,20,20), "", callback_optionsDialogTab);
		b->mHoverText = "User";
		b->SetImage("assets/buttons/options_user.png");
	x += 25;
	
	b = new Button(this, "Network",rect(x,30,20,20), "", callback_optionsDialogTab);
		b->mHoverText = "Network";
		b->SetImage("assets/buttons/options_network.png");
	x += 25;
	
	b = new Button(this, "Audio",rect(x,30,20,20), "", callback_optionsDialogTab);
		b->mHoverText = "Audio";
		b->SetImage("assets/buttons/options_audio.png");
	x += 25;
	
	b = new Button(this, "Graphics",rect(x,30,20,20), "", callback_optionsDialogTab);
		b->mHoverText = "Graphics";
		b->SetImage("assets/buttons/options_video.png");
	x += 25;
	
//Create save button
	b = new Button(this, "save",rect(Width()-30,Height()-30,20,20), "", callback_optionsDialogSave);
		b->mHoverText = "Save Options";
		b->SetImage("assets/buttons/okay.png");

	ResizeChildren();
}

OptionsDialog::~OptionsDialog()
{

}

void OptionsDialog::Render()
{
	//if (gui->GetDemandsFocus() == this)
	//	gui->RenderDarkOverlay();
		
	Frame::Render();
}

void OptionsDialog::Toggle(string id)
{
	mFrameUser->SetVisible( (id == mFrameUser->mId)  );
	mFrameNetwork->SetVisible( (id == mFrameNetwork->mId) );
	mFrameAudio->SetVisible( (id == mFrameAudio->mId)  );
	mFrameGraphics->SetVisible( (id == mFrameGraphics->mId) );
}

void OptionsDialog::_buildFrameUser()
{
	Input* i;
	SmallSelect* ss;
	Checkbox* c;
	int y = 0;

	mFrameUser->mSortable = false;

	new Label(mFrameUser, "", rect(0,y), "Nickname");
	i = new Input(mFrameUser, "nick", rect(95, y, 150, 20), "", 32, true, NULL);
		i->SetText( game->mPlayer->mName );
		//i->mHoverText = "No spaces.";
	y += 25;
	
	new Label(mFrameUser, "", rect(0,y), "Alerts");
	ss = new SmallSelect(mFrameUser, "alerts", rect(95, y, 150, 20), NULL);
		ss->AddItem("Off");
		ss->AddItem("Always");
		ss->AddItem("Only When Inactive");
		ss->mSelectedIndex = config.GetParamInt("system", "alerts");
		ss->mHoverText = "Controls when to flash the title bar for a new message";
	y += 25;
	
/*	c = new Checkbox(mFrameUser, "names", rect(0,y), "Show Player Names", 0);
		c->SetState( game->mPlayerData.GetParamInt("map", "shownames") );
	y += 25;
		
	c = new Checkbox(mFrameUser, "trading", rect(0,y), "Allow Trade Requests", 0);
		c->SetState( game->mPlayerData.GetParamInt("map", "trading") );
	y += 25;
*/

	c = new Checkbox(mFrameUser, "stamps", rect(0,y), "Show Timestamps", 0);
		c->SetState( config.GetParamInt("console", "timestamps") );
	y += 25;
}

void OptionsDialog::_buildFrameNetwork()
{
	Input* i;
	Checkbox* c;
	int y = 0;
	
	mFrameNetwork->mSortable = false;

/*
	new Label(mFrameNetwork, "", rect(0,y), "Reconnect");
	i = new Input(mFrameNetwork, "reconnect", rect(95, y, 150, 20), "0123456789", 0, true, NULL);
		i->SetText( "0" ); //game->mConfig.GetParamString("connection", "delay") );
		i->mHoverText = "Delay (in minutes) between reconnect attempts. 0 to disable.";
	y += 25;
*/
	
	c = new Checkbox(mFrameNetwork, "joinparts", rect(0,y), "Show Joins/Parts", 0);
		c->SetState( game->mShowJoinParts );
	y += 25;
	
	c = new Checkbox(mFrameNetwork, "addr", rect(0,y), "Show Addresses In Joins/Parts", 0);
		c->SetState( game->mShowAddresses );
	y += 25;
	
	c = new Checkbox(mFrameNetwork, "privmsg", rect(0,y), "Accept Private Messages", 0);
		c->SetState( game->mPlayerData.GetParamInt("map", "privmsg") );
	y += 25;

}

void OptionsDialog::_buildFrameAudio()
{
	Scrollbar* sc;
	Checkbox* c;
	int y = 0;
	
	mFrameAudio->mSortable = false;
	
	new Label(mFrameAudio, "", rect(0,y), "Volume");
	sc = new Scrollbar(mFrameAudio, "vol", rect(95, y, 150, 20), HORIZONTAL, MAX_VOLUME, 10, 0, NULL);
		sc->SetValue( config.GetParamInt("sound", "volume") );
	y += 25;
	
	c = new Checkbox(mFrameAudio, "", rect(0,y), "Enable Voice Chat", 0);
		c->SetState( 0 );
		c->mHoverText = "Haha, you wish";
	y += 25;

}

void OptionsDialog::_buildFrameGraphics()
{
	Input* i;
	Checkbox* c;
	int y = 0;

	mFrameGraphics->mSortable = false;
	
	new Label(mFrameGraphics, "", rect(0,y), "FPS Cap");
	i = new Input(mFrameGraphics, "fps", rect(95, y, 150, 20), "0123456789", 0, true, NULL);
		i->SetText( config.GetParamString("system", "fps") );
	y += 25;

	c = new Checkbox(mFrameGraphics, "lowcpu", rect(0,y), "Use Lower Cpu When Inactive", 0);
		c->SetState( config.GetParamInt("system", "lowcpu") );
	y += 25;
		
	c = new Checkbox(mFrameGraphics, "nolimit", rect(0,y), "Max Speed (Not Recommended)", 0);
		c->SetState( config.GetParamInt("system", "nolimit") );
		c->mHoverText = "Seriously, this is a 2D chat room. Why have 4000 FPS?";
	y += 25;	
}

void OptionsDialog::Save()
{
	//save settings to xml, set shit, etc. blah blah
	Checkbox* c;
	Input* i;
	SmallSelect* ss;
	Scrollbar* sc;
	string s;
	uShort u;
	
	i = (Input*)mFrameUser->Get("nick");
	if (i)
	{
		s = i->GetText();
		if (s.find(" ", 0) != string::npos)
			s.erase(s.find(" ", 0));

		replace(&s, "\\n", ""); //filter out \n 
		if (!stripCodes(s).empty())
		{
			if (game->mNet->IsConnected())
				game->mNet->ChangeNick(s);
			else
			{
				if (userlist)
					userlist->ChangeNick(game->mPlayer->mName, s);
				game->mPlayer->mName = s;
			}
				
			TiXmlElement* e = game->mPlayerData.mDoc.FirstChildElement("data")->FirstChildElement("user");
			game->mPlayerData.SetParamString(e, "nick", s);
			game->SavePlayerData();
		}
		else
		{
			game->GetChat()->AddMessage("\\c900 * Invalid nickname, ignoring.");
		}
	}

	c = (Checkbox*)mFrameUser->Get("stamps");
	if (c)
	{
		config.SetParamInt("console", "timestamps", c->GetState());
	}
			
	c = (Checkbox*)mFrameUser->Get("names");
	if (c) 
	{
		game->mPlayerData.SetParamInt("map", "shownames", c->GetState());
		if (game->mMap)
			game->mMap->mShowPlayerNames = c->GetState();
	}
		
	c = (Checkbox*)mFrameUser->Get("trading");
	if (c) 
	{
		game->mPlayerData.SetParamInt("map", "trading", c->GetState());
	}	
	
	ss = (SmallSelect*)mFrameUser->Get("alerts");
	if (ss) 
		config.SetParamInt("system", "alerts", ss->mSelectedIndex);

	c = (Checkbox*)mFrameNetwork->Get("joinparts");
	if (c)
	{
		game->mPlayerData.SetParamInt("map", "joinparts", c->GetState());
		game->mShowJoinParts = c->GetState();
	}
	
	c = (Checkbox*)mFrameNetwork->Get("addr");
	if (c)
	{
		game->mPlayerData.SetParamInt("map", "addresses", c->GetState());
		game->mShowAddresses = c->GetState();
	}

/*
	i = (Input*)mFrameNetwork->Get("reconnect");
	if (i)
	{
		game->mConfig.SetParamInt("connection", "delay", sti(i->GetText()));
		//TODO: this
	}
*/				
	c = (Checkbox*)mFrameNetwork->Get("privmsg");
	if (c)
	{
		game->mPlayerData.SetParamInt("map", "privmsg", c->GetState());
	}
		
	sc = (Scrollbar*)mFrameAudio->Get("vol");
	if (sc) 
	{
		config.SetParamInt("sound", "volume", sc->GetValue());
		if (sound)
			sound->SetVolume( sc->GetValue() );
	}
	
	c = (Checkbox*)mFrameGraphics->Get("nolimit");
	if (c)
	{
		config.SetParamInt("system", "nolimit", c->GetState());
		gui->mNoFpsLimit = c->GetState();
	}
	
	c = (Checkbox*)mFrameGraphics->Get("lowcpu");
	if (c)
	{
		config.SetParamInt("system", "lowcpu", c->GetState());
		gui->mUseLowCpu = c->GetState();
	}

	i = (Input*)mFrameGraphics->Get("fps");
	if (i)
	{
		u = sti(i->GetText());
		if (u < 5)
			u = 5;
		config.SetParamInt("system", "fps", u);
		gui->mFpsCap = u;
	}

	game->SavePlayerData();
	config.SaveToFile(CONFIG_FILENAME);
}


