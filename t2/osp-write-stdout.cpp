#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

const int status_success = 0;
const int MaxMessageLength = 1024;

int WriteToStdout(char *message);

int main(int argc, char *argv[])
{
    char message[] = "put this message in the stdout please";
    return WriteToStdout(message);
}


int WriteToStdout(char *message)
{
    // int stdOutFd = dup(STDOUT_FILENO);
    int status = status_success;
    
    // returns MaxMessageLength if len(message) > MaxMessageLength
    // how to detect silent truncation 
    size_t length = strnlen(message, MaxMessageLength);

    if(-1 == write(STDOUT_FILENO, message, length))
    {
        status = errno;
        printf("Unable to write:(%d) - %s", errno, strerror(errno));
        goto exit1;
    }

exit1:
    return status;
}
