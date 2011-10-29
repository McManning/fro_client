
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


#include "OptionsDialog.h"

#include "../core/widgets/Button.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/Checkbox.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/Scrollbar.h"
#include "../core/widgets/SmallSelect.h"

#include "../core/net/IrcNet2.h"
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

OptionsDialog::OptionsDialog()
	: Frame(gui, "OptionsDialog", rect(), "Options", true, false, true, true)
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
	
/*	b = new Button(this, "Audio",rect(x,30,20,20), "", callback_optionsDialogTab);
		b->mHoverText = "Audio";
		b->SetImage("assets/buttons/options_audio.png");
	x += 25;
*/	
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
	i = new Input(mFrameUser, "nick", rect(95, y, 150, 20), 
					"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\\_-^`[]{}|", 
					32, true, NULL);
		i->SetText( game->mPlayer->GetName() );
	y += 25;
	
	new Label(mFrameUser, "", rect(0,y), "Alerts");
	ss = new SmallSelect(mFrameUser, "alerts", rect(95, y, 150, 20), NULL);
		ss->AddItem("Off");
		ss->AddItem("Always");
		ss->AddItem("Only When Inactive");
		ss->mSelectedIndex = gui->mSystemAlertType;
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
		c->SetState( sti(game->mUserData.GetValue("MapSettings", "Timestamps")) );
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
		c->SetState( sti(game->mUserData.GetValue("MapSettings", "PrivMsg")) );
	y += 25;

}

void OptionsDialog::_buildFrameAudio()
{
#ifdef _SOUNDMANAGER_H_
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
#endif
}

void OptionsDialog::_buildFrameGraphics()
{
	Input* i;
	Checkbox* c;
	int y = 0;

	mFrameGraphics->mSortable = false;
	
	new Label(mFrameGraphics, "", rect(0,y), "FPS Cap");
	i = new Input(mFrameGraphics, "fps", rect(95, y, 150, 20), "0123456789", 0, true, NULL);
		i->SetText( its(gui->mFpsCap) );
	y += 25;

	c = new Checkbox(mFrameGraphics, "lowcpu", rect(0,y), "Use Lower Cpu When Inactive", 0);
		c->SetState( gui->mUseLowCpu );
	y += 25;
		
	c = new Checkbox(mFrameGraphics, "nolimit", rect(0,y), "Max Speed (Not Recommended)", 0);
		c->SetState( gui->mNoFpsLimit );
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
	
		// nope!
		replace(&s, "\\n", "");
		replace(&s, "\\t", "");
		replace(&s, "\n", "");
		replace(&s, "\t", "");

		if (!stripCodes(s).empty())
		{
			if (game->mNet->IsConnected())
			{
				game->mNet->ChangeNick(s);
			}
			else
			{
				if (userlist)
					userlist->ChangeNick(game->mPlayer->GetName(), s);
				game->mPlayer->SetName(s);

				game->mUserData.SetValue("MapSettings", "Nick", s);
			}
		}
		else
		{
			game->GetChat()->AddMessage("\\c900 * Invalid nickname, ignoring.");
		}
	}

	c = (Checkbox*)mFrameUser->Get("stamps");
	if (c)
	{
		game->mUserData.SetValue("MapSettings", "Timestamps", its(c->GetState()));
		if (game->GetChat())
			game->GetChat()->mShowTimestamps = c->GetState();
	}
			
	c = (Checkbox*)mFrameUser->Get("names");
	if (c) 
	{
		game->mUserData.SetValue("MapSettings", "ShowNames", its(c->GetState()));
		if (game->mMap)
			game->mMap->mShowPlayerNames = c->GetState();
	}
/*
	c = (Checkbox*)mFrameUser->Get("trading");
	if (c) 
	{
		game->mPlayerData.SetParamInt("map", "trading", c->GetState());
	}	
*/
	ss = (SmallSelect*)mFrameUser->Get("alerts");
	if (ss) 
	{
		game->mUserData.SetValue("System", "Alerts", its(ss->mSelectedIndex));
		gui->mSystemAlertType = ss->mSelectedIndex;
	}
	
	c = (Checkbox*)mFrameNetwork->Get("joinparts");
	if (c)
	{
		game->mUserData.SetValue("MapSettings", "JoinParts", its(c->GetState()));
		game->mShowJoinParts = c->GetState();
	}
	
	c = (Checkbox*)mFrameNetwork->Get("addr");
	if (c)
	{
		game->mUserData.SetValue("MapSettings", "ShowAddresses", its(c->GetState()));
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
		game->mUserData.SetValue("MapSettings", "PrivMsg", its(c->GetState()));
	}
	
#ifdef _SOUNDMANAGER_H_		
	sc = (Scrollbar*)mFrameAudio->Get("vol");
	if (sc) 
	{
		config.SetParamInt("sound", "volume", sc->GetValue());

		if (sound)
			sound->SetVolume( sc->GetValue() );

	}
#endif	
	c = (Checkbox*)mFrameGraphics->Get("nolimit");
	if (c)
	{
		game->mUserData.SetValue("System", "NoLimit", its(c->GetState()));
		gui->mNoFpsLimit = c->GetState();
	}
	
	c = (Checkbox*)mFrameGraphics->Get("lowcpu");
	if (c)
	{
		game->mUserData.SetValue("System", "LowCpu", its(c->GetState()));
		gui->mUseLowCpu = c->GetState();
	}

	i = (Input*)mFrameGraphics->Get("fps");
	if (i)
	{
		u = sti(i->GetText());
		if (u < 5)
			u = 5;
		game->mUserData.SetValue("System", "FPS", its(u));
		gui->mFpsCap = u;
	}
}


