/* tlb cache for vmm */
#ifndef __OSP_VMM_TLB_CPP__
#define __OSP_VMM_TLB_CPP__

#include "common_includes.h"
#include "globals.h"

using std::vector;

/* tlb entry, page -> frame translation */
typedef struct TlbEntry
{
    PageNumber pageNumber;
    FrameNumber frameNumber;
    bool valid;

    TlbEntry()
    {
        valid = false;
    }

    void operator=(TlbEntry te)
    {
        this->pageNumber = te.pageNumber;
        this->frameNumber = te.frameNumber;
        this->valid = te.valid;
    }

} TlbEntry;


/* tlb table */
class Tlb
{
    vector<TlbEntry> table;
    int tlbHitCount;
    int totalNumberOfLookup;
    int robin;

  public:
    Tlb()
    {
        this->initialize();
    }

    int initialize()
    {
        tlbHitCount = 0;
        totalNumberOfLookup = 0;
        robin = -1;
        table.clear();
        table.resize(NumberOfTlbEntries);
    }

    /* see if there is an entry for tlb */
    int lookup(PageNumber pageno, FrameNumber &frameno, bool &hit)
    {
        int rval = status_success;
        TlbEntry tlbEntry;
        bool tlbhit = false;

        zlog(ZLOG_LOC, "Tlb::lookup - lookup page : %d\n", pageno);
        for(int i=0; i<NumberOfTlbEntries; i++)
        {
            tlbEntry = table[i];

            /* check if tlb entry is valid */
            if(tlbEntry.valid == true)
            {
                if(tlbEntry.pageNumber == pageno)
                {
                    zlog(ZLOG_LOC, "Tlb::lookup - tlb hit!\n");
                    tlbhit = true;
                    break;
                }
            }
        }

        /* if tlb hit, get frame number and update fault */
        if(true == tlbhit)
        {
            zlog(ZLOG_LOC, "Tlb::lookup - updating frame number\n");
            frameno = tlbEntry.frameNumber;
        }
        else
        {
            zlog(ZLOG_LOC, "Tlb::lookup - tlb miss\n");
        }

        hit = tlbhit;
        return rval;
    }

    /* add entry to tlb */
    int addEntry(TlbEntry tlbEntry)
    {
        int rval = status_success;
        int victim;

        /* get victim */
        zlog(ZLOG_LOC, "Tlb::addEntry - get victim page\n");
        if(status_failure == getVictim(victim))
        {
            zlog(ZLOG_LOC, "Tlb::addEntry - Unable to get victim page\n");
            rval = status_failure;
            goto exit1;
        }

        /* now update page */
        zlog(ZLOG_LOC, "Tlb::addEntry - updating tlb at victim frame : %d\n", victim);

        table[victim] = tlbEntry;

    exit1:
        return rval;
    }

    /* replacement policy - round robin */
    int getVictim(int &victim)
    {
        robin = (robin + 1) % NumberOfTlbEntries;
        victim = robin;

        return status_success;
    }
};

#endif
