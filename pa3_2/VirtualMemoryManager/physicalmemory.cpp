/* module to represent physical memory */

#ifndef __OSP_VMM_PHYSICALMEMORY_CPP__
#define __OSP_VMM_PHYSICALMEMORY_CPP__

#include "common_includes.h"
#include "pageblock.cpp"

using std::vector;

class PhysicalMemory
{
    /* list of frames */
    vector<Page> frames;
    int frameno;

  public:
    PhysicalMemory()
    {
        this->initialize();
    }

    int initialize()
    {
        frames.empty();
        frames.resize(PageSize);
        frameno = -1;
    }

    ~PhysicalMemory()
    {
    }

    /* get a byte address form physical memory */
    int readByte(PhysicalAddress address, Byte &byte)
    {
        int rval = status_success;
        int framenum = -1;
        int offset = -1;

        /* split physical address byte */
        zlog(ZLOG_LOC, "PhysicalMemory::readByte - getting frame number and offset for address - %d\n", address);
        framenum = getFrameNumberFromPhysicalAddress(address);
        offset = getOffsetFromPhysicalAddress(address);

        /* get byte */
        zlog(ZLOG_LOC, "PhysicalMemory::readByte - getting byte at frame-offset  (%d - %d)\n", framenum, offset);
        rval = frames[framenum].getByte(offset, byte);
        // frames[framenum].dumpPage();

        return rval;
    }

    /* load frame to memory */
    int loadFrame(int frameno, Page page)
    {
        int rval = status_success;

        /* check frame no  */
        zlog(ZLOG_LOC, "PhysicalMemory::loadFrame - loading page to frame number : %d\n", frameno);
        if(frameno >= NumberOfFramesInPm)
        {
            zlog(ZLOG_LOC, "PhysicalMemory::loadFrame - frame number is not valid (%d) - Total number of frames (%d)\n",
            frameno, NumberOfFramesInPm);
            rval = status_failure;
            goto exit1;
        }

        /* copy page to frame */
        frames[frameno] = page;

    exit1:
        return rval;
    }

    /* add a page to a frame in physical memory */
    int addPage(Page page, int &frameno)
    {
        int rval = status_success;
        int victim = -1;

        /* get victim page */
        zlog(ZLOG_LOC, "PhysicalMemory::addPage - find victim page\n");
        if(status_success != (rval = findVictim(victim)))
        {
            rval = status_failure;
            zlog(ZLOG_LOC, "PhysicalMemory::addPage - unable to find victim page\n");
            goto exit1;
        }

        /* write to victim frame */
        zlog(ZLOG_LOC, "PhysicalMemory::addPage - loading frame at victim : %d\n", victim);
        rval = loadFrame(victim, page);
        frameno = victim;

    exit1:
        return rval;
    }

    /* write a byte to address form physical memory */
    int writeByte(PhysicalAddress address, Byte byte)
    {
        int rval = status_success;
        int frameno = -1;
        int offset = -1;

        /* split physical address byte */
        zlog(ZLOG_LOC, "PhysicalMemory::writeByte - getting frame number and offset for address - %d\n", address);
        frameno = getFrameNumberFromPhysicalAddress(address);
        offset = getOffsetFromPhysicalAddress(address);

        /* get byte */
        zlog(ZLOG_LOC, "PhysicalMemory::writeByte - getting byte at frame-offset  (%d - %d)\n", frameno, offset);
        rval = frames[frameno].putByte(offset, byte);

        return rval;
    }

    /* frame replacement policy */
    int findVictim(int &victim)
    {
        /* use whatever algo we want, LRU, MRU etc */
        /* currently this is round robin */
        frameno = (frameno + 1) % NumberOfFramesInPm;
        victim = frameno;

        return status_success;
    }

    /* dump physical memory, debugging purpose */
    int dumpMemory(int frameno)
    {
        this->frames[frameno].dumpPage();
    }
};


#endif
