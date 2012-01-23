
#include <time.h>
#include <cstdarg>
#include <fstream>
#include "Logger.h"

Logger logger;

Logger::Logger() :
	mLog(0),
	mErrorLog(0)
{
}

Logger::~Logger()
{
}

void Logger::Write(const string& s)
{
	_prefix(mLog);

	fprintf(mLog, s.c_str());

	_suffix(mLog);
}

void Logger::Write(const char* fmt, ...)
{
    va_list args;

	_prefix(mLog);

	va_start(args, fmt);
    vfprintf(mLog, fmt, args);
	va_end(args);

	_suffix(mLog);
}

void Logger::WriteError(const string& s)
{
    _prefix(mErrorLog);

	fprintf(mErrorLog, s.c_str());

	_suffix(mErrorLog);
}

void Logger::WriteError(const char* fmt, ...)
{
    va_list args;

	_prefix(mErrorLog);

	va_start(args, fmt);
    vfprintf(mErrorLog, fmt, args);
	va_end(args);

	_suffix(mErrorLog);
}

void Logger::_prefix(FILE* log)
{
	time_t now;
	struct tm* info;
	time(&now);
	info = localtime(&now);

	char buffer[16];
	strftime(buffer, 16, "[%H:%M:%S] ", info);
	fprintf(log, buffer);
}

void Logger::_suffix(FILE* log)
{
	fprintf(log, "\n");
	fflush(log);
}

int Logger::Start()
{
	time_t now;
	struct tm* info;

	// open a log
	mLog = fopen("logs/info.log", "w");
	if (!mLog)
	{
		printf("Could not open handle to info.log\n");
		return 0;
	}

	mErrorLog = fopen("logs/error.log", "a");
	if (!mErrorLog)
	{
        printf("Could not open handle to error.log\n");
        return 0;
	}

	// write header
	time(&now);
	info = localtime(&now);

	char buffer[32];
	strftime(buffer, 32, "%c", info);

	fprintf(mLog, "** Log started on %s **\n", buffer);

	return 1;
}

void Logger::Close()
{
    fprintf(mLog, "** Closing Logger **\n");
    if (mLog)
    {
		fclose(mLog);
        mLog = 0;
    }

    if (mErrorLog)
    {
        fclose(mErrorLog);
        mErrorLog = 0;
    }
}
