
#include <zlib.h>
#include <fstream>

#ifdef WIN32
#  include <windows.h>
#  include <io.h> //_findfirsti64, _findnexti64, _findclose, _finddatai64, _mkdir
#else
#  include <sys/stat.h> //mkdir
#endif

#include "FileIO.h"
#include "md5.h"
#include "Crypt.h"

uLong getFilesize(string filename)
{
    FILE *file = fopen(filename.c_str(), "rb");
    if (!file)
    	return 0;
    
    fseek (file, 0, SEEK_END);
    uLong size = ftell(file);
    fclose (file);
    return size;
}

bool decompressFile(string infile, string outfile)
{
    gzFile in = gzopen(infile.c_str(), "rb");
    FILE *out = fopen(outfile.c_str(), "wb");

	if (!in)
	{
		if (out) fclose(out);
		return false;
	}

	if (!out)
	{
		if (in) gzclose(in);
		return false;
	}

    char buffer[1024];
    uLong bytesRead = 0;
    while ((bytesRead = gzread(in, buffer, sizeof(buffer))) > 0)
	{
        fwrite(buffer, 1, bytesRead, out);
    }

    gzclose(in);
    fclose(out);

	return true;
}

bool compressFile(string infile, string outfile)
{
	FILE *in = fopen(infile.c_str(), "rb");
	gzFile out = gzopen(outfile.c_str(), "wb");

	if (!in)
	{
		if (out) gzclose(out);
		return false;
	}

	if (!out)
	{
		if (in) fclose(in);
		return false;
	}

    char buffer[1024];
    uLong bytesRead = 0, totalRead = 0;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), in)) > 0)
	{
        totalRead += bytesRead;
        gzwrite(out, buffer, bytesRead);
    }

    fclose(in);
    gzclose(out);

	uLong totalWrote = getFilesize(outfile);
	PRINTF("Read %ld bytes, Wrote %ld bytes, Compression factor %4.2f%%\n",
		totalRead, totalWrote, (1.0 - totalWrote * 1.0 / totalRead) * 100.0);

	return true;
}


//Wouldn't this fuck up since zlib could have non-string bytes in it?
//Source: http://www.koders.com/cpp/fid1E20E6047F06D7088BF9CD60A2EF4785399686B3.aspx?s=cryptopp.cpp
string compressString(string s)
{

	uLong dstlen = s.length() + 300;
	Bytef* buffer = new Bytef[dstlen];

	int ret = compress2(buffer, &dstlen,
						reinterpret_cast<const Bytef*>(s.data()), s.length(),
						Z_BEST_COMPRESSION);
	string tmp(reinterpret_cast<const char*>(buffer), dstlen);

	switch (ret)
	{
		case Z_OK:
			break;
		case Z_MEM_ERROR:
			THROWNUM(1);
		case Z_BUF_ERROR:
			THROWNUM(201);
		case Z_STREAM_ERROR:
			THROWNUM(202);
		default: break;
	}

	delete buffer;
	return tmp;
}

string decompressString(string s)
{
	uLong dstlen = s.size() * 10 + 300;
	Bytef* buffer = new Bytef[dstlen];
	memset(buffer, 0, dstlen);

	int ret = uncompress(buffer, &dstlen,
						reinterpret_cast<const Bytef*>(s.data()), s.length());
	string tmp(reinterpret_cast<const char*>(buffer), dstlen);

	switch (ret)
	{
		case Z_OK:
			break;
		case Z_MEM_ERROR:
			THROWNUM(1);
		case Z_BUF_ERROR: {
			THROWNUM(201);
		} break;
		case Z_DATA_ERROR:
			PRINT("Corrupt/Incomplete input");
			tmp.clear();
			break;
		case Z_ERRNO:
			WARNING("Z_ERRNO");
			tmp.clear();
			break;
		case Z_VERSION_ERROR:
			THROWNUM(202);
		case Z_STREAM_ERROR:
			WARNING("Z_STREAM_ERROR");
			tmp.clear();
			break;
		default: break;
	}

	delete[] buffer;
	return tmp;
}

//Very simplistic encryption scheme. THEY WON'T EXPECT THE SIMPLE SHIT
void scrambleFile(string infile, string outfile, string password, bool descramble, uShort max)
{
    char value;
    uLong passMark = 0;
    uLong locMark = 0;

    std::fstream in(infile.c_str(), std::fstream::in | std::fstream::binary);
    std::fstream out(outfile.c_str(), std::fstream::out | std::fstream::binary);

	//get size of file
  	in.seekg(0, std::ifstream::end);
  	uLong inSize = in.tellg();
  	in.seekg(0);

	do {
		in.read((char*)&value, sizeof(char));

		if (descramble)
		{
        	value -= password.at(passMark) * passMark;
		}
		else
		{
		  	value += password.at(passMark) * passMark;
        }

        std::streamsize n = in.gcount();
		out.write ((char*)&value, n);

		passMark++;
		if (passMark >= password.size())
			passMark = 0;

		if (in.eof())
			break;

		locMark++;

		if (max > 0 && locMark > max) //dump the rest as-is
		{
			char* buffer = new char[inSize - locMark];
			in.read(buffer, inSize - locMark);
			out.write(buffer, inSize - locMark);
			delete[] buffer;
			break;
		}
    } while (in.good());

    in.close();
	in.clear();

    out.close();
	out.clear();
}

