
#ifndef _AUTOUPDATER_H_
#define _AUTOUPDATER_H_

#include "../core/widgets/Frame.h"

class Multiline;
class Label;
class AutoUpdater : public Frame
{
  public:
	AutoUpdater();
	~AutoUpdater();
	
	enum 
	{
		AUTOUPDATER_GETTING_MANIFEST = 0,
		AUTOUPDATER_GETTING_FILES,
		AUTOUPDATER_NO_UPDATES,
		AUTOUPDATER_COMPLETE,
		AUTOUPDATER_ERROR,
	};
	
	void SendRequestForManifest();
	void ManifestDownloadFailure(string& reason);
	void ParseManifest();
	void QueueFiles(string& items);
	void GetFile(string& url, string& file, string& hash);
	void FileDownloadSuccess(string& url, string& filename);
	void FileDownloadFailure(string& url, string& filename, string& reason);
	bool ExtractFile(string& file);
	void Finished();
	void SetState(int state);
	void SetError(string error);
	
	int mCompletedFiles;
	int mTotalFiles;
	
	Multiline* mLog;
	Label* mProgress;
};

#endif //_AUTOUPDATER_H_

