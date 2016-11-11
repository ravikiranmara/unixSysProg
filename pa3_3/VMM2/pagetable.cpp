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
    PageNumber pageno;
    int accessno;
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
        this->strategy = RoundRobin;
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
        for(int i=0; i<table.size(); i++)
        {
            table[i].isValid = false;
            table[i].frameNumber = i;
            table[i].accessno = 0;
        }
    }
    
    void accessPageRoundRobin(PageNumber pageNumber)
    {
        return;
    }

    void accessPageLRU(PageNumber pageNumber)
    {
        table[pageNumber].accessno = accessno; 
        accessno++;
        return;
    }
    
    /* book keeping when page hit */
    int accessPage(PageNumber pageNumber) 
    {
        switch(strategy)
        {
            case RoundRobin: 
                        accessPageRoundRobin(pageNumber);
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

        return rval;
    }

    /* frame replacement policy */
    int findVictim(PageNumber &victim)
    {
        switch(strategy)
        {
            case RoundRobin: 
                        findVictimByRoundRobin(victim);
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

    int findVictimByLRU (PageNumber &victim)
    {
        /* use whatever algo we want, LRU, MRU etc */
        /* currently this is round robin */
        int min = INT_MAX;
        int pageno = 0;
        
        for(int i=0; i<table.size(); i++) {
            if(true == table[i].isValid) {
                if(table[i].accessno < min) {
                    min = table[i].accessno;
                    pageno = i;
                }
            }
        }
        
        victim = pageno;
        zlog(ZLOG_LOC, "PageEntry::findVictimByLRU (%d) : Pageno %d, victim %d, lastaccess %d\n", 
                this->accessno, pageno, victim, min);

        return status_success;
    }
    
    int findVictimByRoundRobin(PageNumber &victim)
    {
        /* use whatever algo we want, LRU, MRU etc */
        /* currently this is round robin */
        pageno  = (pageno + 1) % NumberOfPageInVm;
        victim = pageno;
        zlog(ZLOG_LOC, "PageEntry::findVictimByRoundRobin : Pageno %d, victim %d\n", pageno, victim);

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
