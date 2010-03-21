
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





