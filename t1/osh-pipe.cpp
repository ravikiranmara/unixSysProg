#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>

const int status_success = 0;
const int PipeReadIndex = 0;
const int PipeWriteIndex = 1;

int DoPipeTest();

int main(int argc, char* argv[])
{
    DoPipeTest();

    printf("Exit from program\n");
    return 0;
}


int DoPipeTest()
{
    int fd[2];      // file descriptors for pipe
    int status = status_success;
    pid_t child = -1;
    char message[] = "The parent wants you home by 8\n";
    int messageLength = strlen(message);

    
    // create pipe
    printf("Creating pipe\n");
    if(-1 == pipe(fd))
    {
        status = errno;
        printf("Error creating pipe:(%d) - %s", errno, strerror(errno));
        goto exit1;
    }

    // create child
    printf("Forking child\n");
    if(-1 == (child = fork()))
    {
        status = errno;
        printf("Error Creating chlild:(%d) - %s", errno, strerror(errno));
        goto exit2;
    }

    // child close write pipe
    if(child == 0)
    {
        close(fd[PipeWriteIndex]);

        // now do child processing
        char buffer[PIPE_BUF+1];
        ssize_t readBytes = 0;

        // read from pipe 
        readBytes = read(fd[PipeReadIndex], buffer, PIPE_BUF);
        if(-1 == readBytes)
        {
            status = errno;
            printf("Child error while reading file:(%d) - %s\n", errno, strerror(errno));
            goto exit3;
        }
// null terminate to print. here we are sending only string,so its ok
        buffer[readBytes] = '\0';
        printf("Child got the following message : %s \n", buffer);

        exit(0);
    }
    
    // parent close read pipe
    if(child > 0)
    {
        close(fd[PipeReadIndex]);

        // check message length is less than pipe_buf
        // (in our case it is. otherwise, we need to write in loop)
        int i = 0;
        int writeLength;
        while(messageLength > 0)
        {
            writeLength = messageLength;
            if(messageLength > PIPE_BUF)
            {
                writeLength = PIPE_BUF;
            }

            // send message from parent
            ssize_t writeBytes = 0;
            writeBytes = write(fd[PipeWriteIndex], message + i, writeLength);
            if(-1 == writeBytes)
            {
                status = errno;
                printf("Parent error while writing to file:(%d) - %s\n", errno, strerror(errno));
                goto exit3;
            }

            i += writeLength;
            messageLength -= writeLength;
        }

        // now wait for the child to exit
        int childExitStatus = -1;
        if(-1 == waitpid(child, &childExitStatus, 0))
        {
            status = errno;
            printf("Parent error while waiting :(%d) - %s\n", errno, strerror(errno));
            goto exit3;
        }

        if(true == WIFEXITED(status))
        {
            printf("Parent - child exited successfully\n");
        }
    }

exit3:    
exit2:
    close(fd[PipeReadIndex]);
    close(fd[PipeWriteIndex]);

exit1:
    return 0;
}
