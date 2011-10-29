
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


#ifndef _BOLTFILE_H_
#define _BOLTFILE_H_

#include "../Common.h"

#define BOLTFILE_HEADER "BOLT"

class BoltFile
{
  public:
	BoltFile();
	~BoltFile();

	/*
		Load from a file or memory location into mData.
		If from a location in memory, will simply take the address.
		(Do not free the buffer after calling Load)
	*/
	int Load(string file);
	int Load(char* buffer, uLong length);
	
	/*
		Save the contents of mData to the specified file.
		To save an encrypted version, call Encrypt() before this, etc.
	*/
	int Save(string file);
	
	
	/*	Encrypts the contents of mData
		Returns:	2 if mData is already detected as encrypted (Could be corrupt)
					1 if successfully encrypted mData
					0 if an error occured while encrypting
	*/
	int Encrypt();
	
	/*	Decrypts the contents of mData
		Returns:	2 if mData is already detected as decrypted (Could be corrupt)
					1 if successfully decrypted mData
					0 if an error occured while decrypting
	*/
	int Decrypt();

	//Unloads file from memory
	void Flush();
	
	//Contains the file data, in either an encrypted or decrypted state
	char* mData;
	uLong mLength;
	
	/* 	Password and encryption length used for the file. Must 
		Use the same password for both encrypt and decrypt
		If mPassword is empty, will not (en/de)crypt.
		If mEncryptLength is 0, will (en/de)crypt the entire file.
			(This excludes the header)
	*/
	string mPassword;
	uLong mEncryptLength;
	
  private:
	/*
		Utilities used by Encrypt/Decrypt
	*/
	int _unscramble(char* data, uLong len, uLong pos);
	int _scramble(char* data, uLong len, uLong pos);
	
};

#endif //_BOLTFILE_H_





