
#include <SDL/SDL_syswm.h>
#include <time.h>
#include "Common.h"
#include "io/FileIO.h"

applicationState appState;

void throwError(const char* file, int line, int num)
{
	systemErrorMessage("Fatal Error", "[" + string(file) + "] Code ln" + its(line) + " [" + its(num)
							+ "] \n\nREPORT THIS IMMEDIATELY!");

	FILE* f = fopen("logs/error.log", "a");
	if (f)
	{
		fprintf(f, "[%s] THROW - %s:%i - %i\n", timestamp(true).c_str(), file, line, num);
		fclose(f);
	}

	exit(0); //Hateful, but necessary on fatal errors
}

void warning(const char* file, int line, string msg)
{
	printf("WARNING [%s:%i] %s\n", file, line, msg.c_str());
	fflush(stdout); //make sure this outputs

	FILE* f = fopen("logs/error.log", "a");
	if (f)
	{
		fprintf(f, "[%s] WARNING - %s:%i - %s\n", timestamp(true).c_str(), file, line, msg.c_str());
		fclose(f);
	}
}

void fatal(const char* file, int line, string msg)
{
	printf("FATAL [%s:%i] %s\n", file, line, msg.c_str());
	fflush(stdout); //make sure this outputs
	systemErrorMessage("Fatal Error",
						"[" + string(file) + ":" + its(line) + "] "
							+ msg + "\n\nREPORT THIS IMMEDIATELY!");

	FILE* f = fopen("logs/error.log", "a");
	if (f)
	{
		fprintf(f, "[%s] FATAL - %s:%i - %s\n", timestamp(true).c_str(), file, line, msg.c_str());
		fclose(f);
	}

	exit(0); //Hateful, but necessary on fatal errors
}

void print(string msg)
{
	printf("%s\n", msg.c_str());
	fflush(stdout); //make sure this outputs
}

bool isAppClosing()
{
	return (appState == APPSTATE_CLOSING);
}

//////// RANDOM.H

#define MAX_UNSIGNED_SHORT 	(0xffff)
#define RANDOM_A 			(1103515245)
#define RANDOM_C 			(12345)
#define RANDOM_SHIFT 		(16)

uLong globalSeed;

//To get the same set of results, just set the same seed
void seedRnd(uLong ulSeed)
{
	globalSeed = (ulSeed > 0) ? ulSeed : time(NULL);
	srand(time(NULL));
}

int rnd(int min, int max, uLong* seed)
{
	if (min == max) return min;
	if (max < min)
	{
		int temp = min;
		min = max;
		max = temp;
	}

	if (!seed)
		seed = &globalSeed;

	*seed = RANDOM_A * (*seed) + RANDOM_C;

	int num = max - min;
	int us = (uShort)(((*seed) >> RANDOM_SHIFT) & MAX_UNSIGNED_SHORT)
				* (uLong)num / (MAX_UNSIGNED_SHORT + 1);

	return us + min;
}

int rnd2(int min, int max) //could always use different forms
{
	if (min == max) return min;
	if (max < min)
	{
		int temp = min;
		min = max;
		max = temp;
	}
    max -= min;
	return rand() % max + min;
}

///// TYPES.H

int sti(string str) {
  return atoi(str.c_str());
}

double stod(string str) {
  return atof(str.c_str());
}

string its(int i) {
	char nresult[14];
	sprintf(nresult, "%i", i);
	return nresult;
}

string dts(double d) {
	char nresult[14];
	sprintf(nresult, "%.12lg", d);
	return nresult;
}

string pts(void* p) {
	char nresult[10];
	sprintf(nresult, "0x%p", p);
	return nresult;
}

string cts(color c) {
	return "C{" + its(c.r) + "," + its(c.g) + ","
			+ its(c.b) + "," + its(c.a) + "}";
}

string rts(rect r) {
	return "(" + its(r.x) + "," + its(r.y) + ","
			+ its(r.w) + "," + its(r.h) + ")";
}

string p2dts(point2d p) {
	return "(" + its(p.x) + "," + its(p.y) + ")";
}


////////// RECT.H

bool isDefaultRect(rect r)
{
	return (r.x == 0 && r.y == 0 && r.w == 0 && r.h == 0);
}

bool isDefaultPoint2d(point2d p)
{
	point2d pp;
	return (p.x == pp.x && p.y == pp.y);
}

