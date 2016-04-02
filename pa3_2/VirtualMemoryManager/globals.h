#ifndef __OSP_VMM_GLOBALS_H__
#define __OSP_VMM_GLOBALS_H__

#include <sys/types.h>
#include <stdint.h>
#include <cmath>

/*
* Definitions for global and config data
*/

/* return status success */
const int status_success = 0;
const int status_failure = -1; /* there has to be a better way */

/* address representations */
typedef uint16_t VirtualAddress;    /* represents logical address (8bit page no + 8 bit offset) */
typedef uint16_t PhysicalAddress;   /* represents physical address (8bit page no + 8 bit offset)  */
typedef uint8_t Byte;               /* to repersent byte insetad of chars */

/* very convenient for us that Frame number is 8 bits. If it is more
choose next available type (16 bits). we just need a representation
that is large enough to hold the frame and page number.  */
typedef uint8_t FrameNumber;
typedef uint8_t PageNumber;

/* VMM Page table config */
const int PageSize = 256;
const int FrameSize = 256;
const int NumberOfPageInVm = 256;  /* Number of pages in table */
const int NumberOfFramesInPm = 256;     /* Number of frames in physical memory  */
const int NumberOfTlbEntries = 16;         /* number of cache entries in tlb table */
const int PhysicalMemorySize = 65536;   /* size of physical memory. do we need this? */

const int numBitsForFrameNo = log2(NumberOfFramesInPm); /* 256 pages = 2^8 = 8 bits */
const int numBitsForPageNo = log2(NumberOfPageInVm); /* 256 pages = 2^8 = 8 bits */
const int numBitsForPageOffset = log2(PageSize);
const int numBitsForFrameOffset = log2(FrameSize);

/* bitmasks (change if page memory size changes)*/
const VirtualAddress VirtualAddressOffsetMask = (Byte)0xFF;
const PhysicalAddress PhysicalAddressOffsetMask = (Byte)0xFF;

const VirtualAddress pageNumMask = (Byte)0xFF00;
const PhysicalAddress frameNumMaks = (Byte)0xFF00;

/* bitmask helper */
int getOffsetFromPhysicalAddress(PhysicalAddress address)
{
    return address & PhysicalAddressOffsetMask;
}

int getOffsetFromLogicalAddress(VirtualAddress address)
{
    return address & VirtualAddressOffsetMask;
}

int getFrameNumberFromPhysicalAddress(PhysicalAddress address)
{
    return (address & frameNumMaks) >> numBitsForFrameNo;
}

int getPageNumberFromLogicalAddress(VirtualAddress address)
{
    return (address & pageNumMask) >> numBitsForPageNo;
}

#endif
