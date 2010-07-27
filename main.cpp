
#include "core/Common.h"
#include "core/Screen.h"
#include "core/GuiManager.h"
#include "core/widgets/Checkbox.h"
#include "core/widgets/Frame.h"
#include "core/io/FileIO.h"
#include "core/io/XmlFile.h"
//#include "objecteditor/ObjectEditor.h"
//#include "mapeditor/MapEditorDialog.h"

//#define GUI_ONLY

#ifndef GUI_ONLY
#	include "game/GameManager.h"
#	include "game/TerrainTest.h"
#endif

/*	If the last time we ran this, it didn't shut down properly, log it */
void checkLastRun()
{
	FILE* f;

	if (fileExists("logs/marker"))
	{
		f = fopen("logs/error.log", "a");
		if (f)
		{
			fprintf(f, "[%s] MARKER\n", timestamp(true).c_str());
			fclose(f);
		}
		if (getFilesize("logs/out.log") > 0)
			copyFile("logs/out.log", "logs/crashlog_" + timestamp(true) + ".log");
	}
	else
	{
		f = fopen("logs/marker", "w");
		fprintf(f, ":P");
		fclose(f);	
	}
}

/*	If we have an updater_update.exe, replace updater.exe and erase */
void checkForUpdaterUpdate()
{
	if (fileExists("updater_update.exe"))
	{
		copyFile("updater_update.exe", "updater.exe");
		removeFile("updater_update.exe");	
	}
}

//TODO: A main that doesn't look SO FUCKING BAD. 
int main (int argc, char *argv[])
{
	buildDirectoryTree("logs/");

	freopen("logs/out.log", "w", stdout);
	removeFile("logs/time_profile.log");

	checkLastRun();
	checkForUpdaterUpdate();
	
	appState = APPSTATE_STARTING;

	printf("[main] Starting app at %s.\n", timestamp(true).c_str());
	try
	{
		Uint32 screenFlags = SDL_SWSURFACE | SDL_DOUBLEBUF;
		SetScreenFlags(	screenFlags );

#ifndef GUI_ONLY
       	if (argc > 1) //argv[0] is full application path
       	{
			if (strcmp(argv[1], "-terra") == 0)
			{
				new GuiManager();
				new TerrainTest();
			}
		}
		else //no arguments, run natural game
		{
			new GuiManager();
			
			PRINT("Booting up GM");
			new GameManager();
		}
		
#else
		new GuiManager();
		
		new Checkbox(gui, "", rect(20, 20), "OMFG CHECKBOX", 0);
		new Frame(gui, "", rect(20, 20, 200, 200), "This is a dialog", true, true, true, true);
#endif		

	//	console->HookCommand("create_actor", callback_CreateActor);
		gui->MainLoop();
    }
    catch (std::exception& e)
	{
		printf("[main] Std Exception: %s\n", e.what());
		systemErrorMessage("Std Exception", e.what());
    }
	catch (exception e)
	{
		printf("[main] Exception: [%s] Code %i . %i\n", e.file, e.line, e.code);
		systemErrorMessage("Exception", "[" + string(e.file) + "] Code " + its(e.line) + "." + its(e.code)
							+ "\n\nREPORT THIS IMMEDIATELY!");
	}
	catch (const char* e)
	{
		printf("[main] Const Exception: %s\n", e);
		systemErrorMessage("Const Exception ", string(e) + "\n\nREPORT THIS IMMEDIATELY!");
	}
    catch (...)
	{
		printf("[main] Unknown Exception\n");
		systemErrorMessage("Exception", "Unknown Error\n\nREPORT THIS IMMEDIATELY!");
	}

	appState = APPSTATE_CLOSING; //in case it wasn't set elsewhere
	delete gui;

	// Avoid reporting that the shutdown was a failure
	removeFile("logs/marker");

	printf("[main] Closing app at %s, have a nice day.\n", timestamp(true).c_str());

	return 0;
}
