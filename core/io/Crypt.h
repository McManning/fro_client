
#ifndef _CRYPT_H_
#define _CRYPT_H_

#include "../Common.h"

static const string base64_generic = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";
			 
static const string base64_custom =
			 "qirxhpblmcodjngwtfzukveysa"
			 "ASYEVKUZFTWGNJDOCMLBPHXRIQ"
 			 "+-5478129630";

string base64_decode(const char* encoded_string, int in_len, 
						const string& charset = base64_custom, char endbyte = '.');
string base64_encode(const char* bytes_to_encode, int in_len, 
						const string& charset = base64_custom, char endbyte = '.');

string makePassword(string a, string b);
string decryptString(string msg, string pass);
string encryptString(string msg, string pass);
void scrambleString(string& msg, string pass, bool descramble);

void CPHP_Encrypt(string& s, const string& key);
void CPHP_Decrypt(string& s, const string& key);

#endif //_CRYPT_H_
