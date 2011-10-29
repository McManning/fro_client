
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


/*
	Various IO and Operating System functions
*/

#ifndef _FILEIO_H_
#define _FILEIO_H_

#include "../Common.h"

string MD5String(const char* msg);
string MD5File(string file);

//decompresses into a temporary file, then descrambles decompressed back into fileName and deletes temp
bool decryptFile(string infile, string outfile, string password, uLong encryptionLength);

//Compresses into a temporary file, then scrambles compressed back into fileName and deletes temp
bool encryptFile(string infile, string outfile, string password, uLong encryptionLength);

//Very simplistic encryption scheme. THEY WON'T EXPECT THE SIMPLE SHIT
void scrambleFile(string infile, string outfile, string password, bool descramble, uShort max);

bool compressFile(string infile, string outfile);
bool decompressFile(string infile, string outfile);

string compressString(string s);
string decompressString(string s);

uLong getFilesize(string filename);
bool removeFile(string file);
void copyFile(string src, string dst);
//string getUniqueFilename();

void systemErrorMessage(string title, string msg);

string getClipboardString();
void sendStringToClipboard(string msg);

//Cache control
string getCacheFilename(string url, string version, bool eraseOtherVersions);
void killCacheVersionsExcluding(string filename, string version);
string getTemporaryCacheFilename();

string getFileDirectory(string path);

string getNextFileMatchingPattern(const string& start, const string& pattern);
string getPreviousFileMatchingPattern(const string& start, const string& pattern);

bool getFilesMatchingPattern(std::vector<string>& list, const string& pattern);
string getRandomFileMatchingPattern(const string& pattern);

bool fileToString(string& s, string file);
bool stringToFile(string& s, string file);
bool fileTovString(vString& v, string file, string delimiter);
bool vStringToFile(vString& v, string file, string delimiter);

/*
	path = a/b/c/file.ext
	Will construct directory tree a/b/c/ in root if it does not exist
	Returns false if it can't build directories, or the path is malformed

	Input -> Result
	abc -> Is filename in root, returns true
	abc/def -> Creates directory abc
	abc/def/ -> Creates directory abc and subdirectory def
*/
bool buildDirectoryTree(string path);


#endif //_FILEIO_H_
