/* simple program to read from input and write to output
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

const int status_success = 0;
const int MaxBuf = 1024;

int echo();

int main(int argc, char* argv[])
{
   return echo(); 
}

int echo()
{
    char buffer[MaxBuf];
    int status = status_success;
    int readlen = 0, writelen = 0;

    // read from buffer
    if(-1 == (readlen = read(STDIN_FILENO, buffer, MaxBuf)))
    {
        status = errno;
        printf("Error while reading buffer:(%d)-%s", errno, strerror(errno));
        goto exit1;
    }

    // do while the buffer is not eof
    while(readlen > 0)
    {
        if(-1 == (writelen = write(STDOUT_FILENO, buffer, readlen)))
        {
            status = errno;
            printf("Error while writing file:(%d)-%s", errno, strerror(errno));
            goto exit1;
        }

        // now try reading again
        if(-1 == (readlen = read(STDIN_FILENO, buffer, MaxBuf)))
        {
            status = errno;
            printf("Error while reading buffer:(%d)-%s", errno, strerror(errno));
            goto exit1;
        }
    }

exit1:
    return status;
}

