
#include <stdio.h>
#include <windows.h>
#include <io.h> //_findfirsti64, _findnexti64, _findclose, _finddatai64, _mkdir

//#define LOGGING 1
#ifdef LOGGING
#	define LOG printf
#else
#	define LOG
#endif

/**
	Moves all files in our working directory prefixed with .~ to their counterpart, without the .~ prefix
*/
int MoveUpdatedResources()
{
	struct _finddata_t data;
	long hFile;
	
	if ( (hFile = _findfirst(".~*", &data)) == -1L )
	{
		return 0;
	}
	else
	{
		do
		{
			CopyFile(data.name, data.name + 2, FALSE);
			remove(data.name);
		} 
		while ( _findnext(hFile, &data) == 0 );

		_findclose( hFile );
		return 1;
	}
}

int main(int argc, char *argv[])
{
	int result;
	DWORD handle;
	HANDLE proc;
	
#ifdef LOGGING
	freopen("merger.log", "w", stdout);
#endif

	if (argc < 1)
	{
		return 0;
	}
	
	handle = (DWORD)atoi(argv[1]);

	LOG("Opening process %d...\n", handle);

	proc = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, handle);
	if (proc == NULL)
	{
		LOG("Process not open\n");	
	}
	else
	{
		LOG("Waiting for process to end...\n");
		if (WaitForSingleObject(proc, 5000) != WAIT_OBJECT_0)
		{
			LOG("Taking too long. Terminating.\n");
			TerminateProcess(proc, 0);
		}
		else
		{
			LOG("Process closed gracefully.\n");	
		}
		
		CloseHandle(proc);
	}

	LOG("Performing merge\n");
	MoveUpdatedResources();

	LOG("Rebooting fro.exe\n");
	
	// ParentHWND, OperationString, Filename, ParametersPassed, CurrentDirectory, ShowCommand
	result = (int)ShellExecute(NULL, "open", "fro.exe", "", NULL, SW_SHOWNORMAL);
	if (result <= 32)
	{
		char buffer[64];
		sprintf(buffer, "Encountered error code %d while trying to execute fro.exe", result);
		
		MessageBox(NULL, buffer, "Error",
					MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST);
	}
	
	LOG("Cave Johnson. We're done here.\n");

	return 0;
}


