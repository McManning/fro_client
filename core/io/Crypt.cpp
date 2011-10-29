
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


#include "Crypt.h"

             //WHXYEVKRIGNJDQMLBPASOCUZFT
			 //ASYEVKUZFTWGNJDOCMLBPHXRIQ
			 //qirxhpblmcodjngwtfzukveysa
			 //jnBPR52!)4gKASO$%qil7N+MLzxd39mIGUwc&^~ukvJD{:|oQ?}]-ysa*fe#Chp(@*rtb

static inline bool is_base64(unsigned char c, const string& charset) 
{
  return (charset.find(c) != string::npos);
}

string base64_encode(const char* bytes_to_encode, int in_len, const string& charset, char endbyte) 
{
  string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += charset[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += charset[char_array_4[j]];

    while((i++ < 3))
      ret += endbyte;

  }

  return ret;

}

string base64_decode(const char* encoded_string, int in_len, const string& charset, char endbyte) 
{
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  string ret;

  while (in_len-- && ( encoded_string[in_] != endbyte) && is_base64(encoded_string[in_], charset)) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = charset.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = charset.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

//Simple byte shifting based on the password characters. 
//(TODO: What if it results in NULL? Would it cut off the string prematurely? Haven't had an issue yet.. but still...)
void scrambleString(string& msg, string pass, bool descramble) 
{
	int passChar = 0;
	//(en)(de)crypt using the password characters as keys
	for(int i = 0; i < msg.length(); i++)
	{
		if (descramble)
		{
        	msg.at(i) -= pass.at(passChar); // * passMark;
		}
		else 
		{
		  	msg.at(i) += pass.at(passChar); // * passMark;
        }
		i++;
		passChar++;
		if (passChar >= pass.length()) 
			passChar = 0;
    }
}

string encryptString(string msg, string pass) 
{
	//return msg;
	scrambleString(msg, pass, false);
	return base64_encode(msg.c_str(), msg.length());  
}

string decryptString(string msg, string pass) 
{
	//return msg;
	msg = base64_decode(msg.c_str(), msg.length());
	scrambleString(msg, pass, true);
	return msg;
}

string makePassword(string a, string b)
{
	a += b; //should do something better here but I'm lazy
	return base64_encode(a.c_str(), a.length());
}

/* 
	Encrypt/Decrypt algorithms to work with a similar PHP function set
*/

void CPHP_Encrypt(string& s, const string& key) 
{
	string result;
	int len = s.length();
	char c, keyc;
	
	for (int i = 0; i < len; ++i) 
	{
		c = s.at(i);
		keyc = key.at( (i % key.length()) - 1 );
		c = c + keyc;
		result += c;
	}

	s = base64_encode(result.c_str(), result.length(), 
						base64_generic, '=');
}

void CPHP_Decrypt(string& s, const string& key) 
{
	string result;
	int len = s.length();
	char c, keyc;

	s = base64_decode(s.c_str(), s.length(), 
						base64_generic, '=');

	for (int i = 0; i < len; ++i) 
	{
		c = s.at(i);
		keyc = key.at( (i % key.length()) - 1 );
		c = c - keyc;
		result += c;
	}

	s = result;
}
