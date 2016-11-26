/*
* OSP programming assignment 3
* Virtual Memory Manager
*/

#include "common_includes.h"
#include <pthread.h>
#include <cmath>
#include <fstream>

#include "zlog.h"
#include "globals.h"
#include "backingstore.cpp"
#include "physicalmemory.cpp"
#include "pagetable.cpp"
#include "vmm.cpp"

/* using std */
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::strerror;
using std::string;
using std::vector;

using std::ifstream;
using std::ofstream;

/* functions prototypes */
void finalize_zlog();

/* reads argv, converts it to a vector of strings */
vector<string> parseArgs(int argc, char *argv[])
{
    vector<string> args;
    string temp;

    /* foreach args, convert to string and add to args */
    for(int i=0; i<argc; i++)
    {
        temp.clear();
        temp = string(argv[i]);
        args.push_back(temp);
    }

    return args;
}

/* dump args, debugging purpose */
void dumpArgs(vector<string> args)
{
    zlog(ZLOG_LOC, "Global::dumpArgs - dump args : %d\n", args.size());
    for(int i=0; i<args.size(); ++i)
    {
       zlog(ZLOG_LOC, "%s ", args.at(i).c_str());
    }

    zlog(ZLOG_LOC, "\n");
    return;
}


/* main logic. read from input file, use virtual memory manager to translate
the address, and read the byte */
int run(vector<string> args)
{
    int rval = status_success;

    ReplacementStrategy strategy;
    if(0 == args[3].compare("fifo")) { strategy = Fifo; }
else if (0 == args[3].compare("lru")) { strategy = LRU; }
    else { cout << "Invalid Input for strategy. Choose between {fifo or lru}"; }
    
    VirtualMemoryManager vmm(args[1].c_str(), strategy);     /* does memory management */
    VirtualAddress virtualAddress;                      /* input from file, address to get */
    PhysicalAddress physicalAddress;                    /* output from vmm. the location of the byte in physical memory */
    Byte byte;                                          /* contains a single byte of data */
    string line;                                        /* buffer to read from file */
    int address;
    char ch;                                            /* hack, we need to print in signed char, cout << is not printing in the rquired format */
    char buffer[1024];                                  /* so we use sprintf to print to buffer and write to file. I'm sorry */
    int totallookups, pagefaults, pagehits, tlbhits;    /* stats of vmm, which we will print later */


    /* open input file */
    zlog(ZLOG_LOC, "Global::run - open input and output files \n");
    ifstream input(args[2].c_str());                    /* input file of addresss */
    ofstream output(OutputFilename.c_str());

    /* check if files opened properly */
    zlog(ZLOG_LOC, "Global::run - check if input file is open\n");
    if(false == input.is_open())
    {
        rval = errno;
        zlog(ZLOG_LOC, "unable to open input file : %s - %s", args[1].c_str(), strerror(rval));
        goto exit1;
    }

    zlog(ZLOG_LOC, "Global::run - check if output file is open\n");
    if(false == output.is_open())
    {
        rval = errno;
        zlog(ZLOG_LOC, "unable to output input file : %s - %s", OutputFilename.c_str(), strerror(rval));
        goto exit2;
    }

    /* while input */
    while(getline(input, line))
    {
        zlog(ZLOG_LOC, "================================================\n");
        address = atoi(line.c_str());
        virtualAddress = (VirtualAddress)address;

        /* vmm call to translate address */
        zlog(ZLOG_LOC, "Global::run - get byte for address : %d\n", virtualAddress);
        vmm.readByte(virtualAddress, physicalAddress, byte);
        ch = byte;

        /* now print to output and log file */
        sprintf (buffer, "Virtual address: %d Physical address: %d Value: %d\n",
               virtualAddress, physicalAddress, ch);
        output << buffer;
        zlog(ZLOG_LOC, buffer);
    }

    /* lets print stats before exiting */
    vmm.getStats(totallookups, pagefaults, pagehits, tlbhits);
    sprintf(buffer, "Number of Translated Addresses = %d\nPage Faults = %d\nPage Fault Rate = %.3f\nTLB Hits = %d\nTLB Hit Rate = %.3f\n",
                       totallookups, pagefaults, (float)pagefaults/totallookups, tlbhits, (float)tlbhits/totallookups);
    output << buffer;
    zlog(ZLOG_LOC, buffer);

    output.close();
exit2:
    input.close();
exit1:
    return rval;
}

/* main */
int main(int argc, char* argv[])
{
    int rval = status_success;
    bool status = status_success;
    vector<string> args;

    try
    {
        /* initialize logging */
        // init_zlog();     /* log to stdout */
        zlog_init("vmm_trace.log");     /* log to file */

        /*  get args */
        zlog(ZLOG_LOC, "Global::Main - parse args\n");
        args = parseArgs(argc, argv);
        dumpArgs(args);

        /* if args count is not sufficient throw error and exit */
        if(argc < 4)
        {
            zlog(ZLOG_LOC, "Global::Main - Not enough parameters\n");
            zlog(ZLOG_LOC, "Global::Main - %s <Backing Store> <input file name> <strategy>\n", argv[0]);
            cout << "Error : Not enough parameters\n";
            cout << "Syntax  - " << argv[0] << " <Backing Store> <input file name> <strategy>" << std::endl;
            cout << "Strategy can be fifo or lru" << std::endl;
            goto exit1;
        }

        /* run main logic */
        rval = run(args);

        /* update return value */
        rval = status_success;
    }
    catch(const std::runtime_error& re)
    {
        zlog(ZLOG_LOC, "Global::Main - Runtime error: %s\n", re.what());
        rval = status_failure;
    }
    catch(const std::exception& ex)
    {
        zlog(ZLOG_LOC, "Global::Main - Exception error: %s\n", ex.what());
        rval = status_failure;
    }
    catch(...)
    {
        zlog(ZLOG_LOC, "Global::Main - Unknown Exception occured \n");
        rval = status_failure;
    }

exit1:
    zlog(ZLOG_LOC, "Global::Main - End of program\n");
    finalize_zlog();

    return rval;
}

/**** logging related - ignore ****/
/* we'll refine this a bit more later */
int init_zlog()
{
    /* initialize to stdout */
    zlog_init_stdout();

    /* initialize to file */
    //zlog_init("vmm.log");

    /* the output is stored in buffer and flushed
    periodically. we initialize the thread . zlog-config.h
    contains the timing value */
    zlog_init_flush_thread();

    // sample log
    zlog(ZLOG_LOC, "Global::init_zlog - initialized log params\n");

    return 0;
}

void finalize_zlog()
{
    zlog_finish();
}
