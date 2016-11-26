/* page table implementation */

#ifndef __OSP_VMM_PAGETABLE_CPP__
#define __OSP_VMM_PAGETABLE_CPP__

#include "common_includes.h"
#include <limits.h>
#include "pageblock.cpp"

using std::vector;

/* page entry. contains list of properties for page */
typedef struct PageEntry
{
    bool isValid;
    bool isModified;
    int accessno;
    int fifono; 
    FrameNumber frameNumber;

    void dump()
    {
        zlog(ZLOG_LOC, "PageEntry::dump - valid (%d), modified (%d), frameNumber (%d)\n",
            isValid, isModified, frameNumber);
    }

    void operator=(PageEntry pe)
    {
        this->isModified = pe.isModified;
        this->isValid = pe.isValid;
        this->frameNumber = pe.frameNumber;
    }

} PageEntry;

/*  a table of page entries. one per logical address page
    a lookup function to get frame number from page number
    a some additional functions to manage the table add, update etc
*/
class PageTable
{
    vector<PageEntry> table;
    FrameNumber frameno;
    PageNumber pageno;
    int accessno;
    int fifono;
    ReplacementStrategy strategy;

  public:
    PageTable()
    {
        this->initialize();
    }

    /* set page table size and initialze pageno for round robin */
    int initialize()
    {
        table.empty();
        table.resize(NumberOfPageInVm);
        pageno = -1;
        frameno = -1;
        this->strategy = Fifo;
        this->invalidateTable();
    }

    int setStrategy(ReplacementStrategy strategy)
    {
        this->strategy = strategy;
    }

    /* invalidate all page entries */
    void invalidateTable()
    {
        this->accessno = 0;
        this->fifono = 0;
        for(int i=0; i<table.size(); i++)
        {
            table[i].isValid = false;
            table[i].frameNumber = i;
            table[i].accessno = 0;
            table[i].fifono = 0;
        }
    }

    void accessPageFifo(PageNumber pageNumber)
    {
        return;
    }

    void accessPageLRU(PageNumber pageNumber)
    {
        table[pageNumber].accessno = accessno;
        accessno++;
        zlog(ZLOG_LOC, "PageEntry::accessPageLru(%d)\n", accessno);
        return;
    }

    /* book keeping when page hit */
    int accessPage(PageNumber pageNumber)
    {
        switch(strategy)
        {
            case Fifo:
                        accessPageFifo(pageNumber);
                        break;

            case LRU:
                        accessPageLRU(pageNumber);
                        break;

            default:
                        zlog(ZLOG_LOC, "PageEntry::findVictim: unknown option\n");
                        break;
        }

        return status_success;
    }

    /* search in the page table to see if we have a valid entry for page number */
    int lookup(PageNumber pageNumber, PageEntry &pageEntry)
    {
        int rval = status_failure;

        /* if page number is valid */
        if(pageNumber < NumberOfPageInVm)
        {
            pageEntry = table[pageNumber];
            this->accessPage(pageNumber);
            rval = status_success;
        }

        /* return page entry */
        return rval;
    }

    int rlookup(FrameNumber frameNumber, PageNumber &pageno, PageEntry &pageEntry) {
        int rval = status_failure;

        for(int i=0; i<table.size(); i++) {
             PageEntry &pe = table[i];
            if(pe.frameNumber == frameNumber && pe.isValid == true) {
                 pageEntry = pe;
                 pageno = i;
                 rval = status_success;
             }
        }

        return rval;
    }

    /* not used, we update instead */
    int addPageEntry(PageEntry pageEntry, PageNumber &pageno)
    {
        int rval = status_success;
        PageNumber victim;

        /* get victim */
        findVictim(victim);

        /* update page  */
        table[victim] = pageEntry;
        pageno = victim;

        return rval;
    }

    /* overwrite page */
    int updatePageEntry(PageEntry pageEntry, PageNumber pageno)
    {
        int rval = status_success;

        /* update page  jjj*/
        table[pageno] = pageEntry;
        table[pageno].accessno = accessno;
        table[pageno].fifono = fifono;
        accessno++; fifono++;

        return rval;
    }

    /* frame replacement policy */
    int findVictim(FrameNumber &victim)
    {
        switch(strategy)
        {
            case Fifo:
                        findVictimByFifo(victim);
                        break;

            case LRU:
                        findVictimByLRU(victim);
                        break;

            default:
                        zlog(ZLOG_LOC, "PageEntry::findVictim: unknown option\n");
                        victim = -1;
                        break;
        }

        return status_success;
    }

    int findVictimByLRU (FrameNumber &victim)
    {
        /* use whatever algo we want, LRU, MRU etc */
        /* currently this is round robin */
        int min = accessno+1;
        int frameno = 0;
        int pageno = -1;

        for(int i=0; i<table.size(); i++) {
            if(true == table[i].isValid) {
                if(table[i].accessno < min) {
                   min = table[i].accessno;
                   frameno = table[i].frameNumber;
                   pageno = i;
                }
            }
        }

        victim = frameno;
        zlog(ZLOG_LOC, "PageEntry::findVictimByLRU : Pageno %d, frameno %d, victim %d, lastaccess %d\n",
                pageno, table[pageno].frameNumber, victim, table[pageno].accessno);

        return status_success;
    }

    int findVictimByFifo(FrameNumber &victim)
    {
                /* use whatever algo we want, LRU, MRU etc */
        /* currently this is round robin */
        int min = fifono+1;
        int frameno = 0;
        int pageno = -1;

        for(int i=0; i<table.size(); i++) {
            if(true == table[i].isValid) {
                if(table[i].fifono < min) {
                   min = table[i].fifono;
                   frameno = table[i].frameNumber;
                   pageno = i;
                }
            }
        }

        victim = frameno;
        zlog(ZLOG_LOC, "PageEntry::findVictimByFifo : Pageno %d, frameno %d, victim %d, lastinsert %d\n",
                pageno, table[pageno].frameNumber, victim, table[pageno].fifono);

        return status_success;
    }

    /* dump physical memory, debugging purpose */
    int dumpPageTable()
    {
        for(int i=0; i<table.size(); i++)
        {
            table[i].dump();
        }
    }
};

#endif // __OSP_VMM_PAGETABLE_CPP__