string serializeRect(rect r)
{
	vString v;
	v.push_back(its(r.x));
	v.push_back(its(r.y));
	v.push_back(its(r.w));
	v.push_back(its(r.h));

	string s;
	implode(&s, &v, " ");
	return s;
}

rect deserializeRect(string s)
{
	vString v;
	if ( !explode(&v, &s, " ") || v.size() < 4 )
		return rect();
	else
		return rect(sti(v.at(0)),
					sti(v.at(1)),
					sti(v.at(2)),
					sti(v.at(3))
					);
}

bool isPointInRect(rect r, sShort x, sShort y)
{
	return (x >= r.x
        	&& x < r.x + r.w-1
        	&& y >= r.y
        	&& y < r.y + r.h-1);
}

bool isPointInPie(point2d center, int radius, int x, int y, int minDeg, int maxDeg)
{
	double min = (double)minDeg * M_PI / 180;
	double max = (double)maxDeg * M_PI / 180;

	double dx = x - center.x;
	double dy = y - center.y;

	//Check if within boundary degrees.
	double theta = atan2( dy, dx );

	if (theta < min || theta > max)
		return false;

 	// != 0 is for game purposes only. Gives infinite vision.
	if (radius != 0)
	{
		//Check if within radius
		double d = sqrt((center.x - x) * (center.x - x)
					+ (center.y - y) * (center.y - y) );

		if (d > radius)
			return false;
	}

	return true;
}

double getDistance(point2d a, point2d b)
{
	return sqrt((a.x - b.x) * (a.x - b.x)
				+ (a.y - b.y) * (a.y - b.y) );
}

bool isRectInPie(point2d center, int radius, rect r, int minDeg, int maxDeg)
{
	if (isPointInPie(center, radius, r.x, r.y, minDeg, maxDeg)) return true;
	if (isPointInPie(center, radius, r.x+r.w-1, r.y+r.h-1, minDeg, maxDeg)) return true;
	if (isPointInPie(center, radius, r.x, r.y+r.h-1, minDeg, maxDeg)) return true;
	if (isPointInPie(center, radius, r.x+r.w-1, r.y, minDeg, maxDeg)) return true;
	return false;
}

//FIXED: TOUCHING RECTANGLES DOES NOT EQUAL INTERSECTING RECTANGLES

bool areRectsIntersecting(rect a, rect b)
{
	return !( b.x > a.x+a.w
	        || b.x+b.w < a.x
	        || b.y > a.y+a.h
	        || b.y+b.h < a.y
	        );
/*
    //Go through boxTwos corners and see if they're in boxOne

    if (isPointInRect(a, b.x, b.y)) return true;
    if (isPointInRect(a, b.x + b.w-1, b.y + b.h-1)) return true;
    if (isPointInRect(a, b.x, b.y + b.h-1)) return true;
    if (isPointInRect(a, b.x + b.w-1, b.y)) return true;

    //go through boxOnes corners and see if they're in boxTwo
    if (isPointInRect(b, a.x, a.y)) return true;
    if (isPointInRect(b, a.x + a.w-1, a.y + a.h-1)) return true;
    if (isPointInRect(b, a.x, a.y + a.h-1)) return true;
    if (isPointInRect(b, a.x + a.w-1, a.y)) return true;

    //Didn't match up at all
    return false;*/
}

rect rectIntersection(const rect& a, const rect& b)
{
	rect r;
	r.x = MAX(a.x, b.x);
	r.y = MAX(a.y, b.y);
	sShort x2 = MIN(a.x+a.w, b.x+b.w);
	sShort y2 = MIN(a.y+a.h, b.y+b.h);

	//check for invalid intersection
	if (x2 < r.x || y2 < r.y)
		return rect();

	r.w = x2 - r.x;
	r.h = y2 - r.y;

	return r;
}

void offsetRectByAngle(rect* r, sShort theta, sShort distance)
{
	r->x += static_cast<sShort>(distance * cos(theta * M_PI / 180));
	r->y += static_cast<sShort>(distance * sin(theta * M_PI / 180));
}

