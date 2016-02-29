
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
    std::string exit = "exit";

    this->inputHandler.clear();
    std::string token;

    while (stop != true)
    {
        this->printPrompt();

        this->inputHandler.clear();
        this->inputHandler.readInput();

        this->parser.getCommandList(inputHandler, &command);

        this->executor.executeCommandList(command);

        if(true == this->executor.get_isExitFromShell())
        {
            break;
        }
    }

    std::cout << "Exit from shell";
    return status;
}

#endif
