
#include <lua.hpp>
#include "ConvoLib.h"
#include "LuaCommon.h"
#include "../game/GameManager.h"
#include "../map/Map.h"
#include "../core/widgets/Frame.h"
#include "../core/widgets/Multiline.h"

/***********************************************
 * Convo dialog class controlled by lua
 ***********************************************/

/*
	Lua callback format:
	function callback(convo, selectedOptionText, userdata)
		blah blah
	end
*/

#define CHOICE_WIDTH 250

class ConvoDialog : public Frame
{
  public:
	ConvoDialog(string title);
	~ConvoDialog();

	void SetPosition(rect r);
	void ResizeChildren();
	void Render();
	
	//called when an option is selected. Returns whatever lua returns, or 0 if an error occured.
	int DoOption(sShort index);

	void Clear();
	void AddOption(string text, string luaFunction);
	void SetText(string text);
	
	lua_State* luaState;
	int luaReference; //index of our linked userdata in the lua registry
	vString luaFunctions;
	
	Multiline* mList;
	Multiline* mText;
};

ConvoDialog* convo;

void callback_convoDialogOption(Multiline* m)
{
	if (m->mSelected > -1 && m->mSelected < m->mLines.size() && convo)
	{
		convo->DoOption(m->mSelected);
	}
}

ConvoDialog::ConvoDialog(string title)
	: Frame(gui, "ConvoDialog", rect(), title, true, true, false, true)
{
	luaState = NULL;
	luaReference = LUA_NOREF;

	uShort y = 30;

	mText = new Multiline(this, "", rect());
	mText->SetPosition( rect(10, y, CHOICE_WIDTH - 20, mText->mFont->GetHeight() * 7 + 5) );
	y += mText->Height() + 10;	

	mList = makeList(this, "", rect());
	mList->SetPosition( rect(10, y, CHOICE_WIDTH - 20, mList->mFont->GetHeight() * 5 + 5) );
	mList->onLeftSingleClickCallback = callback_convoDialogOption;
	mList->mSelectOnHover = true;
	y += mList->Height() + 10;

	SetSize(CHOICE_WIDTH, y);
//	DemandFocus();
	ResizeChildren();
	Center();
	
	if (game)
		game->ToggleHud(false);
	
	convo = this;
}

ConvoDialog::~ConvoDialog()
{
	if (luaState)
		luaL_unref(luaState, LUA_REGISTRYINDEX, luaReference);
	
	//if (game && !game->IsInDuel())
	//	game->ToggleHud(true);
	
	convo = NULL;
}

void ConvoDialog::ResizeChildren() //overridden so we can move things around properly
{
	mText->SetPosition( rect(10, 30, Width() - 20, Height() - mList->Height() - 45) );
	mList->SetPosition( rect(10, mText->Height() + 35, Width() - 20, mList->Height() ) );
	mList->SetTopLine(0);
	mText->SetTopLine(0);
	
	Frame::ResizeChildren(); //takes care of titlebar stuff (close button, sizer, caption, etc)
}

void ConvoDialog::SetPosition(rect r)
{
	if (r.w >= 250 && r.h >= 200)
		Frame::SetPosition(r);
}

void ConvoDialog::Render()
{	
//	if (gui->GetDemandsFocus() == this)
//		gui->RenderDarkOverlay();

	Frame::Render();
}

int ConvoDialog::DoOption(sShort index)
{
	string s = mList->GetSelectedText();
	string luaFunction = luaFunctions.at(index);

	if (!luaState) return 0;

	lua_getglobal(luaState, luaFunction.c_str()); //get function name

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(luaState, -1))
	{
		//cannot be luaError because we need to return!
		WARNING("Lua Function " + luaFunction + " not found during Convo::DoOption");
		return 0;
	}

	lua_pushlightuserdata(luaState, this);
	lua_pushstring(luaState, s.c_str());
	lua_rawgeti(luaState, LUA_REGISTRYINDEX, luaReference);

	int result = 0;
	if (lua_pcall(luaState, 3, 1, 0) != 0)
	{
		console->AddMessage("\\c900 * LUACONVO [" + luaFunction + "] " + string(lua_tostring(luaState, -1)));
        result = 0;
	}
	else
	{
		if (lua_isnumber(luaState, -1))
		{
			result = (int)lua_tonumber(luaState, -1); //get the result
		}
	}

	lua_pop(luaState, 1); //get rid of result from stack
	return result;
}

