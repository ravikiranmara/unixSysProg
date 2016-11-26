/* vmm module for managing page table etc.

   The main responsibility of virtual memory manager is to manage
   virtual addresss space (in tandem with memory manager )and translate
   the virtual address to physical address.

   Additionally we are also performing the read operation in VMM.

   Ideally backing store, physical memory are different entities, that are to
   be enclosed in a bigger contianer say Computer. To avoid complicating the code, we include
   it here.
*/

#ifndef __OSP_VMM_VMM_CPP__
#define __OSP_VMM_VMM_CPP__

#include "common_includes.h"
#include "pagetable.cpp"
#include "physicalmemory.cpp"
#include "tlb.cpp"

class VirtualMemoryManager
{
    PhysicalMemory physicalMemory;      /* represents RAM, list of frames with data */
    BackingStore backingStore;          /* represents persistant storage, hard disk */
    PageTable pageTable;                /* contains logical to physical address mapping */
    WorkingSet workingSet;              /* book keeping for physical memory */
    Tlb tlb;                            /* cache for page table */

    int pagehits;                       /* used to keep stats */
    int pagefaults;
    int tlbhits;
    int pagelookups;

  public:
    VirtualMemoryManager(string backingStoreFile, ReplacementStrategy strategy) : backingStore(backingStoreFile.c_str())
    {
        /* initialize my stats to zero */
        pagehits = 0;
        pagefaults = 0;
        pagelookups = 0;
        tlbhits = 0;
        pageTable.setStrategy(strategy);
    }

    /* translate virtual address to pysical address. handles tlbmiss, page faults,
    loads page to physical memory in case of page fault */
    int translate(VirtualAddress address, PhysicalAddress &physicalAddress)
    {
        int rval = status_success;

        PageEntry pageEntry;    /* the page entry from page table. We use this for lookup in page table, and get frame no from it */
        PageEntry victimEntry;
        Page page;              /* in case we need to get page from backing store and add it to physical memory */
        FrameNumber frameno;    /* in case we need to add page to physical memory */
        PageNumber pageno = -1; /* page number we will lookkup. get this from virtual address */
        FrameNumber frameVictim = -1;
        PageNumber pageVictim = -1;

        int offset = -1;        /* offset we will lookkup. get this from virtual address */
        bool pagehit = false;   /* flags */
        bool tlbhit = false;


        /* break address to page no offset */
        //zlog(ZLOG_LOC, "VirtualMemoryManager::translate- get page number and offset\n");
        pageno = getPageNumberFromLogicalAddress(address);
        offset = getOffsetFromLogicalAddress(address);

        /* we are storing he required frame number in the frameno value, weather we get a hit
        from tlb, page table or page fault */

        /* check in tlb */
        tlb.lookup(pageno, frameno, tlbhit);
        if(true == tlbhit)
        {
            /* we are done here */
            zlog(ZLOG_LOC, "VirtualMemoryManager::translate - tlb hit !!\n");
            pageTable.lookup(pageno, pageEntry);
            tlbhits++;
        }
        else
        {
            /* check in page table tlb miss */
            zlog(ZLOG_LOC, "VirtualMemoryManager::translate- get page entry for page no : %d\n", pageno);
            if(status_success != pageTable.lookup(pageno, pageEntry))
            {
                rval = status_failure;
                zlog(ZLOG_LOC, "VirtualMemoryManager::translate- unable to get page table entry\n");
                goto exit1;
            }

            /* check if this is a page fault */
            // zlog(ZLOG_LOC, "VirtualMemoryManager::translate- check for page fault\n");
            if(true == pageEntry.isValid)
            {
                /* we are done */
                zlog(ZLOG_LOC, "VirtualMemoryManager::translate- page hit! \n");
                frameno = pageEntry.frameNumber;
                pagehit = true;

                /* update counter */
                pagehits++;
            }

            /* page fault code */
            if(false == pagehit)
            {
                /* update counter */
                pagefaults++;

                /* get page from backing store */
                zlog(ZLOG_LOC, "VirtualMemoryManager::translate- page fault on page : %d, reading block : %d from backing store\n",
                            pageno, pageno);
                if(status_success != backingStore.getPage(page, pageno*PageSize, PageSize))
                {
                    zlog(ZLOG_LOC, "VirtualMemoryManager::translate- unable to get page form backing store\n");
                    rval = status_failure;
                    goto exit2;
                }

                /* check if we have a vacant memory */
                if(workingSet.isFreeFrame())
                {
                    frameno = workingSet.getFreeFrame();
                }
                else
                {
                    /* get victim page no */
                    pageTable.findVictim(frameVictim);
                    pageTable.rlookup(frameVictim, pageVictim, victimEntry);
                    frameno = frameVictim;

                    /* make victim invalid */
                    if(victimEntry.isValid == true)
                    {
                        victimEntry.isValid = false;
                        zlog(ZLOG_LOC, "VirtualMemoryManager::translate- update page table victim\n");
                        if(status_success != pageTable.updatePageEntry(victimEntry, pageVictim))
                        {
                            zlog(ZLOG_LOC, "VirtualMemoryManager::translate- unable to add entry to page table\n");
                            rval = status_failure;
                            goto exit2;
                        }

                        zlog(ZLOG_LOC, "VirtualMemoryManager::translate- invalidate entry in tlb\n");
                        tlb.invalidate(victimEntry.frameNumber);
                    }
                }

                /* add to physical memory */
                zlog(ZLOG_LOC, "VirtualMemoryManager::translate- putting the page in physical memory at victim : %d\n", frameno);
                if(status_success != physicalMemory.addPage(page, frameno))
                {
                    zlog(ZLOG_LOC, "VirtualMemoryManager::translate- unable to add page to physical memory\n");
                    rval = status_failure;
                    goto exit2;
                }

                /* update page entry */
                // zlog(ZLOG_LOC, "VirtualMemoryManager::translate- update page Entry \n");
                pageEntry.frameNumber = frameno;
                pageEntry.isModified  = false;
                pageEntry.isValid = true;

                /* add page entry to page table */
                // zlog(ZLOG_LOC, "VirtualMemoryManager::translate- update page table lookup\n");
                if(status_success != pageTable.updatePageEntry(pageEntry, pageno))
                {
                    zlog(ZLOG_LOC, "VirtualMemoryManager::translate- unable to add entry to page table\n");
                    rval = status_failure;
                    goto exit2;
                }

                // zlog(ZLOG_LOC, "VirtualMemoryManager::translate- lookup page table again\n");
                frameno = pageEntry.frameNumber;
                pagehit = true;
            }

            /* by this time we have a valid page frame mapping
                if there was a tlb miss, update tlb table */
            if(false == tlbhit)
            {
                TlbEntry tlbEntry;
                tlbEntry.pageNumber = pageno;
                tlbEntry.frameNumber = frameno;
                tlbEntry.valid = true;

                tlb.addEntry(tlbEntry);
            }
        }

        /* page entry contains the frame number required */
        physicalAddress = (frameno << numBitsForFrameOffset) + offset;

    exit2:
    exit1:
        return rval;
    }

