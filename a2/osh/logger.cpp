#ifndef _OSH_LOGGER_CPP_
#define _OSH_LOGGER_CPP_

#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include "logger.h"
#include <errno.h>
#include <string.h>
#include <sstream>

using std::cerr;

int Logger::OpenFile(string filepath)
{
    int file = -1;

    if(-1 == (file = open(filepath.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO)))
    {
        cerr << "Error Opening input redir file:(" << errno << ")-" << strerror(errno);
        return errno;
    }

    this->fid = file;

    return logger_success;
}


int Logger::CloseFile()
{
    // check if it is std in, out or err
    if(this->fid > 2)
    {
        close(this->fid);
    }

    return logger_success;
}


Logger::Logger()
{
    // default out it stdout
    this->fid = STDOUT_FILENO;
    this->logLevel = none;

    return;
}

Logger::Logger(string filepath, LogLevel level)
{
    this->OpenFile(filepath);
    this->logLevel = level;

    return;
}

int Logger::set_logLevel(LogLevel level)
{
    this->logLevel = level;
}

string Logger::get_logLevelString(LogLevel level)
{
    string message;

    switch(level)
    {
        case none:
            message = log_none;
            break;

        case critical:
            message = log_critical;
            break;

        case error:
            message = log_error;
            break;

        case warning:
            message = log_warning;
            break;

        case debug:
            message = log_debug;
            break;

        case info:
            message = log_info;
            break;

        case verbose:
            message = log_verbose;
            break;
    }

    return message;
}

bool Logger::isLogMessage()
{
    return (this->logLevel > none)? true : false;
}

void Logger::log(LogLevel level, string message)
{
    string errString = this->get_logLevelString(level);
    errString += message;

    if(level < this->logLevel)
    {
        pthread_mutex_lock(&mutex1);
        write(this->fid, message.c_str(), message.length());
        pthread_mutex_unlock(&mutex1);
    }

    return;
}

void Logger::log(LogLevel level, string message, int err)
{
    std::ostringstream strstrm;
    strstrm << errno ;
    string errString = this->get_logLevelString(level);
    errString += message;
    errString += strstrm.str();
    //iota(err, buffer, 10);
    //errString += buffer;
    errString += strerror(err);

    if(level < this->logLevel)
    {
        pthread_mutex_lock(&mutex1);
        write(this->fid, errString.c_str(), errString.length());
        pthread_mutex_unlock(&mutex1);
    }

    return;
}


#endif

