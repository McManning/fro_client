
#ifdef WIN32xx
//#	include <windows.h>
#	include <process.h>
#endif

#include "AutoUpdater.h"
#include "../core/GuiManager.h"
#include "../core/net/DownloadManager.h"
#include "../core/io/FileIO.h"
#include "../core/widgets/Console.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/Label.h"

void callback_manifestResponse_Success(downloadData* data)
{
	AutoUpdater* au = (AutoUpdater*)data->userData;
	if (au)
	{
		au->ParseManifest();
	}
}

void callback_manifestResponse_Failure(downloadData* data)
{
	AutoUpdater* au = (AutoUpdater*)data->userData;
	if (au)
	{
		string reason = its(data->errorCode); // + ":" + its(data->httpError);
		au->ManifestDownloadFailure(reason);
	}
}

void callback_FileDownload_Success(downloadData* data)
{
	AutoUpdater* au = (AutoUpdater*)data->userData;
	if (au)
	{
		au->FileDownloadSuccess(data->url, data->filename);
	}
}

void callback_FileDownload_Failure(downloadData* data)
{
	AutoUpdater* au = (AutoUpdater*)data->userData;
	if (au)
	{
		string reason = its(data->errorCode); // + ":" + its(data->httpError);
		
		au->FileDownloadFailure(data->url, data->filename, reason);
	}
}

AutoUpdater::AutoUpdater()
	: Frame(gui, "", rect(0,0,300,200), "Auto Updater", true, false, false, true)
{
	//DemandFocus();
	
	mLog = new Multiline(this, "", rect(5, 55, Width() - 10, Height() - 60));
		//mLog->mHighlightSelected = true;
		mLog->mWrap = false;
		
	mProgress = new Label(this, "", rect(5, 30), "Starting...");
}

AutoUpdater::~AutoUpdater()
{

}

void AutoUpdater::SendRequestForManifest()
{
	//generate the request url
	string url = "http://sybolt.com/drm-svr/update.php";
	url += "?ver=" APP_VERSION;

	DEBUGOUT(url);

	string file = DIR_CACHE + string("manifest.res");

	SetState(AUTOUPDATER_GETTING_MANIFEST);
	
	if (!downloader->QueueDownload(url, file, 
									this, 
									callback_manifestResponse_Success, 
									callback_manifestResponse_Failure,
									true, true))
	{
		SetError("Failed to request manifest");
	}
}

void AutoUpdater::ManifestDownloadFailure(string& reason)
{
	SetError("Could not download manifest. Reason: " + reason);
	// TODO: log error and retry download!
}

void AutoUpdater::ParseManifest()
{
	string text;
	if (!fileToString(text, DIR_CACHE + string("manifest.res")))
	{
		SetError("Failed to load manifest file");
		return;	
	}

	mCompletedFiles = 0;
	mTotalFiles = 0;
	
	//check for error:Some message
	if (text.find("error:", 0) == 0)
	{
		SetError(text.substr(6));
		return;
	}
	
	QueueFiles(text);
}
	
/**
	If a master:URL line is read in, will change the master source for updates.
	If a URL:HASH line is read in, will md5 hash the matching local file. If the local
		does not match the remote, it will queue a download to replace the local. 
	Will also check for .gz'd files, and compare uncompressed hashes. 
*/
void AutoUpdater::QueueFiles(string& items)
{
	string master, path, hash, localFile, localHash, url;
	bool download;
	vString v;
	vString v2;
	size_t pos, pos2;

	if (items.empty())
		return;
	
	replace(&items, "\r", "");
	explode(&v, &items, "\n");

	for (int i = 0; i < v.size(); ++i)
	{
		//cleanup all whitespace
		for (pos = 0; pos < v.at(i).length(); ++pos)
		{
			if (isWhitespace(v.at(i).at(pos)))
			{
				v.at(i).erase(v.at(i).begin() + pos);
				--pos;
			}
		}

		if (v.at(i).find("master:", 0) == 0)
		{
			master = v.at(i).substr(7);
			printf("\tMaster set to %s\n", master.c_str());
		}
		else if (!v.at(i).empty())
		{
			if (master.empty())
			{
				SetError("Malformed manifest");
				return;
			}
			
			v2.clear();
			explode(&v2, &v.at(i), ":");

			path = v2.at(0);
			url = master + path;
			download = true;

			switch (v2.size())
			{
				case 1: // case of path/to/file.ext
					// if it's just on disk, don't download
					if (fileExists(path))
						download = false;
					
					break;
				case 2: // path/to/file.ext:UNCOMPRESSED_HASH
					// if files match, don't download
					if (md5file(path) == v2.at(1))
					{
						download = false;
					}
					else
					{
						hash = v2.at(1);
					}
					break;
				case 3: // path/to/file.ext:UNCOMPRESSED_HASH:COMPRESSED_HASH
					
					path.erase(path.length()-3); // erase .gz extension
					
					// if our local hash matches the uncompressed one, don't download
					if (md5file(path) == v2.at(1))
					{
						download = false;
					}
					else
					{
						path += ".gz";
						hash = v2.at(2);
					}
					break;
				default: break;
			}

			if (download)
				GetFile(url, path, hash);
				
		} // if !v.at(i).empty()
	} // for all in v
	
	//if the manifest is empty, or we got everything, we're done!
	if (mTotalFiles == 0 || mTotalFiles == mCompletedFiles)
	{
		SetState(AUTOUPDATER_NO_UPDATES);
	}
}

