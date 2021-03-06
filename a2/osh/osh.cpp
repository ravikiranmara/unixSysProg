
#ifndef _OSH_OSH_CPP_
#define _OSH_OSH_CPP_

#include <string>
#include <unistd.h>

#include "globals.h"
#include "osh.h"
#include "command.cpp"
#include "parser.cpp"
#include "executor.cpp"

void osh::printPrompt()
{
//    std::cout << "type 'exit' to exit" << std::endl;
    //std::cout << "(" << getpid() << ") ";
    std::cout << prompt;
}

int osh::run()
{
    int status = status_success;
    bool stop = false;

    Command *command = NULL;

    this->inputHandler.clear();
    std::string token;

    int j = 0;
    while (true != stop && j++ < 23)
    {
        this->printPrompt();

        this->inputHandler.clear();
        this->inputHandler.readInput();

        status = this->parser.getCommandList(inputHandler, &command);
        logger.log(info, "Got command list ");
        if(logger.isLogMessage())
        {
            command->DumpCommandChain();
        }

        if(status_success == status)
            this->executor.executeCommandList(command);
        else
        {
            cerr << "Ambiguous command format\n";
        }

        if(true == this->executor.get_isExitFromShell())
        {
            stop = true;
        }
    }

    //std::cout << "Exit from shell";
    return status;
}

#endif
