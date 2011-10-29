
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


#ifndef _AUTOUPDATER_H_
#define _AUTOUPDATER_H_

#include "../core/widgets/Frame.h"

class Multiline;
class Label;
class Button;
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
	void SaveLog();
	void Retry();
	
	int mCompletedFiles;
	int mTotalFiles;
	
	Multiline* mLog;
	Label* mProgress;
	Button* mRetry;
};

#endif //_AUTOUPDATER_H_