void offsetRectByDirection(rect* r, direction dir, sShort distance)
{
	//offsetRectByAngle(r, directionToAngle(dir), distance);
	/*Can't use the angle calculation because 45deg and whatnot doesn't work
		with our current movement calculations. Yes I know diagonals are
		going to be a bit faster and further than moving horiz/vert, but the
		grid is small and not very noticeable
	*/
	switch (dir)
	{
		case SOUTHEAST:
			offsetRectByAngle(r, 90, distance); //do south
			offsetRectByAngle(r, 0, distance); //do east
			break;
		case SOUTHWEST:
			offsetRectByAngle(r, 90, distance); //do south
			offsetRectByAngle(r, 180, distance); //do west
			break;
		case NORTHEAST:
			offsetRectByAngle(r, 270, distance); //do north
			offsetRectByAngle(r, 0, distance); //do east
			break;
		case NORTHWEST:
			offsetRectByAngle(r, 270, distance); //do north
			offsetRectByAngle(r, 180, distance); //do west
			break;
		default: //angle offset is fine for horiz/vert
			offsetRectByAngle(r, directionToAngle(dir), distance);
			break;
	}
}

///////// COLOR.H

bool isDefaultColor(color c)
{
	return (c.r == 0 && c.g == 0 && c.b == 0 && c.a == 255);
}

bool isGreyscale(color c)
{
	return (c.r == c.g && c.g == c.b);
}

color invertColor(color c)
{
	return color(255-c.r, 255-c.g, 255-c.b);
}

bool isDark(color c)
{
	return sqrt(c.r * c.r + c.g * c.g + c.b * c.b) < 130;
}

//Return RGB value of a 3 letter string. (0->9 for each part)
color slashCtoColor(string msg)
{
	if (msg.find("\\c", 0) == 0)
		msg.erase(0, 2);

	if (msg.length() < 3)
		return color();

	color c;
	c.r = sti(msg.substr(0, 1));
	c.g = sti(msg.substr(1, 1));
	c.b = sti(msg.substr(2, 1));

	if (c.r > 9) c.r = 9;
	if (c.g > 9) c.g = 9;
	if (c.b > 9) c.b = 9;

	c.r *= 28;
	c.g *= 28;
	c.b *= 28;
	return c;
}

string colorToHex(color c) //color(255,0,0) -> FF0000
{
	char htmlcolor[7];
	sprintf(htmlcolor, "%02X%02X%02X", c.r, c.g, c.b);
	return htmlcolor;
}

int hexToDec(string hex)
{
    int value = 0;

    int a = 0;
    int b = hex.length() - 1;
    for (; b >= 0; a++, b--)
    {
        if (hex[b] >= '0' && hex[b] <= '9')
        {
            value += (hex[b] - '0') * (1 << (a * 4));
        }
        else
        {
            switch (hex[b])
            {
                case 'A':
                case 'a':
                    value += 10 * (1 << (a * 4));
                    break;
                case 'B':
                case 'b':
                    value += 11 * (1 << (a * 4));
                    break;
                case 'C':
                case 'c':
                    value += 12 * (1 << (a * 4));
                    break;
                case 'D':
                case 'd':
                    value += 13 * (1 << (a * 4));
                    break;
                case 'E':
                case 'e':
                    value += 14 * (1 << (a * 4));
                    break;
                case 'F':
                case 'f':
                    value += 15 * (1 << (a * 4));
                    break;
                default: return 0;
            }
        }
    }
    return value;
}

color hexToColor(string hex) //FF0000 -> color(255,0,0)
{
	if (hex.at(0) == '#')
		hex.erase(0, 1);

	color c;
	if (hex.size() > 1)
		c.r = hexToDec(hex.substr(0, 2));
	if (hex.size() > 3)
		c.g = hexToDec(hex.substr(2, 2));
	if (hex.size() > 5)
		c.b = hexToDec(hex.substr(4, 2));

	return c;
}

/////////// STRING.H

bool explode(vString* result, string* str, string delim, bool includeBlanks, int limit)
{
	if (!str || !result || str->empty() || delim.empty()) return false;

	string tmp_str;
	size_t pos;
	size_t strpos = 0;
	while (true)
	{
		pos = str->find(delim, strpos);

		if (pos != string::npos)
		{
			tmp_str = str->substr(strpos, pos - strpos);
			if (!tmp_str.empty() || includeBlanks)
				result->push_back(tmp_str);
			strpos = pos + delim.length();
		}
		else //no more delimiters
		{
			if (strpos <= str->length())
			{
				result->push_back(str->substr(strpos));
			}
			return true;
		}

		if (result->size() >= limit && limit != 0) //dump rest and done
		{
			result->push_back(str->substr(strpos));
			return true;
		}
	}
	return true;
}

bool implode(string* result, vString* vs, string delim)
{
	if (!vs || !result || vs->empty()) return false;

	for (uShort i = 0; i < vs->size(); i++)
	{
		*result += vs->at(i) + delim;
	}

	return true;
}

