
#ifndef _DOWNLOADMANAGER_H_
#define _DOWNLOADMANAGER_H_

#include "../Common.h"

const int MAX_DOWNLOAD_FILESIZE = (3 * 1024 * 1024); //3 MB

typedef enum
{
	DEC_LOADING = 0,
	DEC_SUCCESS,
	DEC_BADHOST,
	DEC_CONNECTFAIL,
	DEC_FILENOTFOUND,
	DEC_BADHASH
} downloadErrorCode;

struct downloadData
{
	string url;
	string filename;
	void (*onSuccess)(downloadData*);
	void (*onFailure)(downloadData*);
	void* userData;
	int byteCap; //maximum file size before cutoff
	downloadErrorCode errorCode;
	string md5hash;
};

struct SDL_Thread;

struct downloadThread
{
	SDL_Thread* thread;
	downloadData* currentData; //what we're currently downloading
};

/* NOTES:
	-The dlMngr is being deleted AFTER most classes.. so ANY calls to ANYTHING
		from the onFailure function must be checked prior, that includes things
		like graphics, mapManager, gui, font, etc. Things that need to be deleted
		like custom structures passed must still be removed, else it'll get reported
		as a memory leak.
	-use the function isAppClosing() for the threads!
	-Prefix all dlMngr callbacks with dlCallback_ for easy searching. Just like
		all timerMngr callbacks should be prefixed with timer_

*/

//#define OFFLINE_MODE

struct SDL_mutex;
class DownloadManager
{
  public:
	DownloadManager();
	~DownloadManager();

	bool Initialize(uShort threadCount);

	/* Cancels all queued downloads and destroys the threads.
		Returns when the threads are done with their current downloads. */
	void Flush();

	void FlushQueue();

	//Calls the proper callbacks for completed downloads in the main thread.
	void Process();

	/*Add a new download
		If overwrite, the file matching the same name will be erased before downloading.
		If !overwrite and the file exists, it'll push this data right into the completed stack
			and ready to have onSuccess called. Reason we don't tell them right after call this is
			to maintain the same flow of logic between online/offline files. I don't want one bit
			of code loading the file if it exists and another bit loading from a callback after download.
			If you seriously want that, just do if (fileExists(file)) { ... }
		If !overrideQueueLock and the queue is locked, this will return false and nothing will get queued.
		If the file downloaded hits cap in bytes (or, if cap is 0 and hits MAX_DOWNLOAD_FILESIZE)
			it will delete the file and return failure.
		If md5hash is set:
				if the file already exists, and with the same hash, overwrite will be ignored and it'll return
				success. Else it'll download, and verify the hash. If the downloaded file doesn't match, it'll
				return failure.
	*/
	bool QueueDownload(string url, string file, void* userData,
						void (*onSuccess)(downloadData*), void (*onFailure)(downloadData*),
						bool overwrite, bool overrideQueueLock = false, string md5hash = "");

	/*Will NULL out the userData of all downloadData* in both lists if it matches
		the passed userData. This won't actually delete the downloadData due to the fact
		that it might be called during Process(); and fucking with vectors order while
		fucking with vectors order isn't a good idea. Ever.
	*/
	bool NullMatchingUserData(void* userData);

	bool IsIdle();

	void LockQueue()
	{
		mCanQueue = false;
	};

	void UnlockQueue()
	{
		mCanQueue = true;
	};
	
	int GetByteCap() const { return mByteCap; };
	void SetByteCap(int cap) { mByteCap = cap; };

	std::vector<downloadThread*> mThreads;
	std::vector<downloadData*> mQueued;
	std::vector<downloadData*> mCompleted;

	SDL_mutex* mQueuedMutex;
	SDL_mutex* mCompletedMutex;

  private:
	bool mCanQueue; //if we can add new files to the queue
	bool mFlushing;
	
	int mByteCap;
};

extern DownloadManager* downloader;

bool sendHttpGet(string url, string file, int cap = 0);

#endif //_DOWNLOADMANAGER_H_