void ConvoDialog::Clear()
{
	luaFunctions.clear();
	mList->Clear();
}

void ConvoDialog::AddOption(string text, string luaFunction)
{
	mList->AddMessage(text);
	mList->SetTopLine(0);
	luaFunctions.push_back(luaFunction);
}

void ConvoDialog::SetText(string text)
{
	mText->Clear();
	mText->AddMessage(text);
	mText->SetTopLine(0);
}

void _checkConvo(lua_State* ls)
{
	if (!convo)
	{
		string err = "No current Convo";
		lua_pushstring( ls, err.c_str() );
		lua_error( ls );
	}
}

/***********************************************
 * Lua Functions
 ***********************************************/
 
// .New("title") - Creates a new conversation dialog and returns it
int convo_New(lua_State* ls)
{
	luaCountArgs(ls, 1);
	
	if (!lua_isstring(ls, 1))
		return luaError(ls, "Convo.New", "Bad Params");
	
	if (convo != NULL)
	{
		lua_pushboolean(ls, false);
	}
	else
	{
		convo = new ConvoDialog(lua_tostring(ls, 1));
		convo->luaState = ls;
		
		lua_pushboolean(ls, true);
	}
	return 1;
}

// .SetUserdata(userdata) - Sets data associated with this convo dialog instance
// Accepts strings, numbers, or c pointers
int convo_SetUserdata(lua_State* ls)
{
	_checkConvo(ls);
	luaCountArgs(ls, 1);

	lua_pushvalue(ls, 1); //copy the value at index to the top of the stack
	convo->luaReference = luaL_ref(ls, LUA_REGISTRYINDEX); //create a reference to the stack top and 

	return 0;
}

// .GetUserdata() - Returns associated user data. Either a string, number, or c ptr
int convo_GetUserdata(lua_State* ls)
{
	_checkConvo(ls);

	//Push our reference to a lua object
	lua_rawgeti(ls, LUA_REGISTRYINDEX, convo->luaReference);
	
	return 1;
}

// .SetText("message")
int convo_SetText(lua_State* ls)
{
	_checkConvo(ls);
	luaCountArgs(ls, 1);
	
	if (!lua_isstring(ls, 1))
		return luaError(ls, "Convo.SetText", "Bad Params");

	convo->SetText( lua_tostring(ls, 1) );
		
	return 0;
}

// .AddOption("text", "lua_callback") - Add a selectable option that'll do the call when chosen
int convo_AddOption(lua_State* ls)
{
	_checkConvo(ls);
	luaCountArgs(ls, 2);
	
	if (!lua_isstring(ls, 1) || !lua_isstring(ls, 2))
		return luaError(ls, "Convo.AddOption", "Bad Params");

	convo->AddOption( lua_tostring(ls, 1), lua_tostring(ls, 2) );
	
	return 0;
}

// .Clear() - Clear all options of the convo
int convo_Clear(lua_State* ls)
{
	_checkConvo(ls);
	convo->Clear();
	return 0;
}

// .Close() - Close the specified convo dialog
int convo_Close(lua_State* ls)
{
	_checkConvo(ls);
	convo->Die();
	return 0;
}

static const luaL_Reg functions[] = {
	{"New", convo_New},
	{"SetUserdata", convo_SetUserdata},
	{"GetUserdata", convo_GetUserdata},
	{"SetText", convo_SetText},
	{"AddOption", convo_AddOption},
	{"Clear", convo_Clear},
	{"Close", convo_Close},
	{NULL, NULL}
};

void RegisterConvoLib(lua_State* ls)
{
	luaL_register( ls, "Convo", functions );
	convo = NULL;
}

void UnregisterConvoLib(lua_State* ls)
{
	if (convo)
	{
		convo->Die();	
		convo->luaState = NULL; //the state will be unloaded right after Unregister.. ends
	}
	convo = NULL;
}

