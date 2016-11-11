/* test module */

int test_backing(string filename)
{
    int rval = status_success;
    BackingStore bs (filename.c_str()); //  = new BackingStore(filename.c_str());
    Page buffer;
    PhysicalMemory physicalMemory;
    PhysicalAddress address;
    Byte byte;

    zlog(ZLOG_LOC, "Global::test backing start\n");
    /* check if page is open */
    if(true == bs.isFileOpen())
    {
        zlog(ZLOG_LOC, "Global::test_backing - file is open\n");
    }

    /* get a chunk of memory  */
    if(status_success != (rval = bs.getPage(buffer, 0, PageSize)))
    {
        zlog(ZLOG_LOC, "Global::test_backing Error : unable to get page - %d\n", rval);
        zlog(ZLOG_LOC, strerror(rval));
        goto exit1;
    }

    /* print values */
    buffer.dumpPage();

    /* write page to physical memory */
    int frameno;
    if(status_success != physicalMemory.addPage(buffer, frameno))
    {
        zlog(ZLOG_LOC, "Global::test_backing - failed to add page to physical memory\n");
        goto exit1;
    }

    /* dump to see if physical memory is same */
    zlog(ZLOG_LOC, "Global::test_backing - dumping physical memory page load\n");
    physicalMemory.dumpMemory(frameno);

    /* see if we can get and set byte on physical memory */
    address = (frameno << numBitsForFrameOffset);
    for(int i=0; i<20; i++)
    {
        zlog(ZLOG_LOC, "Global::test_backing - get byte at : %d\n", address);

        /* get */
        physicalMemory.readByte(address, byte);
        zlog(ZLOG_LOC, "Global::test_backing - got byte - %d\n", byte);
        address++;
    }

    /* set byte */
    byte = 10;
    physicalMemory.writeByte(address, byte);

    /* get again */
    physicalMemory.readByte(address, byte);
    zlog(ZLOG_LOC, "Global::test_backing - new got byte - %d\n", byte);

exit1:
    /* close file */
    bs.closeFile();
    zlog(ZLOG_LOC, "Global::test_backing - exit : %d\n", rval);
    return rval;

}

void test_pagetable()
{
    PageTable pageTable;
    PageEntry pageEntry;
    PageNumber pageno;

    /* add a bunch of pages */
    for (int i=0; i<100; i++)
    {
        /* page entry edit */
        pageEntry.frameNumber = i;
        pageEntry.isValid = true;
        pageEntry.isModified = false;

        /* add page entry */
        pageTable.addPageEntry(pageEntry, pageno);
    }

    /* dump page table */
    // pageTable.dumpPageTable();

    /* lets lookup instead */
    for(int i=0; i<100; i++)
    {
        pageTable.lookup(i, pageEntry);
        pageEntry.dump();
    }

    return;
}
