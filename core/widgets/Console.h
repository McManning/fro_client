
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "Frame.h"

/*
	Works like this:
		User inputs: var_x
		Console searches for matching command: var_x
		It checks the data members for non-null values,
		and outputs them, or calls the linked callback, etc.
*/
class Console;
struct consoleCommand
{
	consoleCommand()
        : callback(NULL), link(NULL), type(VOIDPTR)
	{
	};

	typedef enum
	{
		INT = 0,
		DOUBLE,
		STRING,
		VOIDPTR,
		RECT,
		POINT2D
	} commandType;

	string cmd;

	//Various events that could occur when the command is triggered
	//TODO: Combine these, so it can only hold ONE. Whatever.. type that was.
	void (*callback)(Console*, string);
	void* link; //variable linked
	commandType type;
};

const int CONSOLE_HEADER_HEIGHT = 16;
const int CONSOLE_FOOTER_NO_INPUT_HEIGHT = 16;
const int CONSOLE_FOOTER_WITH_INPUT_HEIGHT = 26;

class Input;
class Multiline;
class Console : public Frame
{
  public:
	//("assets/console.png", "log_", false, onInput);
	Console(string id, string title, string savePrefix, color c,
				bool hasExit, bool hasInput, bool hasExtraControls);
	~Console();

	void Render();
	void Event(SDL_Event* event);

	void SetPosition(rect r);
	void ResizeChildren();

	void AddMessage(string msg);

	/*	Will list all commands that start with the text in the input.
		If input is empty, will list all commands.
		TODO: How will I implement this?
	*/
	void ShowCommands();

	void HookCommand(string cmd, void (*callback)(Console*, string));
	void HookCommand(string cmd, consoleCommand::commandType type, void* link);

	void UnhookCommand(string cmd);
	void DoCommand(string s);

	void SaveText();

	void SetTitle(string s);

	Input* mInput;
	Multiline* mOutput;
	Label* mTitle;
	Button* mExit;
	Button* mBackgroundToggle;
//	Button* mFontSizeChange;

	Image* mBackground; //background of the output
	Image* mBackgroundImage; //will be sliced up for the main background

	color mBackgroundColor;

	string mSavePrefix; //when saving log files

	bool mShowTimestamps;
	bool mDrawBackground;

  private:
	void _runHookedCommand(consoleCommand& c, string s);

	std::vector<consoleCommand> mCommandHooks;
};

extern Console* console;

#endif //_CONSOLE_H_
