#ifndef __OSP_VMM_PAGEBLOCK_CPP__
#define __OSP_VMM_PAGEBLOCK_CPP__

#include "common_includes.h"
#include "globals.h"

class Page
{
    Byte page[PageSize];

  public:
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
