#ifndef _OSH_EXECUTOR_CPP_
#define _OSH_EXECUTOR_CPP_

#include "executor.h"

Executor::Executor()
{
}

Executor::~Executor()
{
}

void Executor::execChild(Command &command)
{
    int rval = 0;

    // command set parse state, pid

    // fixup command

    // exec child

    // set rval in child
    command.set_rval(rval);

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

int Executor::executeCommandList(Command &command)
{
    int status = status_success;
    Command *curr;

    // validate command
    if(false == this->isCommandChainValid(command))
    {
        // a better error message
        return status_fail;
    }

    curr = &command;
    while(NULL != curr)
    {
        // is command exit

        // fork child

        // wait for next command

        // wait for next commnad

        // if(fail)skip next command
        cout << "exec command " << std::endl;

        curr->DumpCommand();
        curr = curr->next;
    }

    return status;
}

#endif // _OSH_EXECUTOR_CPP_
