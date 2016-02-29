#ifndef _OSP_PARSER_H_
#define _OSP_PARSER_H_

#include "globals.h"
#include "InputHandler.cpp"
#include "command.cpp"

class Parser
{
  private:
    TokenState tokenState;
    string prevSymbol;
    Command *prevCommand;
    bool endOfInput;

    Command* newCommand();
    int initCommand(Command &command);
    int initArgv(InputHandler &inputHandler, Command & command);
    int initInputOutput(InputHandler &inputHandler, Command &command);
    bool isEndOfCommand(string symbol);
    bool isTokenValid(string token);
    int setParseState(InputHandler &inputHandler, Command &command, string symbol);
    int initConditionalExecSymbol(InputHandler &inputHandler, Command &command);

    bool isTokenSymbol(string token);

  public:
    Parser();
    ~Parser();

    int getCommandList(InputHandler &inputHandler, Command **command);
};


#endif // _OSP_PARSER_H_