/*
	Convert a URL to a filename saveable in our cache. Should be platform-safe, etc.
	Convert backslashes to dash,  \ / : * ? " < > | are all invalid in windows.
	Personally, should convert all non a-zA-z0-9 to hex. Or just Base64 the whole url.

	This new system needs to memorize directory structure to a point on the remote server so that
	we can avoid overwriting (or not overwriting) dupe versions from other locations. Either prefix
	by zone ID or prefix by a safe version of the webservers directory structure. Of course.. the issue
	with that is directories for general-use sites tend to change. And if people host files on a non-dedicated
	server, we can run into structural conflict.
*/

string GetFilenameFromUrl(string url)
{
	size_t i = url.find_last_of('/') + 1;
	if (i == string::npos || i > url.size()-1)
		return url;
	else
		return url.substr(i);
}

/*
	if (wildmatch("bl?h.*", "blah.jpg")) {
	  //we have a match!
	} else {
	  //no match =(
	}

	? is one character replacement, * is as many as we want. =]
	do NOT send a "" string in here else whole thing will fucking crash 8]
*/
int wildmatch(const char *wild, const char *string)
{
	if (strlen(wild) == 0 || strlen(string) == 0)
		return 0;
  // Written by Jack Handy - jakkhandy@hotmail.com
  const char *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }

  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}

string lowercase(string msg)
{
	uShort i = 0;
    while (i < msg.size()) {
        msg.at(i) = tolower(msg.at(i));
        i++;
    }
    return msg;
}

void replace(string* src, string a, string b)
{
	if (src)
	{
		size_t pos = src->find(a, 0);
		while (pos != string::npos)
		{
			src->erase(pos, a.length());
			src->insert(pos, b);
			pos = src->find(a, 0);
		}
	}
}

string htmlSafe(string s)
{
	string result;
	char c[4];
	for (size_t i = 0; i < s.length(); i++)
	{
		if (isalnum(s.at(i)))
		{
			result += s.at(i);
		}
		else //convert and print
		{
			sprintf(c, "%%%02X", s.at(i));
			result += c;
		}
	}
	return result;
}

string stripColorCodes(const string& msg)
{
	string stripped;
	for (size_t i = 0; i < msg.length(); i++)
	{
		if (msg.at(i) == '\\')
		{
			if (msg.length() > i + 1) //enough room for \cRGB
			{
				if (msg.at(i + 1) == 'c')
				{
					i += 4; //skip \cRGB
					continue; //don't add
				}
			}
		}
		stripped += msg.at(i); //another renderable character~
	}
	return stripped;
}

string stripCodes(const string& msg)
{
	string stripped;
	for (size_t i = 0; i < msg.length(); i++)
	{
		if (msg.at(i) == '\\')
		{
			if (msg.length() > i + 1) //enough room for \cRGB
			{
				if (msg.at(i + 1) == 'c')
				{
					i += 4; //skip \cRGB
				}
				else if (msg.at(i+1) == 'n' || msg.at(i+1) == 't') // skip \n and \t
				{
					++i;
				}
				else
				{
					stripped += msg.at(i); //another renderable character
				}
			}
		}
		else if (msg.at(i) != '\n' && msg.at(i) != '\t') // skip REAL \n and \t
		{
			stripped += msg.at(i); //another renderable character
		}
	}
	return stripped;
}

int readFormattedNumber(string num)
{
	sShort p;

	if (num.empty())
		return 0;

	p = num.find("tx");
	if (p != string::npos)
	{
		num.erase(p);
		return sti(num) * 16;
	}

	p = num.find("px");
	if (p != string::npos)
	{
		num.erase(p);
	}
	return sti(num);
}

char getVowel()
{
	int r = rnd2(0, 4);
	switch (r)
	{
		case 1: return 'e';
		case 2: return 'i';
		case 3: return 'o';
		case 4: return 'u';
		default: return 'a';
	}
}

string generateWord(int length)
{
	srand(time(NULL));

	//abcdefghijklmnop [q] rstu [vwxyz] Don't need
	string result;
	char c;
	//uLong l = time(NULL);
	while (result.length() < length)
	{
		if (result.length() % 2)
		{
			result += getVowel();
		}
		else
		{
			c = (char)rnd2('a', 'v');
			//don't add double vowels
			if (c != 'a' && c != 'e' && c != 'i'
				&& c != 'o' && c != 'u' && c != 'q')
			{
				result += c;
			}
		}
	}
	return result;
}

