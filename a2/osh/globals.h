#ifndef _OSP_GLOBAL_H_
#define _OSP_GLOBAL_H_

#include <string>
#include <iostream>
#include "logger.cpp"

using std::cout;
using std::cin;
using std::string;

Logger logger;


const int status_success = 0;
const int status_fail = 1;
const string prompt = "osh>";
const string exitCommand = "exit";
const int Invalid_Pid = 1;
const int Invalid_Fid = 1;
const int PipeReadIndex = 0;
const int PipeWriteIndex = 1;

enum InputMode
{
    I_Stdin,
    I_File,
    I_Pipe
};

enum OutputMode
{
    O_Stdout,
    O_FileNew,
    O_Append,
    O_Pipe
};

enum RunNextCommandSymbol
{
    r_redir_stdout,
    r_redir_stdin,
    r_append_stdout,
    r_redir_pipe,
    r_exec_any,
    r_exec_onsuccess,
    r_exec_onfailure,
    r_none
};

enum ParseState
{
    Blank,
    Invalid,
    NeedToken,
    NewCommand,
    InPath,
    OutPath,
    Pipesym,
    Parsed,
    executing,
    conditionalExec
};

enum TokenState
{
    any,
    filename,
    commnad
};

const string redir_stdout = ">";
const string redir_stdin = "<";
const string append_stdout = ">>";
const string redir_pipe = "|";
const string exec_any = ";";
const string exec_onsuccess = "&&";
const string exec_onfailure = "||";

#endif

