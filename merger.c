
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


