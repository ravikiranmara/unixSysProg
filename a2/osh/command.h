#ifndef _OSH_COMMAND_H_
#define _OSH_COMMAND_H_

#include <string>
#include <sys/types.h>

using std::string;

class Command
{
  private:
    ParseState parseState;
    string binToExecute;
    char **argv;

    InputMode inputMode;
    int inputFid;
    string inputFilename;

    OutputMode outputMode;
    int outputFid;
    string outputFilename;

    RunNextCommandSymbol runNextCommand;

    pid_t pid;
    int rval;

  public:
    Command *next;

    Command()
    {
        this->parseState = NewCommand;
        this->next = NULL;
        this->argv = NULL;
        this->pid = Invalid_Pid;
        this->inputFid = Invalid_Fid;
        this->outputFid = Invalid_Fid;
        this->inputFilename = "";
        this->outputFilename = "";
        this->inputMode = I_Stdin;
        this->outputMode = O_Stdout;
        this->rval = -1;
        this->runNextCommand = r_none;
        this->next = NULL;

        return;
    }

    ~Command()
    {
    }

    int get_rval()
    {
        return this->rval;
    }

    int set_rval(int val)
    {
        this->rval = val;
        return val;
    }

    ParseState get_parseState()
    {
        return this->parseState;
    }

    ParseState set_parseState(ParseState state)
    {
        this->parseState = state;
        return state;
    }

    int get_pid()
    {
        return this->pid;
    }

    int set_pid(int pid)
    {
        this->pid = pid;
        return this->pid;
    }

    string get_binToExecute()
    {
        return this->binToExecute;
    }

    string set_binToExecute(string binname)
    {
        this->binToExecute = binname;
        return this->binToExecute;
    }

    int get_inputFid()
    {
        return this->inputFid;
    }

    int set_inputFid(int fid)
    {
        this->inputFid = fid;
        return this->inputFid;
    }

    string get_inputFilename()
    {
        return this->inputFilename;
    }

    string set_inputFilename(string filename)
    {
        this->inputFilename = filename;
        return this->inputFilename;
    }

    InputMode get_inputMode()
    {
        return this->inputMode;
    }

    InputMode set_inputMode(InputMode mode)
    {
        this->inputMode = mode;
        return this->inputMode;
    }

    int get_outputFid()
    {
        return this->outputFid;
    }

    int set_outputFid(int fid)
    {
        this->outputFid = fid;
        return this->inputFid;
    }

    string get_outputFilename()
    {
        return this->outputFilename;
    }

    string set_outputFilename(string filename)
    {
        this->outputFilename = filename;
        return this->outputFilename;
    }

    OutputMode get_outputMode()
    {
        return this->outputMode;
    }

    OutputMode set_outputMode(OutputMode mode)
    {
        this->outputMode = mode;
        return this->outputMode;
    }

    RunNextCommandSymbol get_runNextCommand()
    {
        return this->runNextCommand;
    }

    RunNextCommandSymbol set_runNextCommand(RunNextCommandSymbol run)
    {
        this->runNextCommand = run;
        return this->runNextCommand;
    }

    RunNextCommandSymbol set_runNextCommand(string symbol)
    {
        if(0 == symbol.compare(redir_stdout))
        {
            this->runNextCommand = r_redir_stdout;
        }
        else if(0 == symbol.compare(redir_stdin))
        {
            this->runNextCommand = r_redir_stdin;
        }
        else if(0 == symbol.compare(redir_pipe))
        {
            this->runNextCommand = r_redir_pipe;
        }
        else if(0 == symbol.compare(append_stdout))
        {
            this->runNextCommand = r_append_stdout;
        }
        else if(0 == symbol.compare(exec_any))
        {
            this->runNextCommand = r_exec_any;
        }
        else if(0 == symbol.compare(exec_onfailure))
        {
            this->runNextCommand = r_exec_onfailure;
        }
        else if(0 == symbol.compare(exec_onsuccess))
        {
            this->runNextCommand = r_exec_onsuccess;
        }

        return this->runNextCommand;
    }

    int addToList(Command &command)
    {
        Command *temp = this->next;
        this->next = &command;
        command.next = temp;

        return status_success;
    }

    int freeCommandChain()
    {
        Command *temp = NULL, *nextCmd = NULL;

        nextCmd = this->next;
        while(nextCmd != NULL)
        {
            temp = nextCmd->next;
            delete nextCmd;
            next = temp;
        }

        this->next = NULL;
        return status_success;
    }

    int set_argv(char** arg)
    {
        int status = status_success;

        this->argv = arg;

        return status;
    }

    char** get_argv()
    {
        return argv;
    }

    int get_executable(string &executable)
    {
        string empty("");
        executable = empty;

        if(NULL != argv[0])
        {
            string temp(argv[0]);
            executable = temp;
        }

        return status_success;
    }

    void DumpCommandMini()
    {
        cout << " ----------Command Details ----------> " << std::endl;
        cout << "Command To run : " << this->binToExecute << std::endl;
        cout << "Arguments argv : ";
        int i = 0;
        while (NULL != argv[i])
        {
            cout << argv[i] << " ";
            i++;
        }

        cout << std::endl;

        InputMode inm = this->get_inputMode();
        string ipms = (I_Pipe == inm)? "PIPE":(I_File == inm)? "FILE":"STDIN";
        cout << "Input mode :" << ipms << std::endl;

        string inf;
        if(0 == ipms.compare("FILE"))
        {
            inf = this->get_inputFilename();
        }
        cout << "Input filename : " << inf << std::endl;

        string opf;
        OutputMode opm = this->get_outputMode();
        string opms;
        switch (opm)
        {
            case O_Pipe:
                opms = "PIPE";
                break;
            case O_FileNew:
                opms = "REDIR";
                break;

            case O_Append:
                opms = "REDIR APPEND";
                break;

            case O_Stdout:
                opms = "STDOUT";
                break;
        }

        cout << "Output Mode : " << opms << std::endl;

        if(opm == O_FileNew || opm == O_Append)
        {
            opf = this->get_outputFilename();
        }

        cout << "Output Filename : " << opf << std::endl;
        cout << "Connector symbol : " << get_nextCommandString(this->get_runNextCommand());

        cout << "\n";

        return;
    }

    void DumpCommand()
    {
        cout << "-------------   Dump Command  ------------" << std::endl;
        cout <<"Parse state:" << this->get_parseState() << std::endl;
        cout << "Input File Mode:" << this->get_inputMode() << std::endl;
        cout << "Input File Id:" << this->get_inputFid() << std::endl;
        cout << "Input Filename:" << this->get_inputFilename() << std::endl;
        cout << "Output File Mode:" << this->get_outputMode() << std::endl;
        cout << "Output File Id:" << this->get_outputFid() << std::endl;
        cout << "Output Filename:" << this->get_outputFilename() << std::endl;
        cout << "Run Next command:" << this->get_runNextCommand() << std::endl;

        cout <<"--- argv ---" << std::endl;
        int i = 0;

        while (NULL != argv[i])
        {
            cout << argv[i] << " ";
            i++;
        }

        cout << std::endl << "----------------------------------------------------" << std::endl;
        return;
    }

    void DumpCommandChain()
    {
        Command *curr = this;

        cout << "======================  DUMP COMMAND LINKED LIST  ================================"<< std::endl;
        while (NULL != curr)
        {
            curr->DumpCommandMini();
            curr = curr->next;
        }
        cout << "==============  END OF DUMP CHAIN ==============="<< std::endl;

    }
};

#endif
