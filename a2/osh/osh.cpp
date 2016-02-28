
#ifndef _OSH_OSH_CPP_
#define _OSH_OSH_CPP_

#include <string>

#include "globals.h"
#include "osh.h"
#include "command.cpp"
#include "parser.cpp"
#include "executor.cpp"

void osh::printPrompt()
{
    std::cout << "type 'exit' to exit" << std::endl;
    std::cout << prompt;
}

int osh::run()
{
    int status = status_success;
    bool stop = false;

    Command command;
    std::string exit = "exit";

    this->inputHandler.clear();
    std::string token;

    while (stop != true)
    {
        this->printPrompt();

        this->inputHandler.clear();
        this->inputHandler.readInput();

        this->parser.getCommandList(inputHandler, command);

        command.DumpCommandChain();

        //this->executor.executeCommandList(command);
    }

    std::cout << "Exit from shell";
    return status;
}

#endif
