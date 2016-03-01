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
        return status_success;
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
    bool endOfArgv = false;
    char **argv;
    int i;

    //cout << "initArgv" << std::endl;

    while (true != endOfArgv)
    {
        // get token
        if(status_success != inputHandler.peekNextToken(token))
        {
            command.set_parseState(Parsed);
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
            endOfArgv = true;
        }
        else
        {
            // add to token
            inputHandler.getNextToken(token);
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
        if(NULL != this->prevCommand)
        {
            if(O_Pipe == this->prevCommand->get_outputMode())
                command.set_parseState(Invalid);
        }
        else
        {
            command.set_parseState(Blank);
            argv = new char*[1];
            argv[0] = NULL;
        }
    }

    // assign to argv
    command.set_argv(argv);

    return status;
}

int Parser::initInputOutput(InputHandler &inputHandler, Command &command)
{
    int status = status_success;
    string token;
    //cout << "init IO" << std::endl;

    // read token
    status = inputHandler.getNextToken(token);

    if(0 == token.compare(redir_stdin))
    {
        if(I_Stdin != command.get_inputMode())
        {
            command.set_parseState(Invalid);
            return status_fail;
        }

        command.set_inputMode(I_File);
        inputHandler.getNextToken(token);
        command.set_runNextCommand(token);
        command.set_inputFilename(token);
    }

    else if(0 == token.compare(redir_stdout))
    {
        if(O_Stdout != command.get_outputMode())
        {
            command.set_parseState(Invalid);
            return status_fail;
        }

        command.set_outputMode(O_FileNew);
        inputHandler.getNextToken(token);
        command.set_runNextCommand(token);
        command.set_outputFilename(token);
    }

    else if(0 == token.compare(append_stdout))
    {
        if(O_Stdout != command.get_outputMode())
        {
            command.set_parseState(Invalid);
            return status_fail;
        }

        command.set_outputMode(O_Append);
        inputHandler.getNextToken(token);
        command.set_outputFilename(token);
        command.set_runNextCommand(token);
        //command.set_parseState(conditionalExec);
    }

    else if(0 == token.compare(redir_pipe))
    {
        if(O_Stdout != command.get_outputMode())
        {
            command.set_parseState(Invalid);
            return status_fail;
        }

        command.set_outputMode(O_Pipe);
        command.set_runNextCommand(token);
        command.set_parseState(Parsed);
    }
    else
    {
        //command.set_parseState(Parsed);
    }

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

int Parser::initConditionalExecSymbol(InputHandler &inputHandler, Command &command)
{
    int status = status_success;
    string token;

    // get the value and append
    inputHandler.getNextToken(token);
    command.set_runNextCommand(token);

    command.set_parseState(Parsed);
    return status;
}

int Parser::getCommandList(InputHandler &inputHandler, Command **command)
{
    int status = status_success;

    string exit = "exit";
    string temp;
    string token, dummy;
    Command *tempCommand = NULL;
    Command *first = NULL;
    Command *curr = NULL;

    this->endOfInput = false;

    int i = 0;
    while (i++ < 50)
    {
        //cout << "iter : " << i << std::endl;
        if(true == inputHandler.isEndOfLine())
        {
            // we have parsed all the line. now exit
            //cout <<"end of line exit\n";
            if(NULL != tempCommand)
            {
                if(Invalid != tempCommand->get_parseState())
                {
                    tempCommand->set_parseState(Parsed);
                }
            }
            break;
        }

        // create our new commnd
        tempCommand = new Command();

        // fix up input output
        status = this->initCommand(*tempCommand);

        // get argv
        if(status_success == status && NeedToken == tempCommand->get_parseState())
        {
            status = initArgv(inputHandler, *tempCommand);
        }

        // blank check
        if(Blank == tempCommand->get_parseState())
        {
            if(NULL != this->prevCommand)
            {
                if(O_Pipe == this->prevCommand->get_outputMode())
                {
                    tempCommand->set_parseState(Invalid);
                    break;
                }
            }

            delete tempCommand;
            continue;
        }

        while ((Parsed != tempCommand->get_parseState()) &&
            (Invalid != tempCommand->get_parseState()) &&
            (Blank != tempCommand->get_parseState()))
        {
            // peek and set parse_state
            status = inputHandler.peekNextToken(token);

            // if redirect, set redirect
            if(status_success == status &&
                ((0 == token.compare(redir_stdout))  ||
                (0 == token.compare(redir_stdin))   ||
                (0 == token.compare(redir_pipe))    ||
                (0 == token.compare(append_stdout))))

            {
                status = this->initInputOutput(inputHandler, *tempCommand);
            }

            else if(status_success == status &&
                ((0 == token.compare(exec_any)) ||
                (0 == token.compare(exec_onfailure)) ||
                (0 == token.compare(exec_onsuccess))))
            {
                this->initConditionalExecSymbol(inputHandler, *tempCommand);
            }

            if(true == this->isEndOfCommand(token) ||
                status_fail == inputHandler.peekNextToken(dummy))
            {
                //cout << "end of command exit\n";

                if(Invalid != tempCommand->get_parseState())
                {
                    tempCommand->set_parseState(Parsed);
                }

                break;
            }
        }

        //cout << "end of command line" << std::endl;
        //we are done, now append and send
        if(status_success == status && Parsed == tempCommand->get_parseState())
        {
            //cout << "init my first\n";
            this->prevCommand = tempCommand;
            if(NULL == first)
            {
                first = tempCommand;
                curr = first;
            }
            else
            {
                curr->next = tempCommand;
                curr = curr->next;
            }
        }

        else if (Blank == tempCommand->get_parseState())
        {

            delete tempCommand;
            //cout << "Blank" << std::endl;
            tempCommand = NULL;
        }

        else if(Invalid == tempCommand->get_parseState())
        {
            delete tempCommand;
            // std::cerr << "Invalid Command" << std::endl;
            status = status_fail;
            tempCommand = NULL;
            first = NULL;
            break;
        }
    }

    *command = first;
/*    if(NULL != command)
        (*command)->DumpCommandChain();
*/
    return status;
}


#endif
