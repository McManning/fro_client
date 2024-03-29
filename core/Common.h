
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


#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include <vector>
#include <math.h>

// TODO: Include the vars here somehow without actually referencing fro_private.h!
#include "../build/build.h"

/*	In case the compiler doesn't define it for us */
#ifndef WIN32
#define WIN32
#endif

#ifndef M_PI
#define M_PI	3.141592654
#endif

typedef unsigned short int uShort; //2 bytes: 0 to 65,535
typedef signed short int sShort; //2 bytes: -32,768 to 32,767
typedef unsigned char byte; //1 byte: 0 to 256
typedef unsigned long int uLong; //4 bytes: 0 to 4,294,967,295

//OPTIMIZETODO: Stop using std
typedef std::string string;
typedef std::vector<string> vString; //Shorthand data type since we use vectors of strings so often

#define sgn(x) ((x<0)?-1:((x>0)?1:0))
//Have it #define abs(x) ((x) > 0 ? (x) : -(x))

#define DEG2RAD(theta) ( theta * M_PI / 180 )
#define RAD2DEG(r) ( r * 180 / M_PI )

#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) > (b)) ? (b) : (a))

#define SAFEDELETE(_p) \
	if (_p) { \
		delete _p; \
		_p = NULL; \
	}

struct exception //Structure for a basic exception error
{
	exception(const char* _file, int _line, int _code)
	{
		file = _file;
		line = _line;
		code = _code;
	};
	const char* file;
	int code; //custom code if we want. Good for tracking multiple throws close together
	int line;
};

#define DIR_CACHE "cache/"

#ifdef __DEBUG__
#	define DEBUG
#endif

#define WARNING(msg) ( _warning(__FILE__, __LINE__, msg) )
#define FATAL(msg) ( _fatal(__FILE__, __LINE__, msg) )
#define ASSERT(val) if (!(val)) { _fatal(__FILE__, __LINE__, "Assertion Failed (" + string(#val) + ")"); }

#ifdef DEBUG
#	define DEBUGOUT(msg) ( logger.Write(msg) )
#	define PRINTF logger.Write

#else
#	define DEBUGOUT
#	define PRINTF

#endif

void _warning(const char* file, int line, string msg);
void _fatal(const char* file, int line, string msg);

//void* newMemory(const char* file, int line, int size, const char* id);
//void deleteMemory(const char* file, int line, void* data, const char* id);

#define ALPHA_OPAQUE 255
#define ALPHA_TRANSPARENT 0

struct color
{
	color(byte _r=0, byte _g=0, byte _b=0, byte _a = ALPHA_OPAQUE)
	{
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	};
	byte r, g, b, a;
};

struct rect
{
	rect(sShort _x=0, sShort _y=0, uShort _w=0, uShort _h=0)
	{
		x = _x;
		y = _y;
		w = _w;
		h = _h;
	};
	uShort w, h;
	sShort x, y;
};

//cuz I don't like using rects for shit that doesn't require a w/h
struct point2d
{
	point2d() { x = y = 0; };
	point2d(sShort _x, sShort _y) {
		x = _x; y = _y;
	};
	sShort x, y;
};

typedef unsigned char direction; //Was once typedef enum, but it creates incompatability issues
enum //matches location of keypad keys
{
	//SKIP 0
	SOUTHWEST = 1,
	SOUTH,
	SOUTHEAST,
	WEST,
	//SKIP 5
	EAST = 6,
	NORTHWEST,
	NORTH,
	NORTHEAST
};

//Global application state that allows all modules to behave
//differently based on this state.
typedef enum
{
	APPSTATE_STARTING = 0,
	APPSTATE_RUNNING,
	APPSTATE_CLOSING,
	APPSTATE_ERROR
} applicationState;
extern applicationState appState;

bool isAppClosing();

//seeds both rnd and rnd2
void seedRnd(uLong ulSeed = 0);

/*	if seed is specified, it will use that seed and then set
	the seed to its MODIFIED AMOUNT. This way we can run repeat rnd patterns
	without disrupting the randomization of other parts of the engine.
*/
int rnd(int min, int max, uLong* seed = NULL);

//Another form of randomizer, uses the C rand() function
int rnd2(int min, int max);

//Simple conversion functions, handy as hell
int sti(string str);
double stod(string str);
string its(int i);
string dts(double d);
string pts(void* p);
string cts(color c);
string rts(rect r);
string p2dts(point2d p);

bool isDefaultRect(rect r);
bool isDefaultColor(color c);
bool isDefaultPoint2d(point2d p);

string serializeRect(rect r);
rect deserializeRect(string s);

bool areRectsIntersecting(rect a, rect b);
bool isPointInRect(rect r, sShort x, sShort y);
bool isPointInPie(point2d center, int radius, int x, int y, int minDeg, int maxDeg);
bool isRectInPie(point2d center, int radius, rect r, int minDeg, int maxDeg);

double getDistance(point2d a, point2d b);

//Returns the resulting rect from the intersection of the two input rects.
//If the are not intersecting, will return a default rect. (0,0,0,0)
rect rectIntersection(const rect& a, const rect& b);

//Below functions are based on the keypad directions. (2 = south, 4 = west, etc)
sShort directionToAngle(direction dir);
char directionToChar(direction dir);
direction charToDirection(char c);

string directionToString(direction dir);
direction stringToDirection(string s);

//Change the rect's x/y based on how far we're moving in a particular direction (using our 8 dir system, not abstract)
void offsetRectByDirection(rect* r, direction dir, sShort distance);
//Change rects x/y using polar coordinates
void offsetRectByAngle(rect* r, sShort theta, sShort distance);

//Explode a string into an vString based on the delimiter. If limit # of children are added, it dumps the rest and exits.
// if includeBlanks is false, it will ignore blank strings
//		(those that are between two delims. Ex: Delim = *, string is **msg***. if includeBlanks is false, the only thing in result will be msg
//		otherwise, it'll have blank strings filled around msg)
bool explode(vString* result, string* str, string delim, bool includeBlanks = false, int limit = 0);

//Condense an vString back into a string, adding delim between each member
bool implode(string* result, vString* vs, string delim);

//File stuff should go into its own cpp..
bool fileExists(string filename);

string GetFilenameFromUrl(string url);

int wildmatch(const char *wild, const char *string);
string lowercase(string msg);
void replace(string* src, string a, string b);

color slashCtoColor(string msg);

bool isGreyscale(color c);

bool isDark(color c);

color invertColor(color c);

string colorToHex(color c); //color(255,0,0) -> FF0000
color hexToColor(string hex); //FF0000 -> color(255,0,0)
int hexToDec(string hex);

string htmlSafe(string s);

string stripColorCodes(const string& msg);
string stripCodes(const string& msg);

/*
	Will read numbers in the form of:
		15px, 15tx, 15, etc.
		15 and 15 px being the same. 15tx will result in a return of 15^16
*/
int readFormattedNumber(string num);

string generateWord(int length); //Returns a somewhat human readable word length long.
char getVowel();

/*	Returns stamp in the form of YYMMDD, if clock is true
	returns YYMMDD[HHMMSS]
*/
string timestamp(bool clock);
string shortTimestamp();

bool isWhitespace(char c);

void flashWindowState(bool bInvert);

#endif //_COMMON_H_

