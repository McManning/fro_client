
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

//TODO: A main that doesn't look SO FUCKING BAD. 
int main (int argc, char *argv[])
{
	buildDirectoryTree("logs/");
	freopen("logs/out.log", "w", stdout);
	removeFile("logs/time_profile.log");
	
	appState = APPSTATE_STARTING;

	printf("[main] Starting app at %s.\n", timestamp(true).c_str());
	try
	{
		Uint32 screenFlags = SDL_SWSURFACE | SDL_DOUBLEBUF;
		SetScreenFlags(	screenFlags );


#ifndef GUI_ONLY
       	if (argc > 1) //argv[0] is full application path
       	{
/*			if (strcmp(argv[1], "-oe") == 0)
			{
				screenFlags |= SDL_RESIZABLE;
				SetScreenFlags(	screenFlags );

				new GuiManager();
				new ObjectEditor();
			}
			else if (strcmp(argv[1], "-me") == 0)
			{
				screenFlags |= SDL_RESIZABLE;
				SetScreenFlags(	screenFlags );
				
				new GuiManager();
				new MapEditorDialog();
			}

			else*/ if (strcmp(argv[1], "-terra") == 0)
			{
				new GuiManager();
				new TerrainTest();
			}
			else if (strcmp(argv[1], "-nologin") == 0)
			{
				new GuiManager();
				new GameManager(false);
			}
		}
		else //no arguments, run natural game
		{
			new GuiManager();
			
			PRINT("Booting up GM");
			new GameManager(true);
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

	printf("[main] Closing app at %s, have a nice day.\n", timestamp(true).c_str());

	return 0;
}