//Compresses into a temporary file, then scrambles compressed back into fileName and deletes temp
bool encryptFile(string infile, string outfile, string password, uLong encryptionLength)
{
	bool result = true;

	string tmp = getTemporaryCacheFilename();

	if (compressFile(infile, tmp))
	{
		scrambleFile(tmp, outfile, password, false, encryptionLength);
	}
	else
	{
		result = false;
	}

	removeFile(tmp.c_str());

	return result;
}

//decompresses into a temporary file, then descrambles decompressed back into fileName and deletes temp
bool decryptFile(string infile, string outfile, string password, uLong encryptionLength)
{
	bool result = true;

	string tmp = getTemporaryCacheFilename();

	scrambleFile(infile, tmp, password, true, encryptionLength);

	if (!decompressFile(tmp, outfile))
	{
		result = false;
	}

	removeFile(tmp.c_str());

	return result;
}

string md5(const char* data, uLong len)
{
    string md5data;
    md5_state_t state;
	md5_byte_t digest[16];

	int di;
    char charData[0];

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)data, len);
	md5_finish(&state, digest);

	for (di = 0; di < 16; di++)
	{
        sprintf(charData, "%02x", digest[di]);
        md5data += charData;
    }

    return md5data;
}

string md5file(string file)
{
    std::ifstream stream(file.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

	uLong size;
    char* memblock;
    string result;

    if (stream.is_open())
	{
        size = stream.tellg();
        memblock = new char[size];
        if (!memblock)
        {
        	FATAL("Out of memory: md5(" + file + ")");
		}
        stream.seekg(0, std::ios::beg);
        stream.read(memblock, size);

        result = md5((const char*)memblock, size);

        stream.close();
        delete[] memblock;
    }

    return result;
}

bool removeFile(string file)
{
	if (fileExists(file))
	{
		#ifdef DEBUG
			copyFile(file, file + ".deleted"); //make a backup for testing purposes
		#endif
		remove(file.c_str());
		return true;
	}

	return false;
}

void copyFile(string src, string dst)
{
#ifdef WIN32
	CopyFile(src.c_str(), dst.c_str(), false);
#else
	ASSERT(0); //TODO: *nix equivalent
#endif
}

/*string getUniqueFilename()
{
	char rndChar[16];
	tmpnam(rndChar); //create a unique filename.
	//TODO: What directory does it check to find a unique name? App Local? FIX.

	string tmp = ".";
	tmp += rndChar;

	return tmp;
}*/

void systemErrorMessage(string title, string msg)
{
#ifdef WIN32
	MessageBox(NULL, msg.c_str(), title.c_str(),
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST);
#else
	printf("[%s] %s\n", title.c_str(), msg.c_str());
#endif
}

string getClipboardString()
{
#ifdef WIN32
    char* buffer = NULL;

    string fromClipboard;

	//open the clipboard
    if (OpenClipboard(NULL))
	{
    	HANDLE hData = GetClipboardData( CF_TEXT );
    	if (hData)
		{
        	char* buffer = (char*)GlobalLock( hData );
        	fromClipboard = buffer;
        	GlobalUnlock( hData );
        }
        CloseClipboard();
    }

    return fromClipboard;
#else
    return "";
#endif
}

void sendStringToClipboard(string msg)
{
	if (msg.empty())
		return;

#ifdef WIN32
	if ( OpenClipboard(NULL) )
	{
		HGLOBAL clipbuffer;
		char* buffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, msg.length() + 1);
		buffer = (char*)GlobalLock(clipbuffer);
		strcpy(buffer, LPCSTR(msg.c_str()));
		GlobalUnlock(clipbuffer);
		SetClipboardData(CF_TEXT,clipbuffer);
		CloseClipboard();
	}
#endif
}

string getTemporaryCacheFilename()
{
	string filename = DIR_CACHE;
	string s;
	do
	{
		s.clear();
		for (int i = 0; i < 10; i++)
			s += (char)rnd('a', 'z');
	}
	while ( fileExists(filename + s) );

	return filename + s;
}

string getCacheFilename(string url, string version, bool eraseOtherVersions)
{
	string filename = DIR_CACHE;

	//filename += GetFilenameFromUrl(url) + ".";

	//clip the url
/*	if (url.length() > 32)
		url = url.substr(url.length() - 32);
	filename += base64_encode(url.c_str(), url.length());*/

	//Create a hash of the url itself. TODO: What's the chances of two files having the same hash?
	filename += md5(url.c_str(), url.length()) + ".";

	if (eraseOtherVersions)
		killCacheVersionsExcluding(filename, version);

	filename += version;

	return filename;
}

