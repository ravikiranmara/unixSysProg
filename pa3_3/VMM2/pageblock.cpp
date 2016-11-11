/*
    just a chunk of memory, and some fancy ways to get/manage it
*/

#ifndef __OSP_VMM_PAGEBLOCK_CPP__
#define __OSP_VMM_PAGEBLOCK_CPP__

#include "common_includes.h"
#include "globals.h"

class Page
{
    Byte page[PageSize];

  public:
    /* read data at offset */
    int getByte(const int offset, Byte &byte)
    {
        int rval = status_failure;

        if(offset < PageSize)
        {
            byte = this->page[offset];
            rval = status_success;
        }

        return rval;
    }

    /* write data to offset */
    int putByte(const int offset, Byte byte)
    {
        int rval = status_failure;

        if(offset < PageSize)
        {
            this->page[offset] = byte;
            rval = status_success;
        }

        return rval;
    }

    /* make it easier to mange page */
    int operator=(Page inputPage)
    {
        for(int i=0; i<PageSize; i++)
        {
            page[i] = inputPage.page[i];
        }

        return status_success;
    }

    int operator=(char* buffer)
    {
        for(int i=0; i<PageSize; i++)
        {
            page[i] = (Byte)buffer[i];
        }

        return status_success;
    }

    /* dump for debug purpose */
    void dumpPage()
    {
        for(int i=0; i<PageSize; i++)
        {
            zlog(ZLOG_LOC, "%d ", page[i]);
        }

        return;
    }
};

#endif
