
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


#include "AutoUpdater.h"
#include "../core/GuiManager.h"
#include "../core/net/DownloadManager.h"
#include "../core/io/FileIO.h"
#include "../core/widgets/Console.h"
#include "../core/widgets/Multiline.h"
#include "../core/widgets/Label.h"
#include "../core/widgets/Button.h"
#include "../game/GameManager.h"
#include "../core/net/IrcNet2.h"

#ifdef WIN32
#	include <windows.h>
#	include <process.h>
#endif

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
		string reason = its(data->errorCode) + ":" + its(data->httpError);
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
		string reason = its(data->errorCode) + ":" + its(data->httpError);

		au->FileDownloadFailure(data->url, data->filename, reason);
	}
}

void callback_saveUpdaterLog(Button* b)
{
	AutoUpdater* au = (AutoUpdater*)b->GetParent();
	if (au)
	{
		au->SaveLog();
	}
}

void callback_retryUpdate(Button* b)
{
	AutoUpdater* au = (AutoUpdater*)b->GetParent();
	if (au)
	{
		au->Retry();
	}
}

AutoUpdater::AutoUpdater()
	: Frame(gui, "AutoUpdater", rect(0,0,400,300), "Auto Updater", true, false, false, true)
{
	//DemandFocus();

	mLog = new Multiline(this, "", rect(5, 55, Width() - 10, Height() - 85));
		//mLog->mHighlightSelected = true;
		mLog->mWrap = false;

	mProgress = new Label(this, "", rect(5, 30), "Starting...");

	Button* b;
	b = new Button(this, "", rect(Width()-25, Height()-25, 20, 20), "", callback_saveUpdaterLog);
		b->SetImage("assets/buttons/clipboard.png");
		b->mHoverText = "Save Log";

	mRetry = new Button(this, "", rect(5, Height()-25, 20, 20), "", callback_retryUpdate);
		mRetry->SetImage("assets/buttons/reload_worlds.png");
		mRetry->mHoverText = "Retry Update";

	Center();
}

AutoUpdater::~AutoUpdater()
{

}

void AutoUpdater::Retry()
{
	mCompletedFiles = 0;
	mTotalFiles = 0;

	mLog->AddMessage("\\c900<!> Retrying Update");
	SendRequestForManifest();
}

void AutoUpdater::SendRequestForManifest()
{
	mRetry->SetActive(false);

	//generate the request url
	string url = "http://sybolt.com/drm-svr/update.php";
	url += "?ver=" VER_STRING;

	DEBUGOUT(url);

	string file = DIR_CACHE "manifest.res";

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
	if (!fileToString(text, DIR_CACHE "manifest.res"))
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
	bool isMergable;
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
		/*for (pos = 0; pos < v.at(i).length(); ++pos)
		{
			if (isWhitespace(v.at(i).at(pos)))
			{
				v.at(i).erase(v.at(i).begin() + pos);
				--pos;
			}
		}*/

		if (v.at(i).find("master:", 0) == 0)
		{
			master = v.at(i).substr(7);
			mLog->AddMessage("\\c005 * Getting from " + master);
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

			// @todo This is hacky. Improve it.
			if (path.find(".~", 0) == 0)
			{
				isMergable = true;
				path.erase(0, 2);
			}
			else
			{
				isMergable = false;
			}

			switch (v2.size())
			{
				case 1: // case of path/to/file.ext
					// if it's just on disk, don't download
					if (fileExists(path)
						|| (isMergable && fileExists(".~" + path)))
						download = false;

					break;
				case 2: // path/to/file.ext:UNCOMPRESSED_HASH
					// if files match, don't download
					if (MD5File(path) == v2.at(1)
						|| (isMergable && MD5File(".~" + path) == v2.at(1)))
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
					if (MD5File(path) == v2.at(1)
						|| (isMergable && MD5File(".~" + path) == v2.at(1)))
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

			if (isMergable)
				path = ".~" + path;

			if (download)
				GetFile(url, path, hash);

		} // if !v.at(i).empty()
	} // for all in v

	//if the manifest is empty, or we got everything, we're done!
	if (mTotalFiles == 0)
	{
		mLog->AddMessage("\\c050 * All files up to date");
		SetState(AUTOUPDATER_NO_UPDATES);
	}
}

void AutoUpdater::GetFile(string& url, string& file, string& hash)
{
	++mTotalFiles;

	mLog->AddMessage(file);
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
		mLog->AddMessage("\\c050* Finished " + filename);
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

	appState = APPSTATE_CLOSING;

#ifdef WIN32

	// send it our process ID to let merger wait for us to close (or terminate if need be)
	char buffer[64];
	sprintf(buffer, "%d", GetCurrentProcessId());

	int result = (int)ShellExecute(NULL, "open", "merger.exe", buffer, NULL, SW_SHOWNORMAL);
	if (result <= 32)
	{
		sprintf(buffer, "Encountered error code %d while trying to execute merger.exe", result);

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
			mLog->AddMessage("\\c005 * Getting Manifest...");
			break;
		case AUTOUPDATER_GETTING_FILES:
			mProgress->SetCaption("Progress 0/" + its(mTotalFiles));
			mLog->AddMessage("Getting Files...");
			break;
		case AUTOUPDATER_NO_UPDATES:
			mLog->AddMessage("\\c030No update necessary");
			Die();
			game->mNet->TryNextServer();
			break;
		case AUTOUPDATER_COMPLETE:
			mLog->AddMessage("\\c030Finished");
			Finished();
			break;
		case AUTOUPDATER_ERROR:

			// so next time it'll try again
			removeFile(DIR_CACHE "manifest.res");
			mRetry->SetActive(true);
			break;
		default: break;
	}
}

void AutoUpdater::SetError(string error)
{
	// something
	mLog->AddMessage("\\c500<!> " + error);
	console->AddMessage("\\c500[AutoUpdater] " + error);

	// messagepopup?

	SetState(AUTOUPDATER_ERROR);
}

void AutoUpdater::SaveLog()
{
	// @todo All this code is from Console::SaveText(). Need to merge it somewhere common!
	// Ex: Multiline::SaveText(filename)

	string s;
	s = "saved/updatelog_" + timestamp(true) + ".html";
	buildDirectoryTree(s);

	FILE* f = fopen(s.c_str(), "w");
	if (!f)
	{
		mLog->AddMessage("\\c500* Could not open output file");
		return;
	}

	fprintf(f, "<html>\n<head>\n"
				"<title>Saved %s</title>\n"
				"</head>\n<body bgcolor=\"#FFFFFF\">\n", timestamp(true).c_str());
	vString v;
	string c;
	for (int i = 0; i < mLog->mRawText.size(); i++)
	{
		//Split up the line via \cRGB
		explode(&v, &mLog->mRawText.at(i), "\\c");
		for (int ii = 0; ii < v.size(); ii++)
		{
			if (ii == 0 && mLog->mRawText.at(i).find("\\c", 0) != 0)
			{
				//if we don't have a font color defined, use default
				c = "000000";
			}
			else
			{
				c = colorToHex( slashCtoColor( v.at(ii).substr(0, 3) ) );
				v.at(ii).erase(0, 3); //erase RGB
			}
			fprintf(f, "<font color=\"#%s\">%s</font>", c.c_str(), v.at(ii).c_str());
		}
		v.clear();
		fprintf(f, "<br/>\n");
	}
	fprintf(f, "</body>\n</html>\n");
	fclose(f);
	mLog->AddMessage("\\c050* Log saved to \\c239" + s);
}

