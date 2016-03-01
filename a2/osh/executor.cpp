#ifndef _OSH_EXECUTOR_CPP_
#define _OSH_EXECUTOR_CPP_

#include "executor.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

using std::cerr;

Executor::Executor()
{
    this->exitShell = false;
}

Executor::~Executor()
{
}

int Executor::fixupStdinOut(Command &command)
{
    int ifile = -1;
    int ofile = -1;
    string token;
    int length;
    char *filename = NULL;

    if(I_File == command.get_inputMode())
    {
        //cout << "udate input stream" << std::endl;
        token = command.get_inputFilename();
        length = token.length();

        filename = new char[length + 1];
        strncpy(filename, token.c_str(), length+1);
        if(-1 == (ifile = open(token.c_str(), O_RDONLY)))
        {
            cerr << "Error Opening input redir file:(" << errno << ")-" << strerror(errno);
            return errno;
        }

        // if create was successful, now dup file handle
        if(-1 == dup2(ifile, STDIN_FILENO))
        {
            cerr << "Error dup input redir file:(" << errno << ")-" << strerror(errno);
            return errno;
        }

        command.set_inputFid(ifile);
        delete filename;
    }

    if(O_FileNew == command.get_outputMode() || O_Append == command.get_outputMode())
    {
        //cout << "udate output stream" << std::endl;
        token = command.get_outputFilename();
        length = token.length();
        int openmode;
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

        filename = new char[length+1];
        strncpy(filename, token.c_str(), length+1);

        if(O_FileNew == command.get_outputMode())
        {
            openmode = O_WRONLY | O_CREAT | O_TRUNC;
        }
        else
        {
            openmode = O_WRONLY | O_APPEND;
        }

        if(-1 == (ofile = open(token.c_str(), openmode, mode)))
        {
            cerr << "Error Opening output redir file:(" << errno << ")-" << strerror(errno);
            return errno;
        }

        if(-1 == dup2(ofile, STDOUT_FILENO))
        {
            cerr << "Error dup output redir file:(" << errno << ")-" << strerror(errno);
            return errno;
        }

        command.set_outputFid(ofile);
        delete filename;
    }

    if(O_Pipe == command.get_outputMode())
    {
        // pipe is already created. just join them
        //cout << "dupe out" << std::endl;
        if(-1 == dup2(command.get_outputFid(), STDOUT_FILENO))
        {
            cerr << "Error attaching pipe to stdout:(" << errno << ")-" << strerror(errno);
            return errno;
        }
    }

    if(I_Pipe == command.get_inputMode())
    {
        //cout << "dupe in" << std::endl;
        if(-1 == dup2(command.get_inputFid(), STDIN_FILENO))
        {
            cerr << "Error attaching pipe to stdin:(" << errno << ")" << strerror(errno);
            return errno;
        }
    }

    return status_success;
}

int Executor::childExecFunction(Command &command)
{
    int rval = 0;
    string token;
    pid_t pid = -1;
    char **argv;

    command.get_executable(token);

    // command set parse state
    command.set_parseState(executing);

    // pid
    pid = getpid();
    command.set_pid(pid);

    // fixup command
    this->fixupStdinOut(command);

    // exec child
    int binlen = token.length()+1;
    char *bin = new char[binlen];
    strncpy(bin, token.c_str(), binlen);
    argv = command.get_argv();

    if(-1 == execvp(bin, argv))
    {
       rval = errno;
       cerr << "Error when execlp:(" << errno << ")- " << strerror(errno) << std::endl;
    }

    // set rval in child
    //command.set_rval(rval);

    //cout << "Child executing : " << token << " , id - " << command.get_pid() << std::endl;
    delete bin;
    exit(rval);
}

bool Executor::isCommandChainValid(Command &command)
{
    int status = true;

    return status;
}

bool Executor::isWaitForChild(Command &command)
{
    string token;
    RunNextCommandSymbol run = command.get_runNextCommand();

    if((r_exec_onfailure == run) ||
        (r_exec_onsuccess == run) ||
        (r_exec_any == run))
    {
        return true;
    }

    return false;
}

