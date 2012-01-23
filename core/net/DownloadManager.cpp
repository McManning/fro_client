
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


#include <fstream> //TODO: Use FILE* instead
#include <SDL/SDL.h>

#include "DownloadManager.h"
//#include "widgets/Console.h"
#include "../io/FileIO.h"
#include "../TimerManager.h"
#include "../MessageManager.h"
#include "../Logger.h"

#ifdef WIN32
	#include <winsock2.h>
#else
    //TODO: Do I need ALL these?
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/select.h>
    #include <arpa/inet.h> //inet_addr
	typedef int SOCKET;
	#define SOCKET_ERROR (-1) /* TODO: not sure if 0 or -1 */
	#define INVALID_SOCKET (-1) /* TODO: not sure if 0 or -1 */
#endif

DownloadManager* downloader;

int thread_downloader(void* param)
{
	downloadThread* dt = (downloadThread*)param;
	int result;

	//while (!downloader->mFlushing)
	while (!isAppClosing())
	{
		SDL_LockMutex(downloader->mQueuedMutex);
		if (!downloader->mQueued.empty())
		{
			//Grab and erase from the queue
			dt->currentData = downloader->mQueued.at(0);
			downloader->mQueued.erase(downloader->mQueued.begin());
			SDL_UnlockMutex(downloader->mQueuedMutex);

			#ifdef OFFLINE_MODE
				//We're going to SIMULATE a download situation, ie.. pause few seconds for a "download" then just read local.
				SDL_Delay(5000);
				dt->currentData->filename = dt->currentData->url;
				dt->currentData->errorCode = (fileExists(dt->currentData->filename)) ? DEC_SUCCESS : DEC_FILENOTFOUND;
			#else
				//SDL_Delay(3000);
				if (!fileExists(dt->currentData->filename))
				{
					result = sendHttpGet(dt->currentData->url, dt->currentData->filename, dt->currentData->byteCap);
					if (result != HTTP_OKAY)
					{
						//WARNING("CONNECT FAIL " + dt->currentData->url);
						dt->currentData->errorCode = DEC_CONNECTFAIL;
						dt->currentData->httpError = result;
					}
					else
					{
						dt->currentData->errorCode = (fileExists(dt->currentData->filename)) ? DEC_SUCCESS : DEC_FILENOTFOUND;
						/*	Success here doesn't necessarily mean it's the correct file. The host
							could have sent a 404 html page. Or an entirely different file.
						*/
						//DEBUGOUT("GOT FILE " + dt->currentData->url);
					//	DEBUGOUT("CODE " + its(dt->currentData->errorCode));
					}
				}
				else //file is there, no need to download
				{
					dt->currentData->errorCode = DEC_SUCCESS;
					//DEBUGOUT("Already Exists " + dt->currentData->filename);
				}
			#endif

			//Push result onto the completed stack
			SDL_LockMutex(downloader->mCompletedMutex);
			downloader->mCompleted.push_back(dt->currentData);
			dt->currentData = NULL;
			SDL_UnlockMutex(downloader->mCompletedMutex);
		}
		else
		{
			SDL_UnlockMutex(downloader->mQueuedMutex);
		}

		SDL_Delay(10);
	}
DEBUGOUT("Closing Down Thread");

	dt->thread = NULL;
	return 0; //??
}

uShort callback_downloaderManagerProcess(timer* t, uLong ms)
{
	DownloadManager* dm = (DownloadManager*)t->userData;
	ASSERT(dm);
	dm->Process();
	return TIMER_CONTINUE;
}

DownloadManager::DownloadManager()
{
	mFlushing = false;
	mQueuedMutex = NULL;
	mCompletedMutex = NULL;
	mCanQueue = true;

	//Start up winsocks
	//TODO: Check if WSA has been initialized earlier, elsewhere
#ifdef WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

}

DownloadManager::~DownloadManager()
{
DEBUGOUT("DL::~DL");
	Flush(); //in case it wasn't called earlier
	timers->RemoveMatchingUserData(this);

	//shut down winsocks
#ifdef WIN32
	WSACleanup(); //clean up winsocks
#endif

}

