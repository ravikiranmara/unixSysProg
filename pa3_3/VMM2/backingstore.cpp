/*
* manage backing store. (currently just read from store)
*
*/

#ifndef __OSP_VMM_BACKINGSTORE_H__
#define __OSP_VMM_BACKINGSTORE_H__

#include "common_includes.h"
#include "pageblock.cpp"

using std::strerror;
using std::string;
using std::cout;

class BackingStore
{
    int file;
    bool isopen;

  public:
    BackingStore(const char *filename)
    {
        file = 0;
        openFile(filename);
        return;
    }

    ~BackingStore()
    {
        closeFile();
    }

    bool isFileOpen()
    {
        return isopen;
    }

    /* normal open, nothing special */
    int openFile(const char *filename)
    {
        int rval = status_success;

        if(-1 == (file = open(filename, O_RDONLY)))
        {
            errno = rval;
            zlog(ZLOG_LOC, "BackingStore::openFile - Unable to open file (%s) : %s\n", filename, strerror(errno));
            goto exit1;
        }

        /* open call succeeded */
        isopen = true;
        zlog(ZLOG_LOC, "BackingStore::openFile - open file succeeded : %s\n", filename);

      exit1:
        return rval;
    }

    /* just normal close, nothing special */
    int closeFile()
    {
        int rval = status_success;

        if(true == isopen)
        {
            if(-1 == close(file))
            {
                errno = rval;
                zlog(ZLOG_LOC, "BackingStore::closeFile - Unable to close file : %s\n", strerror(errno));
                goto exit1;
            }
        }

        isopen = false;
        zlog(ZLOG_LOC, "BackingStore::closeFile - succeeded\n");

      exit1:
        return rval;
    }

    /* seeks to the position of file, and grabs a pagesize of data
        assumes that buffer is already allocated with pagesize bytes */
    int getPage(Page &page, int position, int pagesize)
    {
        int rval = status_success;
        int status = status_success;
        char tbuf[pagesize];

        // seek to file
        zlog(ZLOG_LOC, "BackingStore::getPage - seeking file : %d\n", position);
        if(-1 == lseek(file, position, SEEK_SET))
        {
            rval = errno;
            status = status_failure;
            zlog(ZLOG_LOC, "BackingStore::getPage - Unable to seek to position (%d) : %s\n", position, strerror(errno));
        }

        // read file
        zlog(ZLOG_LOC, "BackingStore::getPage - attempting to read\n");
        if(status_success == status)
        {
            if(-1 == read(file, &tbuf, pagesize))
            {
                errno = rval;
                status = status_failure;
                zlog(ZLOG_LOC, "BackingStore::getPage - Unable to read from file : %s\n", strerror(errno));
            }

            zlog(ZLOG_LOC, "Copy buffer to page\n");
            page = tbuf;
        }

        zlog(ZLOG_LOC, "BackingStore::getPage - exit with status (%d)\n", rval);
        return rval;
    }
};


#endif
