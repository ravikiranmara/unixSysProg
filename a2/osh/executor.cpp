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
        cout << "udate input stream" << std::endl;
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
        cout << "udate output stream" << std::endl;
        token = command.get_outputFilename();
        length = token.length();
        int openmode;
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

        filename = new char[length+1];
        strncpy(filename, token.c_str(), length+1);

        if(O_FileNew == command.get_outputMode())
        {
            openmode = O_WRONLY | O_CREAT;
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
       cout << "Error when execlp:(" << errno << ")- " << strerror(errno) << std::endl;
    }

    // set rval in child
    command.set_rval(rval);

    cout << "Child executing : " << token << " , id - " << command.get_pid() << std::endl;
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
        (r_exec_onsuccess == run && 1 == prevRval))
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

int Executor::executeCommandList(Command *command)
{
    int status = status_success;
    Command *curr;
    string bin;
    pid_t childpid = 0;
    int childRval = 0;

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

    curr = command;
    while(NULL != curr)
    {
        // is command exit
        if(isCommandExit(*curr))
        {
            this->set_isExitFromShell(true);
            break;
        }

        // fork child
        if(-1 == (childpid = fork()))
        {
            // fork failed
            exit(1);
        }

        if(0 == childpid)
        {
            this->childExecFunction(*curr);
        }
        else
        {
            curr->get_executable(bin);
            cout << bin << " - ";

            // wait for next command
            if(this->isWaitForChild(*curr) || NULL == curr->next)
            {
                //cout << "\nWait for child\n";
                waitpid(childpid, &childRval, 0);  // Parent process waits here for child to terminate.
                //cout << "\nChild exited:(" << childRval <<")\n";
            }

            if(true != this->isExecNextCommand(*curr))
            {
                //cout << "skip command : update curr\n";
                curr = curr->next;
            }

            if(NULL != curr)
            {
                curr = curr->next;
            }
        }
    }

    return status;
}

#endif // _OSH_EXECUTOR_CPP_
