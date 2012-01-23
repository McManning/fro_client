
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "Common.h"

class Logger
{
  public:
	Logger();
	~Logger();

	int Start();
	void Close();

	void Write(const string& s);
	void Write(const char* fmt, ...);

    void WriteError(const string& s);
	void WriteError(const char* fmt, ...);

  protected:

	void _prefix(FILE* log);
	void _suffix(FILE* log);

	FILE* mLog;
	FILE* mErrorLog;
};

extern Logger logger;

#endif //_LOGGER_H_

