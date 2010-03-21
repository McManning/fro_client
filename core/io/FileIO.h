
/*
	Various IO and Operating System functions
*/

#ifndef _FILEIO_H_
#define _FILEIO_H_

#include "../Common.h"


string md5file(string file);
string md5(const char* data, uLong len);

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
