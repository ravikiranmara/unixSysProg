
#ifndef _OSH_OSH_CPP_
#define _OSH_OSH_CPP_

#include <string>

#include "globals.h"
#include "osh.h"

void osh::printPrompt()
{
    std::cout << "type 'exit' to exit" << std::endl;
    std::cout<<prompt;
}

int osh::run()
{
    int status = status_success;
    bool stop = false;

    std::string command;
    std::string exit = "exit";

    this->inputHandler.clear();
    std::string token;

    while (stop != true)
    {
        this->printPrompt();
        // std::cin >> command;
        this->inputHandler.readInput();

        //std::cout << command << std::endl;
        while (0 == this->inputHandler.getNextToken(token))
        {
            std::cout << token << std::endl;
            if(0 == token.compare(exit))
            {
                stop = true;
            }
        }
    }

    std::cout << "Exit from shell";
    return status;
}

#endif
