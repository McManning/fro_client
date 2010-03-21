
#ifndef _CRYPT_H_
#define _CRYPT_H_

#include "../Common.h"

string base64_decode(const char* encoded_string, int in_len);
string base64_encode(const char* bytes_to_encode, int in_len);

string makePassword(string a, string b);
string decryptString(string msg, string pass);
string encryptString(string msg, string pass);
void scrambleString(string& msg, string pass, bool descramble);

#endif //_CRYPT_H_
