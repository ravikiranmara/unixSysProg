#ifndef _OSH_EXECUTOR_H_
#define _OSH_EXECUTOR_H_

class Executor
{
  private:
    bool exitShell;

    int patchIO();
    bool isCommandChainValid(Command &command);
    bool isWaitForChild(Command &command);
    bool isExecNextCommand(Command &command);
    bool isCommandExit(Command &command);
    bool isSkipNextCommand(Command &command);
    int fixupStdinOut(Command &command);

    int childExecFunction(Command &command);

  public:
    Executor();
    ~Executor();

    bool get_isExitFromShell();
    bool set_isExitFromShell(bool status);

    int executeCommandList(Command *command);
};

#endif
