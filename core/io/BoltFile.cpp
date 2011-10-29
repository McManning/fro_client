
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


#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include "BoltFile.h"

BoltFile::BoltFile()
{
	mData = NULL;
	mLength = 0;
}

BoltFile::~BoltFile()
{
	Flush();
}

int BoltFile::Load(char* buffer, uLong length)
{
	if (!buffer || length < 1)
	{
		PRINTF("Nothing to load\n");
		return 0;
	}

	Flush();
	mData = buffer;
	mLength = length;

	return 1;
}

//load file contents into mData
int BoltFile::Load(string file)
{
	FILE* f = fopen(file.c_str(), "rb");
	char* data;
	uLong length;

	if (!f)
	{
		PRINTF("Could not open file\n");
		return 0;
	}

    if (fseek(f, 0, SEEK_END) != 0)
    {
		PRINTF("Cannot seek in file\n");
		fclose(f);
		return 0;
	}
    length = ftell(f);

	PRINTF("Len:%ld\n", length);

	fseek(f, 0, SEEK_SET);

	//Create buffer to fit zlib output. "which must be at least 0.1% larger than sourceLen plus 12 bytes."

	data = (char*)malloc(length);
	if (!data)
	{
		PRINTF("malloc error\n");
		fclose(f);
		return 0;
	}

	//dump
	uLong count = fread(data, 1, length, f);
	if (count != length)
	{
		PRINTF("fread error (%ld != %ld)\n", mLength, count);
		fclose(f);
		free(data);
		return 0;
	}
	fclose(f);

	//copy over
	if (mData)
		free(mData);
	mData = data;
	mLength = length;

	PRINTF("File loaded\n");

	return 1;
}

//Write mData to file
int BoltFile::Save(string file)
{
	if (!mData || mLength < 1)
	{
		PRINTF("Nothing to write!\n");
		return 0;
	}

	FILE* f = fopen(file.c_str(), "wb");

	if (!f)
	{
		PRINTF("Could not open output\n");
		return 0;
	}

	//write buffer to file
	uLong count = fwrite( (const void*)mData, 1, mLength, f );
	if ( count != mLength )
	{
		PRINTF("Failed to write buffer (%ld != %ld)\n", mLength, count);
		fclose(f);
		return 0;
	}

	fclose(f);
	PRINTF("File Saved!\n");

	return 1;
}

//Decrypt mData if it isn't already
int BoltFile::Decrypt()
{
	if (mLength < sizeof(BOLTFILE_HEADER) + sizeof(uLong))
	{
		PRINTF("Nothing to decrypt!\n");
		return 0;
	}

	//check for valid header
	if (strstr(mData, BOLTFILE_HEADER) != mData)
	{
		PRINTF("Invalid header! Might already be decrypted!\n");
		return 2;
	}

	uLong cLen;

	uLong pos = sizeof(BOLTFILE_HEADER);

	cLen = (*(uLong*)(mData + pos));
	pos += sizeof(uLong);

	PRINTF("Size: %ld (Pos:%ld)\n", cLen, pos);

	char* cData = (char*)malloc(cLen);
	if (!cData)
	{
		PRINTF("Malloc error\n");
		return 0;
	}

	//decrypt
	if (!_unscramble(mData, mLength, pos))
	{
		PRINTF("Could not unscramble!\n");
		free(cData);
		return 0;
	}

	int ret = uncompress((Bytef*)cData, &cLen,
						(Bytef*)(mData + pos), mLength - pos);
	if ( ret != Z_OK )
	{
		PRINTF("uncompress error: ");
		switch (ret)
		{
			case Z_MEM_ERROR: PRINTF("Z_MEM_ERROR\n"); break;
			case Z_BUF_ERROR: PRINTF("Z_BUF_ERROR\n"); break;
			case Z_DATA_ERROR: PRINTF("Z_DATA_ERROR\n"); break;
			default: PRINTF("CODE:%i\n", ret); break;
		}
		free(cData);
		return 0;
	}

	PRINTF("Closing\n");

	//Move cData into mData
	free(mData);
	mData = cData;
	mLength = cLen;

	return 1;
}

//Encrypt mData if it isn't already
int BoltFile::Encrypt()
{
	if (mLength < 1)
	{
		PRINTF("Nothing to encrypt!\n");
		return 0;
	}

	if (strstr(mData, BOLTFILE_HEADER) == mData)
	{
		PRINTF("Data already encrypted or corrupt!\n");
		return 2;
	}

	uLong cLen = mLength / 10 + mLength + 12;
	uLong pos = sizeof(BOLTFILE_HEADER) + sizeof(uLong);
	char* cData = (char*)malloc(cLen + pos); //allocate enough for the header and len
	if (!cData)
	{
		PRINTF("Malloc error\n");
		return 0;
	}

	uLong len = mLength;

	pos = 0;

	//write header
	strcpy(cData + pos, BOLTFILE_HEADER);
	pos += sizeof(BOLTFILE_HEADER);

	//write len
	(*(uLong *)(cData + pos)) = mLength;
	pos += sizeof(uLong);

	PRINTF("cLen: %ld\n", cLen);

	//write compressed data
	int ret = compress2((Bytef*)(cData + pos), &cLen,
						(Bytef*)mData, mLength,
						Z_BEST_COMPRESSION);
	cLen += pos;

	PRINTF("Ret:%i\n", ret);
	if ( ret != Z_OK )
	{
		PRINTF("Compress2 error: ");
		switch (ret)
		{
			case Z_MEM_ERROR: PRINTF("Z_MEM_ERROR\n"); break;
			case Z_BUF_ERROR: PRINTF("Z_BUF_ERROR\n"); break;
			case Z_STREAM_ERROR: PRINTF("Z_STREAM_ERROR\n"); break;
			default: PRINTF("CODE:%i\n", ret); break;
		}
		free(cData);
		return 0;
	}

	PRINTF("Closing\n");

	//encrypt
	if (!_scramble(cData, cLen, pos))
	{
		PRINTF("Could not scramble!\n");
		free(cData);
		return 0;
	}

	//copy to our memory
	free(mData);
	mData = cData;
	mLength = cLen;

	PRINTF("Read %ld bytes, Wrote %ld bytes, Compression factor %4.2f%%\n",
		len, cLen, (1.0 - cLen * 1.0 / len) * 100.0);

	return 1;
}

//decrypt data starting from pos
int BoltFile::_unscramble(char* data, uLong len, uLong pos)
{
	if (mPassword.empty()) //don't decrypt
		return 1;

	uLong passPos = 0;
	uLong end = pos + mEncryptLength;
	while ( pos < len && (pos < end || mEncryptLength == 0) )
	{
		//Rotate byte based on password letter
		*(data + pos) -= mPassword.at(passPos);

		passPos++;
		if (passPos >= mPassword.length())
			passPos = 0;

		pos++;
	}

	return 1;
}

//encrypt data starting from pos
int BoltFile::_scramble(char* data, uLong len, uLong pos)
{
	if (mPassword.empty()) //don't encrypt
		return 1;

	uLong passPos = 0;
	uLong end = pos + mEncryptLength;
	while ( pos < len && (pos < end || mEncryptLength == 0) )
	{
		//Rotate byte based on password letter
		*(data + pos) += mPassword.at(passPos);

		passPos++;
		if (passPos >= mPassword.length())
			passPos = 0;

		pos++;
	}

	return 1;
}

void BoltFile::Flush()
{
	if (mData)
	{
		free(mData);
		mData = NULL;
	}
	mLength = 0;
}

