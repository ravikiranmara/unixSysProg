#ifndef _OSH_PARSER_H_
#define _OSH_PARSER_H_

#include <vector>
#include <string>
#include <unistd.h>
#include "parser.h"

using std::vector;
using std::string;

Parser::Parser()
{
    this->tokenState = any;
    this->prevSymbol = "";
    this->prevCommand = NULL;
}

Parser::~Parser()
{
}

Command* Parser::newCommand()
{
    Command *command = new Command();
    return command;
}

int Parser::initCommand(Command &commmand)
{
    int status = status_success;

    // this is the first command. nothing special to do
    if(NULL == this->prevCommand)
    {
        commmand.set_inputMode(I_Stdin);
        commmand.set_parseState(NeedToken);
        return status;
    }

    // check if we need to init stdin
    if(O_Pipe == this->prevCommand->get_outputMode())
    {
        commmand.set_inputMode(I_Pipe);
    }

    // anything else? check
    commmand.set_parseState(NeedToken);

    return status;
}

int Parser::initArgv(InputHandler &inputHandler, Command &command)
{
    int status = status_success;
    vector<string> tokens;
    string token;
    bool endOfCommand = false;
    char **argv;
    int i;

    while (true != endOfCommand)
    {
        // get token
        if(status_success != inputHandler.getNextToken(token))
        {
            this->endOfInput = true;
            break;
        }

        // is token valid
        if(false == this->isTokenValid(token))
        {
            // log error and return
        }

        // if token != symbol
        if(true == this->isTokenSymbol(token))
        {
            this->prevSymbol = token;
            command.set_runNextCommand(token);
            setParseState(command, token);
            endOfCommand = true;
        }
        else
        {
            // add to token
            tokens.push_back(token);
        }
    }

    // now convert string vector to argv
    argv = NULL;
    if(0 < tokens.size())
    {
        int length = tokens.size();
        argv = new char*[length +1];

        for(i=0; i<length; i++)
        {
            token = tokens[i];

            argv[i] = new char[token.length()+1];
            strncpy(argv[i], token.c_str(), token.length()+1);
        }

        argv[tokens.size()] = NULL;
    }
    else
    {
        // this is a blank command
        command.set_parseState(Invalid);
        argv = new char*[1];
        argv[0] = NULL;
    }

    // assign to argv
    command.set_argv(argv);

    return status;
}

int Parser::setParseState(Command &command, string symbol)
{
    if(0 == symbol.compare(redir_stdin))
    {
        command.set_parseState(InPath);
    }

    else if(0 == symbol.compare(redir_stdout) || 0 == symbol.compare(append_stdout))
    {
        command.set_parseState(OutPath);
    }

    else
    {
        command.set_parseState(Parsed);
    }

    return status_success;
}

int Parser::initInputOutput(InputHandler &inputHandler, Command &command)
{
    int status = status_success;
    string token;

    // check if the last symbol was redirect
    if(command.get_runNextCommand() == r_redir_stdin)
    {
        command.set_inputMode(I_File);
        inputHandler.getNextToken(token);
        command.set_inputFilename(token);
    }

    else if(command.get_runNextCommand() == r_redir_stdout)
    {
        command.set_outputMode(O_FileNew);
        inputHandler.getNextToken(token);
        command.set_outputFilename(token);
    }

    else if(command.get_runNextCommand() == r_append_stdout)
    {
        command.set_outputMode(O_Append);
        inputHandler.getNextToken(token);
        command.set_outputFilename(token);
    }

    else if(command.get_runNextCommand() == r_redir_pipe)
    {
        command.set_outputMode(O_Pipe);
    }

    command.set_parseState(Parsed);
    return status;
}

bool Parser::isEndOfCommand(string token)
{
    if((0 == (token.compare(redir_pipe))) ||
        (0 == (token.compare(exec_any))) ||
        (0 == (token.compare(exec_onsuccess))) ||
        (0 == (token.compare(exec_onfailure)))
    )
    {
        return true; // for now
    }

    return false;
}

bool Parser::isTokenSymbol(string token)
{
    if((0 == (token.compare(redir_pipe))) ||
        (0 == (token.compare(redir_stdin))) ||
        (0 == (token.compare(redir_stdout))) ||
        (0 == (token.compare(append_stdout))) ||
        (0 == (token.compare(exec_any))) ||
        (0 == (token.compare(exec_onsuccess))) ||
        (0 == (token.compare(exec_onfailure)))
    )
    {
        return true;
    }

    return false;
}

bool Parser::isTokenValid(string token)
{
    bool status = true;



    return status;
}

int Parser::getCommandList(InputHandler &inputHandler, Command &command)
{
    int status = status_success;

    string exit = "exit";
    string temp;
    Command *tempCommand = NULL;

    this->endOfInput = false;

    while (true)
    {
        // create our new commnd
        tempCommand = new Command();

        // fix up input output
        status = this->initCommand(*tempCommand);

        // get argv
        if(status_success == status && NeedToken == tempCommand->get_parseState())
        {
            status = initArgv(inputHandler, *tempCommand);
        }

        // if we hit an empty command, then exita
        if(true == this->endOfInput)
        {
            delete tempCommand;
            break;
        }

        // if redirect, set redirect
        if(status_success == status &&
            (InPath == tempCommand->get_parseState() || OutPath == tempCommand->get_parseState()))
        {
            status = this->initInputOutput(inputHandler, *tempCommand);
        }

        //we are done, now append and send
        if(status_success == status && Parsed == tempCommand->get_parseState())
        {
            this->prevCommand = tempCommand;
            command = *tempCommand;
        }

        tempCommand->DumpCommand();
    }


    return status;
}

#endif