bool DownloadManager::Initialize(uShort threadCount)
{
	SDL_Thread* thread = NULL;
	downloadThread* dt = NULL;
	for (uShort i = 0; i < threadCount; i++)
	{
		dt = new downloadThread;

		thread = SDL_CreateThread(thread_downloader, (void*)dt);
		if (thread)
		{
			logger.Write("Starting worker thread 0x%p", thread);
			dt->thread = thread;
			dt->currentData = NULL;
			mThreads.push_back(dt);
		}
		else
		{
			SAFEDELETE(dt);
			logger.WriteError("SDL_CreateThread error: %s", SDL_GetError());
		}
	}

	mQueuedMutex = SDL_CreateMutex();
	mCompletedMutex = SDL_CreateMutex();

	timers->AddProcess("dlman",
						callback_downloaderManagerProcess,
						NULL,
						this);

	return true;
}

void DownloadManager::Flush()
{
	//mFlushing = true;
	uShort i;
	for (i = 0; i < mThreads.size(); i++)
	{
		//Wait for each thread to time out, they will once their current download is complete.

		//DEBUGOUT("Waiting for thread " + its(i) + ": " + pts(mThreads.at(i)->thread));
		//SDL_WaitThread(mThreads.at(i)->thread, NULL);

		if (mThreads.at(i) && mThreads.at(i)->thread)
		{
			WARNING("Forcing a kill on thread " + pts(mThreads.at(i)->thread));
			SDL_KillThread(mThreads.at(i)->thread);
		}

		DEBUGOUT("Thread " + its(i) + " Done");
		SAFEDELETE(mThreads.at(i));
	}
	DEBUGOUT("All threads done.");
	mThreads.clear();

	//Call onFailure for each one, in case we need to delete the userData
	//and we can't delete HERE because C++ hates deleting void*

	for (i = 0; i < mQueued.size(); i++)
	{
		if (mQueued.at(i))
		{
			if (mQueued.at(i)->onFailure)
				mQueued.at(i)->onFailure(mQueued.at(i));
			delete mQueued.at(i);
		}
	}
	mQueued.clear();

	for (i = 0; i < mCompleted.size(); i++)
	{
		if (mCompleted.at(i))
		{
			if (mCompleted.at(i)->onFailure)
				mCompleted.at(i)->onFailure(mCompleted.at(i));
			delete mCompleted.at(i);
		}
	}
	mCompleted.clear();

	for (i = 0; i < mWaiting.size(); i++)
	{
		if (mWaiting.at(i))
		{
			if (mWaiting.at(i)->onFailure)
				mWaiting.at(i)->onFailure(mWaiting.at(i));
			delete mWaiting.at(i);
		}
	}
	mWaiting.clear();

	SDL_DestroyMutex(mQueuedMutex);
	SDL_DestroyMutex(mCompletedMutex);

	//mFlushing = false; //done
}

void DownloadManager::Process()
{
	SDL_LockMutex(mCompletedMutex);

	MessageData md;

	int w;
	for (int i = 0; i < mCompleted.size(); ++i)
	{
		if (mCompleted.at(i))
		{
			//check for a bad hash for completed downloads
			if ( mCompleted.at(i)->errorCode == DEC_SUCCESS )
			{
				if (!mCompleted.at(i)->md5hash.empty()
					&& MD5File(mCompleted.at(i)->filename) != mCompleted.at(i)->md5hash)
				{
				    logger.WriteError("Hash check failure of %s (%s). Should be %s",
                          mCompleted.at(i)->filename.c_str(),
                          MD5File(mCompleted.at(i)->filename).c_str(),
                          mCompleted.at(i)->md5hash.c_str());

                    mCompleted.at(i)->errorCode = DEC_BADHASH;
				}
			}

			//finally, do our callbacks based on our error code
			if ( mCompleted.at(i)->errorCode == DEC_SUCCESS )
			{
				if (mCompleted.at(i)->onSuccess)
					mCompleted.at(i)->onSuccess(mCompleted.at(i));

				md.SetId("NET_DOWNLOAD_SUCCESS");
			}
			else
			{
				if (mCompleted.at(i)->onFailure)
					mCompleted.at(i)->onFailure(mCompleted.at(i));

				md.SetId("NET_DOWNLOAD_FAILURE");
			}

			// Send a global event notice
			md.WriteString("url", mCompleted.at(i)->url);
			md.WriteString("local", mCompleted.at(i)->filename);
			md.WriteUserdata("userdata", mCompleted.at(i)->userData);
			md.WriteInt("errorcode", mCompleted.at(i)->errorCode);
			md.WriteInt("httperror", mCompleted.at(i)->httpError);
			messenger.Dispatch(md);

			ProcessMatchingWaitingDownloads(mCompleted.at(i));

			delete mCompleted.at(i);
		}
	}
	mCompleted.clear(); //Clear all completed downloads
	SDL_UnlockMutex(mCompletedMutex);

}