void AutoUpdater::GetFile(string& url, string& file, string& hash)
{
	++mTotalFiles;
	
	if (!downloader->QueueDownload(url, file, this,
								callback_FileDownload_Success,
								callback_FileDownload_Failure,
								true, false, hash))
	{
		SetError("Could not queue " + file);
	}
}

void AutoUpdater::FileDownloadSuccess(string& url, string& filename)
{
	++mCompletedFiles;

	if (ExtractFile(filename))
	{
		mLog->AddMessage("\\c009* " + filename);
		mProgress->SetCaption("Progress " + its(mCompletedFiles) + "/" + its(mTotalFiles));	
		
		if (mCompletedFiles == mTotalFiles)
		{
			SetState(AUTOUPDATER_COMPLETE);
		}
	}
}

void AutoUpdater::FileDownloadFailure(string& url, string& filename, string& reason)
{
	SetError("Could not download " + filename + ". Reason: " + reason);
	// TODO: log error and retry download!
}

bool AutoUpdater::ExtractFile(string& file)
{
	// If the file was zlib'd, uncompress  FILENAME.gz to FILENAME
	if (file.substr(file.length()-3) == ".gz")
	{
		if (!decompressFile(file, file.substr(0, file.length()-3)))
		{
			SetError("Failed to decompress " + file);
			return false;
		}
		
		/*	Don't need to worry about hashing the decompressed. If decompression was
			a success, then we know it maintained integrity. 
		*/
		
		// get rid of the compressed version
		removeFile(file);
	}
	
	return true;
}

void AutoUpdater::Finished()
{
	// shell the merger (if necessary) and kill fro
	
	// Just do it regardless

	//appState = APPSTATE_CLOSING;
	
	DEBUGOUT("Shelling");
	
#ifdef WIN32zz
	char buffer[64];
	sprintf(buffer, "%d", _getpid());
	
	int result = (int)ShellExecute(NULL, "open", "fro.exe", buffer, NULL, SW_SHOWNORMAL);
	if (result <= 32)
	{
		sprintf(buffer, "Encountered error code %d while trying to execute fro.exe", result);
		
		MessageBox(NULL, buffer, "Error",
					MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST);
	}

#endif

}

void AutoUpdater::SetState(int state)
{
	//something
	DEBUGOUT("Setting state: " + its(state));
	
	switch (state)
	{
		case AUTOUPDATER_GETTING_MANIFEST:
			mLog->AddMessage("Downloading Manifest...");
			break;
		case AUTOUPDATER_GETTING_FILES:
			mProgress->SetCaption("Progress 0/" + its(mTotalFiles));
			mLog->AddMessage("Getting Files...");
			break;
		case AUTOUPDATER_NO_UPDATES:
			mLog->AddMessage("\\c030No update necessary");
		//	Die();
			// start world selector?
			break;
		case AUTOUPDATER_COMPLETE:
			mLog->AddMessage("\\c030Finished");
			Finished();
			break;
		case AUTOUPDATER_ERROR:
			//idk
			break;
		default: break;
	}
}

void AutoUpdater::SetError(string error)
{
	// something
	mLog->AddMessage("\\c500<!> " + error);

	// messagepopup?
	
	SetState(AUTOUPDATER_ERROR);
}
