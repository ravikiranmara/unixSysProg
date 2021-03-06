#ifndef _OSP_UTILS_H_
#define _OSP_UTILS_H_

#include <string>
#include<vector>
#include <algorithm>
#include <functional>
#include <cctype>
#include "globals.h"

using std::string;
using std::vector;
using std::iterator;

void TokenizeString(const string& str,
                      vector<string>& tokens,
                      const string& delimiters)
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

static string get_nextCommandString(RunNextCommandSymbol symbol)
{
    string rval;

    switch(symbol)
    {
        case r_redir_stdout:
            rval = redir_stdout;
            break;

        case r_redir_stdin:
            rval = redir_stdin;
            break;

        case r_append_stdout:
            rval = append_stdout;
            break;

        case r_redir_pipe:
            rval = redir_pipe;
            break;

        case r_exec_any:
            rval = exec_any;
            break;

        case r_exec_onsuccess:
            rval = exec_onsuccess;
            break;

        case r_exec_onfailure:
            rval = exec_onfailure;
            break;

        case r_none:
            rval = "";
            break;
    }

    return rval;
}

#endif