void DownloadManager::ProcessMatchingWaitingDownloads(downloadData* completed)
{
	// Go through the mWaiting list and find matches, then call them too
	for (int w = 0; w < mWaiting.size(); ++w)
	{
		if (mWaiting.at(w)->url == completed->url)
		{
			mWaiting.at(w)->errorCode = completed->errorCode;

			// They could have different files with the same URL
			if (mWaiting.at(w)->filename != completed->filename)
			{
				copyFile(completed->filename, mWaiting.at(w)->filename);
			}

			// do our callbacks based on our error code
			if ( mWaiting.at(w)->errorCode == DEC_SUCCESS )
			{
				if (mWaiting.at(w)->onSuccess)
					mWaiting.at(w)->onSuccess(mWaiting.at(w));
			}
			else
			{
				if (mWaiting.at(w)->onFailure)
					mWaiting.at(w)->onFailure(mWaiting.at(w));
			}

			delete mWaiting.at(w);
			mWaiting.erase(mWaiting.begin() + w);
			--w;
		}
	}
}

bool DownloadManager::QueueDownload(string url, string file, void* userData,
							void (*onSuccess)(downloadData*), void (*onFailure)(downloadData*),
							bool overwrite, bool overrideQueueLock, string md5hash, int byteCap)
{
	if (!mCanQueue && !overrideQueueLock) //can't queue and can't override, cancel.
	{
		WARNING("Queue locked while trying " + url);
		return false;
	}

	downloadData* data = new downloadData();
	data->url = url;
	data->filename = file;
	data->userData = userData;
	data->onSuccess = onSuccess;
	data->onFailure = onFailure;
	data->errorCode = DEC_LOADING;
	data->md5hash = md5hash;
	data->byteCap = byteCap;

	if (overwrite)
	{
		//only erase the old file if it isn't a match
		if (fileExists(file) &&  MD5File(file) != md5hash)
			removeFile(file);
	}

	//build directory structure if it's not there already
	buildDirectoryTree(file);

	// if we're already downloading this file, store elsewhere
	if (IsUrlQueued(url))
	{
		mWaiting.push_back(data);
	}
	else
	{
		SDL_LockMutex(mQueuedMutex);
		mQueued.push_back(data);
		SDL_UnlockMutex(mQueuedMutex);
	}

	MessageData md("NET_DOWNLOAD_QUEUED");
	md.WriteString("url", url);
	md.WriteString("local", file);
	md.WriteUserdata("userdata", userData);
	messenger.Dispatch(md);

	return true;
}

bool DownloadManager::IsUrlQueued(const string& url)
{
	int i;
	bool result = false;

	SDL_LockMutex(mCompletedMutex);
	SDL_LockMutex(mQueuedMutex);

	for (i = 0; i < mCompleted.size(); i++)
	{
		if (mCompleted.at(i)->url == url)
		{
			result = true;
			break;
		}
	}

	if (!result)
	{
		for (i = 0; i < mQueued.size(); i++)
		{
			if (mQueued.at(i)->url == url)
			{
				result = true;
				break;
			}
		}
	}

	if (!result)
	{
		for (i = 0; i < mThreads.size(); ++i)
		{
			if (mThreads.at(i)->currentData
				&& mThreads.at(i)->currentData->url == url)
			{
				result = true;
				break;
			}
		}
	}

	SDL_UnlockMutex(mCompletedMutex);
	SDL_UnlockMutex(mQueuedMutex);

	return result;
}

