
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
