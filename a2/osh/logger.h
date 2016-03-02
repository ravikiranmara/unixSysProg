#ifndef _OSH_LOGGER_H_
#define _OSH_LOGGER_H_

#include <string>
#include <pthread.h>

using std::string;

const int logger_success = 0;
const int logger_fail = 1;

typedef enum ErrorLevel
{
    none,
    critical,
    error,
    warning,
    debug,
    info,
    verbose,
} LogLevel;

const string log_none = "NONE";
const string log_critical = "CRITICAL";
const string log_error = "ERROR";
const string log_warning = "WARNING";
const string log_debug = "DEBUG";
const string log_info = "INFO";
const string log_verbose = "VERBOSE";

class Logger
{
  private:
    int fid;
    pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

    int OpenFile(string filepath);
    int CloseFile();
    string get_logLevelString(LogLevel level);

    LogLevel logLevel;

  public:
    Logger();
    Logger(string filepath, ErrorLevel errorLevel = debug);

    int set_logLevel(LogLevel level);
    bool isLogMessage();

    void log(LogLevel level, string message);
    void log(LogLevel level, string message, int errno);
};

#endif