int DownloadManager::CountActiveDownloads()
{
	int result = 0;
	for (int i = 0; i < mThreads.size(); ++i)
	{
		if (mThreads.at(i)->currentData)
			++result;
	}
	return result;
}

int DownloadManager::CountMatchingUserData(void* userData)
{
	int count = 0;
	int i;

	//lock these together to stop the threads from processing further COMPLETELY
	SDL_LockMutex(mCompletedMutex);
	SDL_LockMutex(mQueuedMutex);

	for (i = 0; i < mCompleted.size(); i++)
	{
		if (mCompleted.at(i)->userData == userData)
			++count;
	}

	for (i = 0; i < mQueued.size(); i++)
	{
		if (mQueued.at(i)->userData == userData)
			++count;
	}

	for (i = 0; i < mWaiting.size(); i++)
	{
		if (mWaiting.at(i)->userData == userData)
			++count;
	}

	for (i = 0; i < mThreads.size(); i++)
	{
		/*	TODO: Do we need a mutex for currentData? Since the other two are locked, threads
			can't really move anything in/out currentData..
		*/
		if (mThreads.at(i)->currentData && mThreads.at(i)->currentData->userData == userData)
			++count;
	}

	SDL_UnlockMutex(mCompletedMutex);
	SDL_UnlockMutex(mQueuedMutex);

	return count;
}

/*
	Locking is necessary because the thread might add/remove list members during
	iteration, and the .size() could be wrong, causing iteration to go out of scope.
*/
bool DownloadManager::NullMatchingUserData(void* userData)
{
	uShort i;

	//lock these together to stop the threads from processing further COMPLETELY
	SDL_LockMutex(mCompletedMutex);
	SDL_LockMutex(mQueuedMutex);

	for (i = 0; i < mCompleted.size(); i++)
	{
		if (mCompleted.at(i)->userData == userData)
			mCompleted.at(i)->userData = NULL;
	}

	for (i = 0; i < mQueued.size(); i++)
	{
		if (mQueued.at(i)->userData == userData)
			mQueued.at(i)->userData = NULL;
	}

	for (i = 0; i < mWaiting.size(); i++)
	{
		if (mWaiting.at(i)->userData == userData)
			mWaiting.at(i)->userData = NULL;
	}

	for (i = 0; i < mThreads.size(); i++)
	{
		/*	TODO: Do we need a mutex for currentData? Since the other two are locked, threads
			can't really move anything in/out currentData..
		*/
		if (mThreads.at(i)->currentData && mThreads.at(i)->currentData->userData == userData)
			 mThreads.at(i)->currentData->userData = NULL;
	}

	SDL_UnlockMutex(mCompletedMutex);
	SDL_UnlockMutex(mQueuedMutex);

	return true; //TODO: better return value
}

bool DownloadManager::IsIdle()
{
	bool result = false;

	if (mQueued.empty() && mCompleted.empty() && mWaiting.empty())
		result = true;

	//if any of our threads are currently working on a download, we still have stuff left.
	for (uShort i = 0; i < mThreads.size(); i++)
	{
		if (mThreads.at(i)->currentData)
		{
			result = false;
			break;
		}
	}

	return result;
}

//Connects sock to the host~
int openSocket(SOCKET* sock, const char* host)
{
	*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*sock == INVALID_SOCKET)
	{
#ifdef WIN32
		WARNING("SOCKET INVALID: " + its(WSAGetLastError()));
#else
        WARNING("SOCKET INVALID: TODO: port");
#endif
		return HTTP_BAD_SOCKET;
	}

    //TODO: inet_addr obsolete, replace with inet_pton or inet_aton

	struct hostent *hp;
	unsigned int addr;
	if(inet_addr(host) == INADDR_NONE)
	{
		hp = gethostbyname(host);
	}
	else
	{
		addr = inet_addr(host);
		hp = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
	}

	if (!hp) //Could not resolve host
	{
#ifdef WIN32
        closesocket(*sock);
        WARNING("SOCKET Unresolveable " + string(host)
					+ " CODE: " + its(WSAGetLastError()));
#else
        close(*sock);
        WARNING("SOCKET Unresolveable " + string(host)
							+ " CODE: TODO: port code");
#endif
		return HTTP_UNRESOLVEABLE_HOST;
	}

	struct sockaddr_in server;
	server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(80);

	if ( ::connect(*sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
#ifdef WIN32
        closesocket(*sock);
        WARNING("SOCKET Cannot Connect " + string(host)
							+ " CODE: " + its(WSAGetLastError()));
#else
        close(*sock);
        WARNING("SOCKET Cannot Connect " + string(host)
							+ " CODE: TODO: port code");
#endif
		return HTTP_CANNOT_CONNECT;
	}

	return HTTP_OKAY; //all's good
}

