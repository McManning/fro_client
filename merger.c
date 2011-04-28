
#include <windows.h>
#include <io.h> //_findfirsti64, _findnexti64, _findclose, _finddatai64, _mkdir

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
	MoveUpdatedResources();
	
	int handle = -1;
	if (argc > 1)
	{
		handle = atoi(argv[1]);
	}
	
	printf("Remote handle: %d\n", handle);
	
	// ParentHWND, OperationString, Filename, ParametersPassed, CurrentDirectory, ShowCommand
	int result = (int)ShellExecute(NULL, "open", "fro.exe", "", NULL, SW_SHOWNORMAL);
	if (result <= 32)
	{
		char buffer[64];
		sprintf(buffer, "Encountered error code %d while trying to execute fro.exe", result);
		
		MessageBox(NULL, buffer, "Error",
					MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST);
	}

	return 0;
}


