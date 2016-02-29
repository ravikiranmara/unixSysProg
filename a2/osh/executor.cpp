#ifndef _OSH_EXECUTOR_CPP_
#define _OSH_EXECUTOR_CPP_

#include "executor.h"
#include <sys/wait.h>

Executor::Executor()
{
    this->exitShell = false;
}

Executor::~Executor()
{
}

int Executor::childExecFunction(Command &command)
{
    int rval = 0;
    string token;
    pid_t pid = -1;

    command.get_executable(token);

    // command set parse state
    command.set_parseState(executing);

    // pid
    pid = getpid();
    command.set_pid(pid);

    // fixup command


    // exec child


    // set rval in child
    command.set_rval(rval);

    cout << "Child executing : " << token << " , id - " << command.get_pid() << std::endl;

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
                cout << "\nWait for child\n";
                waitpid(childpid, &childRval, 0);  // Parent process waits here for child to terminate.
                cout << "\nChild exited:(" << childRval <<")\n";
            }

            /*
            //if(true == this->isExecNextCommand(*curr))
            {
                cout << "exec next command\n";
            }
            //else
            {
                cout << "skip command : update curr\n";
            //    curr = curr->next;
            }
            */

            if(NULL != curr)
            {
                curr = curr->next;
            }

            //int blah;
            //cin >> blah;
        }
    }

    return status;
}

#endif // _OSH_EXECUTOR_CPP_