    /* uses translate function to get physical address, and reads byte from physcial memory */
    int readByte(VirtualAddress address, PhysicalAddress &physicalAddress, Byte &byte)
    {
        int rval = status_success;

        /* lookup request add counter */
        pagelookups++;

        /* lookup in tlb, page table to get physical address */
        zlog(ZLOG_LOC, "VirtualMemoryManager::readByte - translate address to get physical address : %d \n", address);
        if(status_failure == this->translate(address, physicalAddress))
        {
            zlog(ZLOG_LOC, "VirtualMemoryManager::readByte - unable to translate address \n");
            rval = status_failure;
            goto exit1;
        }

        /* get the byte from physical memory */
        zlog(ZLOG_LOC, "VirtualMemoryManager::readByte - read from physical memory at : %d \n", physicalAddress);
        if(status_failure == physicalMemory.readByte(physicalAddress, byte))
        {
            zlog(ZLOG_LOC, "VirtualMemoryManager::readByte - unable to read from physical memory \n");
            rval = status_failure;
            goto exit1;
        }

    exit1:
        return rval;
    }

    /* not implemented, not required for assignment */
    int writeByte(VirtualAddress address)
    {
        int rval = status_success;

        return rval;
    }

    /* just return the counters */
    int getStats (int &totalLookups, int &pagefaults, int &pageHits, int &tlbhits)
    {
        totalLookups = this->pagelookups;
        pagefaults = this->pagefaults;
        pagehits = this->pagehits;
        tlbhits = this->tlbhits;
    }

};

#endif