bool Executor::get_isExitFromShell()
{
    return this->exitShell;
}

bool Executor::set_isExitFromShell(bool state)
{
    this->exitShell = state;
    return this->exitShell;
}

bool Executor::isExecNextCommand(Command &command)
{
    bool rval = false;

    RunNextCommandSymbol run = command.get_runNextCommand();
    int prevRval = command.get_rval();

    if(r_exec_any == run)
    {
        rval = true;
    }

    return rval;
}

bool Executor::isSkipNextCommand(Command &command)
{
    bool rval = false;

    string token;
    RunNextCommandSymbol run = command.get_runNextCommand();
    int prevRval = command.get_rval();

    if((r_exec_onfailure == run && 0 == prevRval) ||
        (r_exec_onsuccess == run && 0 != prevRval))
    {
        rval = true;
    }

    return rval;
}

bool Executor::isCommandExit(Command &command)
{
    bool rval = false;
    string executable;
    int status = status_success;

    status = command.get_executable(executable);
    if(status_success == status)
    {
        if(0 == executable.compare(exitCommand))
        {
            rval = true;
        }
    }

    return rval;
}

int Executor::pipeCommands(Command &command, int pipeid[2])
{
    int status = status_success;

    // check if command
    if(O_Pipe != command.get_outputMode())
    {
        return status;
    }

    // create pipe
    if(-1 == pipe(pipeid))
    {
        status = errno;
        cerr << "Error creating pipe:(" << errno << ")-" << strerror(errno);
    }

    // attach to command and next
    if(status_success == status)
    {
        //cerr << "set pipe fid incommand" << std::endl;
        command.set_outputFid(pipeid[PipeWriteIndex]);
        (command.next)->set_inputFid(pipeid[PipeReadIndex]);
    }

    return status;
}

int Executor::executeCommandList(Command *command)
{
    int status = status_success;
    Command *curr, *prev;
    string bin;
    pid_t childpid = 0;
    int childRval = 0;
    int pipeid[2];

    if(NULL == command)
    {
        return status;
    }

    // validate command
    if(false == this->isCommandChainValid(*command))
    {
        // a better error message
        return status_fail;
    }

    bool piped = false;
    prev = NULL;
    curr = command;
    while(NULL != curr)
    {
        // is command exit
        if(isCommandExit(*curr))
        {
            this->set_isExitFromShell(true);
            break;
        }

        piped = false;

        // make pipe if pipe
        if(O_Pipe == curr->get_outputMode())
        {
            //cout << "call pipe" << std::endl;
            status = this->pipeCommands(*curr, pipeid);
            piped = true;
        }

        // fork child
        if(-1 == (childpid = fork()))
        {
            // fork failed
            exit(1);
        }

        if(0 == childpid)
        {
            if(true == piped)
            {
                close(pipeid[PipeReadIndex]);
            }

            this->childExecFunction(*curr);
        }
        else
        {
            if(true == piped)
                close(pipeid[PipeWriteIndex]);

            curr->get_executable(bin);
            //cout << bin << " - ";

            // wait for next command
            if(this->isWaitForChild(*curr) || NULL == curr->next)
            {
                //cout << "\nWait for child\n";
                waitpid(childpid, &childRval, 0);  // Parent process waits here for child to terminate.
                curr->set_rval(childRval);

                //if(true == piped)
                //    close(pipeid[PipeWriteIndex]);

                //cout << "\nChild exited:(" << childRval <<")-" << strerror(childRval) << "\n";
            }

            if(true == this->isSkipNextCommand(*curr))
            {
                //cout << "skip command : update curr\n";
                curr = curr->next;
            }

            if(NULL != curr)
            {
                prev = curr;
                curr = curr->next;
            }
        }
    }

    return status;
}

#endif // _OSH_EXECUTOR_CPP_
