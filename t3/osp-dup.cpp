#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

const int status_success = 0;
const int PipeReadIndex = 0;
const int PipeWriteIndex = 1;

int CheckFD(void);

int main(int argc, char* argv[])
{
    return CheckFD();
}

int CheckFD()
{
    int status = status_success;
    int fd[2];
    char message[] = "Hello World\n";
    size_t length = strlen(message);

    // creat pipe
    if(-1 == pipe(fd))
    {
        status = errno;
        printf("Error creating pipe:(%d) - %s", errno, strerror(errno));
        goto exit1;
    }

    // dup write of pipe to stdout
    if(-1 == dup2(STDOUT_FILENO, fd[PipeWriteIndex]))
    {
        status = errno;
        printf("Error duplicating stdout:(%d) - %s", errno, strerror(errno));
        goto exit2;
    }

    // now write to pipe?
    if(-1 == write(fd[PipeWriteIndex], message, length))
    {
        status = errno;
        printf("Unable to write to the pipe:(%d) - %s", errno, strerror(errno));
        goto exit3;
    }

exit3:
exit2:
    close(fd[PipeReadIndex]);
    close(fd[PipeWriteIndex]);
            
exit1:
    return status;

}
