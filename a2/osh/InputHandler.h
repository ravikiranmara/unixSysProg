
/*
 * InputHandler.h
 * used to manage input from user
 */

#ifndef _OSH_INPUTHANDLER_H_
#define _OSH_INPUTHANDLER_H_

#include <string>
#include <vector>

using std::string;
using std::vector;

class InputHandler
{
  private:
    std::string buffer;
    vector<string> tokens;
    string::size_type tokenLength;
    string::size_type tokenIndex;

    int Tokenize();

  public:
    InputHandler();
    ~InputHandler();

    int readInput();
    int getNextToken(string &token);
    int peekNextToken(string &token);
    void clear();


};

#endif
