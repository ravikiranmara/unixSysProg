
#ifndef _OSH_OSH_H_
#define _OSH_OSH_H_

#include "InputHandler.cpp"
#include "parser.cpp"
#include "executor.cpp"

class osh
{
  private:
    InputHandler inputHandler;
    Parser parser;
    Executor executor;

    void printPrompt();

  public:
    int run();
};

#endif
