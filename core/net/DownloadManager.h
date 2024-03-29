
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


#ifndef _DOWNLOADMANAGER_H_
#define _DOWNLOADMANAGER_H_

#include "../Common.h"

typedef enum
{
	DEC_LOADING = 0,
	DEC_WAITING, // This downloadData is waiting for another downloadData to finish with the same file
	DEC_SUCCESS,
	DEC_BADHOST,
	DEC_CONNECTFAIL,
	DEC_FILENOTFOUND,
	DEC_BADHASH
} downloadErrorCode;

// download error codes
// sendHttpGet can return any of these. 
enum {
	HTTP_OKAY = 0, 
	HTTP_MALFORMED_REQUEST,
	HTTP_FILESIZE_CAPPED, // hit max file len
	// returned by open socket:
	HTTP_BAD_SOCKET,
	HTTP_UNRESOLVEABLE_HOST,
	HTTP_CANNOT_CONNECT
};

struct downloadData
{
	string url;
	string filename;
	void (*onSuccess)(downloadData*);
	void (*onFailure)(downloadData*);
	void* userData;
	int byteCap; //maximum file size before cutoff
	downloadErrorCode errorCode;
	int httpError; // detailed error
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

	//Calls the proper callbacks for completed downloads in the main thread.
	void Process();
	
	void ProcessMatchingWaitingDownloads(downloadData* completed);

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
						bool overwrite, bool overrideQueueLock = false, string md5hash = "", int byteCap = 0);

	/** Will NULL out the userData of all downloadData* in both lists if it matches
		the passed userData. This won't actually delete the downloadData due to the fact
		that it might be called during Process(); and fucking with vectors order while
		fucking with vectors order isn't a good idea. Ever.
	*/
	bool NullMatchingUserData(void* userData);

	/**	Returns the number of downloads that match our userdata */
	int CountMatchingUserData(void* userData);

	int CountActiveDownloads();

	bool IsIdle();
	bool IsUrlQueued(const string& url);

	void LockQueue()
	{
		mCanQueue = false;
	};

	void UnlockQueue()
	{
		mCanQueue = true;
	};

	std::vector<downloadThread*> mThreads;
	std::vector<downloadData*> mQueued; // Downloads before being picked up by the threads
	std::vector<downloadData*> mCompleted; // Downloads after being used by the threads
	std::vector<downloadData*> mWaiting; // for duplicate downloads of matching files

	SDL_mutex* mQueuedMutex;
	SDL_mutex* mCompletedMutex;

  private:
	bool mCanQueue; //if we can add new files to the queue
	bool mFlushing;
	
	int mByteCap;
};

extern DownloadManager* downloader;

int sendHttpGet(string url, string file, int cap = 0);

#endif //_DOWNLOADMANAGER_H_