//destroy anything named filename.* excluding filename.version
void killCacheVersionsExcluding(string filename, string version)
{
	string filter = filename + "*";

#ifdef WIN32
	struct _finddata_t data;
	long h = _findfirst(filter.c_str(), &data);

	if (h >= 0)
	{
		do {
			PRINT(data.name);
			if (data.name != string(filename + version))
				remove(data.name);
		} while ( _findnext(h, &data) == 0);

		_findclose(h);
	}
#else
    WARNING("TODO: Portability");
#endif

}

string getFileDirectory(string path)
{
	int i;
	i = path.find_last_of("\\");
	if (i != string::npos) //windows format
		return path.substr(0, i + 1);

	i = path.find_last_of("/");
	if (i != string::npos) //nix format
		return path.substr(0, i + 1);

	return string(); //no directory before the filename
}


bool getFilesMatchingPattern(std::vector<string>& list, const string& pattern)
{
	struct _finddata_t data;
	long hFile;

	if ( (hFile = _findfirst(pattern.c_str(), &data)) == -1L )
	{
		//printf( "No files found with pattern %s\n", pattern.c_str() );
		return false;
	}
	else
	{
		list.push_back(data.name);
		
		while ( _findnext(hFile, &data) == 0 )
			list.push_back(data.name);

		_findclose( hFile );
		return true;
	}
}

string getRandomFileMatchingPattern(const string& pattern)
{
	std::vector<string> list;
	if ( getFilesMatchingPattern(list, pattern) )
		return list.at( rnd(0, list.size()) );

	return string();
}

string getNextFileMatchingPattern(const string& start, const string& pattern)
{
	std::vector<string> list;
	if ( !getFilesMatchingPattern(list, pattern) )
		return start;
	
	if (list.empty())
		return start;

	for (int i = 0; i < list.size(); i++)
	{
		if (list.at(i) == start)
		{
			//if we're the last file, return the first to loop
			if (i == list.size() - 1)
				return list.at(0);
			else //return the next file
				return list.at(i + 1);
		}
	}
	
	//start wasn't found, return first.
	return list.at(0);
}

string getPreviousFileMatchingPattern(const string& start, const string& pattern)
{
	std::vector<string> list;
	if ( !getFilesMatchingPattern(list, pattern) )
		return start;
	
	if (list.empty())
		return start;

	for (int i = list.size()-1; i > -1; --i)
	{
		if (list.at(i) == start)
		{
			//if we're the last file, return the first to loop
			if (i == 0)
				return list.at(list.size()-1);
			else //return the next file
				return list.at(i - 1);
		}
	}
	
	//start wasn't found, return last.
	return list.at(list.size()-1);
}
bool fileToString(string& s, string file)
{
	FILE* f;

	f = fopen(file.c_str(), "r");

	if (!f)
		return false; 

    char buffer[1024];
    uLong bytesRead = 0;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), f)) > 0)
	{
        s.append(buffer, bytesRead);
    }

	fclose(f);

	return true;
}

bool stringToFile(string& s, string file)
{
	FILE* f;

	f = fopen(file.c_str(), "w");

	if (f) 
	{
		fprintf(f, "%s", s.c_str());
		fclose(f);
		return true;
	}

	return false;
}

bool fileTovString(vString& v, string file, string delimiter)
{
	string s;
	if (!fileToString(s, file))
		return false;

	if (!explode(&v, &s, delimiter))
		return false;

	return true;
}

bool vStringToFile(vString& v, string file, string delimiter)
{
	string s;
	if (!implode(&s, &v, delimiter))
		return false;
		
	stringToFile(s, file);
}

/*
	path = a/b/c/file.ext
	Will construct directory tree a/b/c/ in root if it does not exist
	Returns false if it can't build directories, or the path is malformed
	
	Input -> Result
	abc -> Is filename in root, returns true
	abc/def -> Creates directory abc
	abc/def/ -> Creates directory abc and subdirectory def
*/
bool buildDirectoryTree(string path)
{
	path = getFileDirectory(path);
	if (path.empty())
		return true;

	//Convert backslashes to forward
	replace(&path, "\\", "/");
	
	//Traverse the path and construct directories
	vString v;
	explode(&v, &path, "/");

	string cwd;
	for (int i = 0; i < v.size(); ++i)
	{
		cwd += v.at(i);

#ifdef WIN32
		cwd += "\\";
		if (mkdir(cwd.c_str()) != -1)
			return false;
#else
		cwd += "/";
		if (mkdir(cwd.c_str(), 0777) != -1)
			return false;
#endif
	}
	return true;
}

