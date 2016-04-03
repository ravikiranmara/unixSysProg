/* page table implementation */

#ifndef __OSP_VMM_PAGETABLE_CPP__
#define __OSP_VMM_PAGETABLE_CPP__

#include "common_includes.h"
#include "pageblock.cpp"

using std::vector;

/* page entry. contains list of properties for page */
typedef struct PageEntry
{
    bool isValid;
    bool isModified;
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
        this->invalidateTable();
    }

    /* invalidate all page entries */
    void invalidateTable()
    {
        for(int i=0; i<table.size(); i++)
        {
            table[i].isValid = false;
        }
    }

    /* search in the page table to see if we have a valid entry for page number */
    int lookup(PageNumber pageNumber, PageEntry &pageEntry)
    {
        int rval = status_failure;

        /* if page number is valid */
        if(pageNumber < NumberOfPageInVm)
        {
            pageEntry = table[pageNumber];
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

        /* update page  */
        table[pageno] = pageEntry;

        return rval;
    }

    /* frame replacement policy */
    int findVictim(PageNumber &victim)
    {
        /* use whatever algo we want, LRU, MRU etc */
        /* currently this is round robin */
        pageno  = (pageno + 1) % NumberOfPageInVm;
        victim = pageno;

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
