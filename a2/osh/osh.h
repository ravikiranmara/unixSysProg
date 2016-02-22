
#ifndef _OSH_OSH_H_
#define _OSH_OSH_H_

#include "InputHandler.cpp"

class osh
{
  private:
    InputHandler inputHandler;
    void printPrompt();

  public:
    int run();
};

#endif