string timestamp(bool clock)
{
    time_t ltime;
    struct tm *Tm;
    string stamp;

    ltime = time(NULL);
    Tm = localtime(&ltime);

    if (Tm->tm_year-100 < 10) stamp += "0"; //add extra zero
    stamp += its(Tm->tm_year-100);
    if (Tm->tm_mon+1 < 10) stamp += "0"; //add extra zero
    stamp += its(Tm->tm_mon+1);
    if (Tm->tm_mday < 10) stamp += "0"; //add extra zero
    stamp += its(Tm->tm_mday);

    if (clock)
	{
        stamp += "_";
        if (Tm->tm_hour < 10) stamp += "0"; //add extra zero
        stamp += its(Tm->tm_hour);
        if (Tm->tm_min < 10) stamp += "0"; //add extra zero
        stamp += its(Tm->tm_min);
        if (Tm->tm_sec < 10) stamp += "0"; //add extra zero
        stamp += its(Tm->tm_sec);
        //stamp += "]";
    }

    return stamp;
}

string shortTimestamp()
{
	time_t ltime;
    struct tm *Tm;
    string stamp;

    ltime = time(NULL);
    Tm = localtime(&ltime);

    //if (Tm->tm_hour < 10) stamp += "0"; //add extra zero
    stamp += its(Tm->tm_hour) + ":";
    if (Tm->tm_min < 10) stamp += "0"; //add extra zero
    stamp += its(Tm->tm_min) + ":";
    if (Tm->tm_sec < 10) stamp += "0"; //add extra zero
    stamp += its(Tm->tm_sec);

    return stamp;
}

////////// OTHER

sShort directionToAngle(direction dir)
{
	switch (dir)
	{
		case EAST: return 0;
		case SOUTHEAST: return 45;
		case SOUTH: return 90;
		case SOUTHWEST: return 135;
		case WEST: return 180;
		case NORTHWEST: return 225;
		case NORTH: return 270;
		case NORTHEAST: return 315;
		default: return 0;
	}
}

char directionToChar(direction dir)
{
	switch (dir)
	{
		case EAST: return '6';
		case SOUTHEAST: return '3';
		case SOUTH: return '2';
		case SOUTHWEST: return '1';
		case WEST: return '4';
		case NORTHWEST: return '7';
		case NORTH: return '8';
		case NORTHEAST: return '9';
		default: return '2';
	}
}

direction charToDirection(char c)
{
	switch (c)
	{
		case '1': return SOUTHWEST;
		case '2': return SOUTH;
		case '3': return SOUTHEAST;
		case '4': return WEST;
		case '6': return EAST;
		case '7': return NORTHWEST;
		case '8': return NORTH;
		case '9': return NORTHEAST;
		default: return SOUTH;
	}
}

string directionToString(direction dir)
{
	switch (dir)
	{
		case EAST: return "n";
		case SOUTHEAST: return "se";
		case SOUTH: return "s";
		case SOUTHWEST: return "sw";
		case WEST: return "w";
		case NORTHWEST: return "nw";
		case NORTH: return "n";
		case NORTHEAST: return "ne";
		default: return "s";
	}

	return string();
}

direction stringToDirection(string s)
{
	//handle numeric version
	if (s.at(0) >= '1' && s.at(0) <= '9')
		return charToDirection(s.at(0));

	//assume letters (n, e, nw, etc)
	if (s == "n") return NORTH;
	else if (s == "s") return SOUTH;
	else if (s == "e") return EAST;
	else if (s == "w") return WEST;
	else if (s == "ne") return NORTHEAST;
	else if (s == "nw") return NORTHWEST;
	else if (s == "se") return SOUTHEAST;
	else if (s == "sw") return SOUTHWEST;
	else return SOUTH;
}

bool fileExists(string filename)
{
	FILE *f;
	if((f = fopen(filename.c_str(), "r")) == NULL)
	{
		return false;
	}
	else
	{
		fclose(f);
		return true;
	}
}

bool isWhitespace(char c)
{
	return ( isspace( (unsigned char) c ) || c == '\n' || c == '\r' || c == '\t' );
}

void flashWindowState(bool bInvert)
{
#ifdef WIN32
	// TODO: Check return values
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWMInfo(&wmInfo);
	HWND hWnd = wmInfo.window;

	// http://msdn.microsoft.com/en-us/library/ms679346(v=vs.85).aspx
	FlashWindow(hWnd, bInvert);
#endif
}
