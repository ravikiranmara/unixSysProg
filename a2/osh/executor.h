#ifndef _OSH_EXECUTOR_H_
#define _OSH_EXECUTOR_H_

class Executor
{
  private:
    int patchIO();
    bool isCommandChainValid(Command &command);
    void execChild(Command &command);
    bool isWaitForChild(Command &command);
    bool isExecNextCommand(Command &command);
    bool isCommandExit(Command &command);
    bool isSkipNextCommand(Command &command);

  public:
    Executor();
    ~Executor();

    int executeCommandList(Command &command);
};

#endif
