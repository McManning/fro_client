
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


#include "core/Common.h"
#include "core/Screen.h"
#include "core/GuiManager.h"
#include "core/widgets/Checkbox.h"
#include "core/widgets/Frame.h"
#include "core/io/FileIO.h"
#include "core/Logger.h"
//#include "objecteditor/ObjectEditor.h"
//#include "mapeditor/MapEditorDialog.h"

//#define GUI_ONLY

#ifndef GUI_ONLY
#	include "game/GameManager.h"
#endif

/*	If the last time we ran this, it didn't shut down properly, log it */
void checkLastRun()
{
	FILE* f;

	if (fileExists("logs/marker"))
	{
	    logger.WriteError("Marker found");

		if (getFilesize("logs/info.log") > 0)
			copyFile("logs/info.log", "logs/crashlog_" + timestamp(true) + ".log");
	}
	else
	{
		f = fopen("logs/marker", "w");
		fprintf(f, ":P");
		fclose(f);
	}
}

int main (int argc, char *argv[])
{
	buildDirectoryTree("logs/");

	//freopen("logs/info.log", "w", stdout);

	removeFile("logs/time_profile.log");

	appState = APPSTATE_STARTING;

	if (!logger.Start())
        return 0;

	checkLastRun();

	try
	{
		Uint32 screenFlags = SDL_SWSURFACE | SDL_DOUBLEBUF;
		SetScreenFlags(	screenFlags );

		new GuiManager();

#ifndef GUI_ONLY
		DEBUGOUT("Booting up GM");
		new GameManager();
#else
		new Checkbox(gui, "", rect(20, 20), "OMFG CHECKBOX", 0);
		new Frame(gui, "", rect(20, 20, 200, 200), "This is a dialog", true, true, true, true);
#endif

	//	console->HookCommand("create_actor", callback_CreateActor);
		gui->MainLoop();
    }
    catch (std::exception& e)
	{
		logger.Write("[main] Std Exception: %s\n", e.what());
		systemErrorMessage("Std Exception", e.what());
    }
	catch (exception e)
	{
		logger.Write("[main] Exception: [%s] Code %i . %i\n", e.file, e.line, e.code);
		systemErrorMessage("Exception", "[" + string(e.file) + "] Code " + its(e.line) + "." + its(e.code)
							+ "\n\nREPORT THIS IMMEDIATELY!");
	}
	catch (const char* e)
	{
		logger.Write("[main] Const Exception: %s\n", e);
		systemErrorMessage("Const Exception ", string(e) + "\n\nREPORT THIS IMMEDIATELY!");
	}
    catch (...)
	{
		logger.Write("[main] Unknown Exception\n");
		systemErrorMessage("Exception", "Unknown Error\n\nREPORT THIS IMMEDIATELY!");
	}

	appState = APPSTATE_CLOSING; //in case it wasn't set elsewhere
	delete gui;

	// Avoid reporting that the shutdown was a failure
	removeFile("logs/marker");

    logger.Close();

	return 0;
}
