/* vmm module for managing page table etc */

#ifndef __OSP_VMM_VMM_CPP__
#define __OSP_VMM_VMM_CPP__

#include "common_includes.h"
#include "pagetable.cpp"
#include "physicalmemory.cpp"


class VirtualMemoryManager
{
    PhysicalMemory physicalMemory;
    BackingStore backingStore;
    PageTable pageTable;

  public:
    VirtualMemoryManager(string backingStoreFile) : backingStore(backingStoreFile.c_str())
    {

    }

    int translate(VirtualAddress address, PhysicalAddress &physicalAddress)
    {
        int rval = status_success;

        PageEntry pageEntry;
        Page page;
        int frameno;    /* in case we need to add page to physical memory */
        int pageno = -1;
        int offset = -1;
        bool pagehit = false;
        bool tlbhit = false;


        /* break address to page no offset */
        zlog(ZLOG_LOC, "VirtualMemoryManager::translate- get page number and offset\n");
        pageno = getPageNumberFromLogicalAddress(address);
        offset = getOffsetFromLogicalAddress(address);

        /* check in tlb */

        /* check in page table if failure */
        zlog(ZLOG_LOC, "VirtualMemoryManager::translate- get page entry for page no : %d\n", pageno);
        if(status_success != pageTable.lookup(pageno, pageEntry))
        {
            rval = status_failure;
            zlog(ZLOG_LOC, "VirtualMemoryManager::translate- unable to get page table entry\n");
            goto exit1;
        }

        /* check if this is a page fault */
        zlog(ZLOG_LOC, "VirtualMemoryManager::translate- check for page fault\n");
        if(true == pageEntry.isValid)
        {
            zlog(ZLOG_LOC, "VirtualMemoryManager::translate- page hit! \n");
            pagehit = true;
        }

        /* page fault code */
        if(false == pagehit)
        {
            /* get page from backing store */
            zlog(ZLOG_LOC, "VirtualMemoryManager::translate- page fault on page : %d, reading block : %d from backing store\n",
                        pageno, pageno);
            if(status_success != backingStore.getPage(page, pageno*PageSize, PageSize))
            {
                zlog(ZLOG_LOC, "VirtualMemoryManager::translate- unable to get page form backing store\n");
                rval = status_failure;
                goto exit2;
            }

            //page.dumpPage();

            /* add to physical memory */
            zlog(ZLOG_LOC, "VirtualMemoryManager::translate- putting the page in physical memory\n");
            if(status_success != physicalMemory.addPage(page, frameno))
            {
                zlog(ZLOG_LOC, "VirtualMemoryManager::translate- unable to add page to physical memory\n");
                rval = status_failure;
                goto exit2;
            }

            /* update page table */
            zlog(ZLOG_LOC, "VirtualMemoryManager::translate- update page Entry \n");
            pageEntry.frameNumber = frameno;
            pageEntry.isModified  = false;
            pageEntry.isValid = true;

            zlog(ZLOG_LOC, "VirtualMemoryManager::translate- update page table\n");
            if(status_success != pageTable.updatePageEntry(pageEntry, pageno))
            {
                zlog(ZLOG_LOC, "VirtualMemoryManager::translate- unable to add entry to page table\n");
                rval = status_failure;
                goto exit2;
            }

            zlog(ZLOG_LOC, "VirtualMemoryManager::translate- lookup page table again\n");
            pagehit = true;
        }

        /* page entry contains the frame number required */
        physicalAddress = (frameno << numBitsForFrameOffset) + offset;

    exit2:
    exit1:
        return rval;
    }

    int readByte(VirtualAddress address, PhysicalAddress &physicalAddress, Byte &byte)
    {
        int rval = status_success;

        /* lookup */
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

    int writeByte(VirtualAddress address)
    {
        int rval = status_success;

        return rval;
    }

};

#endif