//Splits a url up into a host name and a file path~
bool breakUrl(string url, string* host, string* file)
{
	int a, b;
	if (url.find("http://", 0) == string::npos) //case of: site.com/folder/file.ext
	{
		if (url.find('/', 0) == string::npos) //no slash found, invalid url
			return false;
        b = url.find('/', 0);
		*host = url.substr(0, b); //should be site.com
		*file = url.substr(b);
	}
	else //case of: http://site.com/folder/file.ext
	{
		a = url.find("http://", 0);
		if (url.find('/', a + 7) == string::npos) //no slash found, invalid url
			return false;
		b = url.find('/', a + 7);
		*host = url.substr(a + 7, b - (a + 7));
		*file = url.substr(b); //get the rest (/folder/file.ext)
	}
	return true;
}

//Rip off the header and download raw contents into file.
//(ASSUMING the header properly ends with \r\n\r\n)
int sendHttpGet(string url, string file, int cap)
{
	char buff[1024*8];
	string host, path;
	SOCKET conn;
	int y, p;
	long size;
	std::fstream f;
    int headerEnd;
    int result = HTTP_OKAY;

	if (!breakUrl(url, &host, &path))
	{
		return HTTP_MALFORMED_REQUEST;
	}

	//console->AddMessage("HTTP Opening Socket");

	y = openSocket(&conn, host.c_str());
	if (y != HTTP_OKAY)
	{
		return y;
	}

	//send our request~
	sprintf(buff,
		"GET %s HTTP/1.0\r\n"
		"User-Agent: Drm\r\n"
		"Host: %s\r\n"
		"Connection: close\r\n\r\n",
		path.c_str(), host.c_str());

	//DEBUGOUT(buff);

	send(conn, buff, strlen(buff), 0);

	//console->AddMessage("HTTP Receiving header");

	y = recv(conn, buff, sizeof(buff), 0); //grab first chunk

	if (y != 0 && y != SOCKET_ERROR)
	{
		f.open(file.c_str(), std::fstream::out|std::fstream::trunc|std::fstream::binary);

	    string buffer = buff;

	    headerEnd = buffer.find("\r\n\r\n", 0);
	    headerEnd += 4;
	    f.write(buff + headerEnd, y - headerEnd); //write the shit from the header~

	//	console->AddMessage("HTTP Filtering Header out. Header Len: " + its(headerEnd));

		p = buffer.find("Content-Length: ");
		if (p != string::npos)
		{
        	p += sizeof("Content-Length: ") - 1;
        	size = sti( buffer.substr(p, buffer.find('\n', p) - p) );
        //	console->AddMessage("HTTP Filesize: " + its(size));
    	}
		else // just download till the socket closes
		{
			size = 0;
		}
		p = y - headerEnd; //current amount written NOT including the header

		while (p < size || size == 0)
		{
			//check for files that are too large. If they are, cancel download and delete file
			if ( cap > 0 && p > cap )
			{
				f.close();
				result = HTTP_FILESIZE_CAPPED;
				break;
			}

			y = recv(conn, buff, sizeof(buff), 0);

			//write the rest of the file
			if (y == 0 || y == SOCKET_ERROR)
			{
				break;
			}
			else
			{
				f.write(buff, y);
				p += y;
			}
		}

		f.close();
	}

	//console->AddMessage("HTTP Cleaning up");

#ifdef WIN32
	closesocket(conn); //done
#else
    close(conn);
#endif

	if (result != HTTP_OKAY) // incomplete file
	{
		removeFile(file);
	}

	return result;
}
