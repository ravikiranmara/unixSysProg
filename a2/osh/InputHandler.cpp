
#ifndef _OSH_INPUTHANDLER_CPP_
#define _OSH_INPUTHANDLER_CPP_

#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#include "globals.h"
#include "utils.h"
#include "InputHandler.h"


InputHandler::InputHandler()
{
    this->clear();
}

InputHandler::~InputHandler()
{
    this->clear();
}

void InputHandler::clear()
{
    this->tokens.clear();
    this->tokenIndex = -1;
    this->tokenLength = 0;

    buffer.clear();
}

int InputHandler::readInput()
{
    int status = status_success;
    string tempbuf;

    std::getline(std::cin, tempbuf);

    // read was successful. update buffer
    this->buffer = tempbuf;

    // tokenize string
    this->Tokenize();

    return status;
}

int InputHandler::Tokenize()
{
    int status = status_success;
    vector<string> temp;
    string tempInput = this->buffer;

    // need to check if token worked fine
    TokenizeString(tempInput, temp, " ");

    this->tokens = temp;
    this->tokenIndex = 0;

    return status;
}

int InputHandler::getNextToken(string& token)
{
    int status = status_success;
    string temp;

    if(tokenIndex < this->tokens.size())
    {
        temp = this->tokens[this->tokenIndex];
        token = trim(temp);
        this->tokenIndex++;
    }
    else
    {
        token = "";
        status = 1;
    }

    return status;
}

int InputHandler::peekNextToken(string& token)
{
    int status = status_success;
    string temp;

    if(tokenIndex < this->tokens.size())
    {
        temp = this->tokens[this->tokenIndex];
        token = trim(temp);
    }
    else
    {
        token = "";
        status = status_fail;
    }

    return status;
}


#endif
