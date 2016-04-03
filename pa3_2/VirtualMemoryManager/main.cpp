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

/* functions */

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

int run(string inputfile)
{
    int rval = status_success;
    VirtualMemoryManager vmm(BackingStoreFilename);
    VirtualAddress virtualAddress;
    PhysicalAddress physicalAddress;
    Byte byte;
    string line;
    int address;
    char ch;
    char buffer[1024];
    int totallookups, pagefaults, pagehits, tlbhits;

    /* open input file */
    ifstream input(inputfile.c_str());
    ofstream output(OutputFilename.c_str());
    if(false == input.is_open())
    {
        rval = errno;
        zlog(ZLOG_LOC, "unable to open input file : %s - %s", inputfile.c_str(), strerror(rval));
        goto exit1;
    }

    /* open output file */
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
        vmm.readByte(virtualAddress, physicalAddress, byte);
        /* zlog(ZLOG_LOC, "Global::test_vmm - *********  virtual address : %d, Physical address : %d, data : %d \n",
                        virtualAddress, physicalAddress, (signed)byte); */
        ch = byte;

        /*cout << "Virtual Address : " << virtualAddress << ", Physical Address : " << physicalAddress
                << " , Value : " << int(ch) << endl;*/

        sprintf (buffer, "Virtual address: %d Physical address: %d Value: %d\n",
               virtualAddress, physicalAddress, ch);
        output << buffer;
    }

    /* lets print stats before exiting */
    vmm.getStats(totallookups, pagefaults, pagehits, tlbhits);
    sprintf(buffer, "Number of Translated Addresses = %d\nPage Faults = %d\nPage Fault Rate = %.3f\nTLB Hits = %d\nTLB Hit Rate = %.3f\n",
                       totallookups, pagefaults, (float)pagefaults/totallookups, tlbhits, (float)tlbhits/totallookups);
    output << buffer;

    output.close();
exit2:
    input.close();
exit1:
    return status_success;
}

/* main */
int main(int argc, char* argv[])
{
    int rval = status_success;
    bool status = status_success;
    vector<string> args;

    try
    {
        // initialize logging
        // init_zlog();
        zlog_init("vmm_trace.log");

        // get args
        zlog(ZLOG_LOC, "Global::Main - parse args\n");
        args = parseArgs(argc, argv);
        dumpArgs(args);

        /* if args count is not sufficient throw error and exit */
        if(argc < 2)
        {
            zlog(ZLOG_LOC, "Global::Main - Not enough parameters\n");
            zlog(ZLOG_LOC, "Global::Main - %s <input file name>\n", argv[0]);
            goto exit1;
        }

        /* send it to vmm */


        /* test backing store */
        // test_backing(args.at(1));
        // test_pagetable();
        run(args[1]);

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
