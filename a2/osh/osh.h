
#ifndef _OSH_OSH_H_
#define _OSH_OSH_H_

#include "InputHandler.cpp"
#include "parser.cpp"

class osh
{
  private:
    InputHandler inputHandler;
    Parser parser;

    void printPrompt();

  public:
    int run();
};

#endif
