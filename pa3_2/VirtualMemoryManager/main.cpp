/*
* OSP programming assignment 3
* Virtual Memory Manager
*/

#include "common_includes.h"
#include <pthread.h>

#include "globals.h"
#include "zlog.h"
#include "backingstore.cpp"
#include "physicalmemory.cpp"
#include "pagetable.cpp"

/* using std */
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::strerror;
using std::string;
using std::vector;

/* extern */


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

/* test module */

int test_backing(string filename)
{
    int rval = status_success;
    BackingStore bs (filename.c_str()); //  = new BackingStore(filename.c_str());
    Page buffer;
    PhysicalMemory physicalMemory;
    PhysicalAddress address;
    Byte byte;

    zlog(ZLOG_LOC, "Global::test backing start\n");
    /* check if page is open */
    if(true == bs.isFileOpen())
    {
        zlog(ZLOG_LOC, "Global::test_backing - file is open\n");
    }

    /* get a chunk of memory  */
    if(status_success != (rval = bs.getPage(buffer, 0, PageSize)))
    {
        zlog(ZLOG_LOC, "Global::test_backing Error : unable to get page - %d\n", rval);
        zlog(ZLOG_LOC, strerror(rval));
        goto exit1;
    }

    /* print values */
    buffer.dumpPage();

    /* write page to physical memory */
    int frameno;
    if(status_success != physicalMemory.addPage(buffer, frameno))
    {
        zlog(ZLOG_LOC, "Global::test_backing - failed to add page to physical memory\n");
        goto exit1;
    }

    /* dump to see if physical memory is same */
    zlog(ZLOG_LOC, "Global::test_backing - dumping physical memory page load\n");
    physicalMemory.dumpMemory(frameno);

    /* see if we can get and set byte on physical memory */
    address = (frameno << numBitsForFrameOffset);
    for(int i=0; i<20; i++)
    {
        zlog(ZLOG_LOC, "Global::test_backing - get byte at : %d\n", address);

        /* get */
        physicalMemory.readByte(address, byte);
        zlog(ZLOG_LOC, "Global::test_backing - got byte - %d\n", byte);
        address++;
    }

    /* set byte */
    byte = 10;
    physicalMemory.writeByte(address, byte);

    /* get again */
    physicalMemory.readByte(address, byte);
    zlog(ZLOG_LOC, "Global::test_backing - new got byte - %d\n", byte);

exit1:
    /* close file */
    bs.closeFile();
    zlog(ZLOG_LOC, "Global::test_backing - exit : %d\n", rval);
    return rval;

}

void test_pagetable()
{
    PageTable pageTable;
    PageEntry pageEntry;
    PageNumber pageno;

    /* add a bunch of pages */
    for (int i=0; i<100; i++)
    {
        /* page entry edit */
        pageEntry.frameNumber = i;
        pageEntry.isValid = true;
        pageEntry.isModified = false;

        /* add page entry */
        pageTable.addPageEntry(pageEntry, pageno);
    }

    /* dump page table */
    // pageTable.dumpPageTable();

    /* lets lookup instead */
    for(int i=0; i<100; i++)
    {
        pageTable.lookup(i, pageEntry);
        pageEntry.dump();
    }

    return;
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
        init_zlog();

        // get args
        zlog(ZLOG_LOC, "Global::Main - parse args\n");
        args = parseArgs(argc, argv);
        dumpArgs(args);

        /* if args count is not sufficient throw error and exit */
        if(argc < 2)
        {
            zlog(ZLOG_LOC, "Global::Main - Not enough parameters\n");
            zlog(ZLOG_LOC, "Global::Main - %s <backingFilename>\n", argv[0]);
            goto exit1;
        }

        /* send it to vmm */



        /* test backing store */
        // test_backing(args.at(1));
        test_pagetable();

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
