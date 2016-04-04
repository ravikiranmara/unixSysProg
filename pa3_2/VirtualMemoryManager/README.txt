Instructions to compile
------------------------------
please run the makefile. This outputs the executable a.out
<The program uses a third party log library zlog for logging. the files are included in the submission>

Testing 
---------

for testing, I used the same file. the following are the results 

<snip>
    Number of Translated Addresses = 1000
    Page Faults = 244
    Page Fault Rate = 0.244
    TLB Hits = 54
    TLB Hit Rate = 0.054
</snip>

We can make the following observations. 
    - We encountered 244 page faults. We had a total of 256 pages in the backing store. So 12 pages were never brought into memory (as they were not required). Thus this demonstrates how virtual memory can save memory space. 
    - The number of page faults max out at 256, because the logical address space is equal to the physical address space. 
    - Tlb hit rate is very low. Increasing the number of tlb (globals.h - NumberOfTlbEntries = 32), increases tlb hit rate. Also the policy used for tlb replacement is round robin (fifo), this also affects the tlb hit rate. Most Frequently used, Least Recently used policies might result in better tlb hit rate. 

<snip tlb=32> 
    Number of Translated Addresses = 1000
    Page Faults = 244
    Page Fault Rate = 0.244
    TLB Hits = 118
    TLB Hit Rate = 0.118
</snip>


Known Issues 
------------------
    - now known issues, the program runs to completion as expected. 

    - issues faced during implementation, the logic of the program is pretty straight forward. At times, debugging was tricky, as we need to track down the error. 


Learning
-------------
    - An overview of how the address translation works. The mapping of physical memory to logical memory. 
    - Demand paging - this shows that not all of the pages are required for execution. 
    - tlb and page table operations - options to use differnt policies
    - How the differnt modules are organized in the operating system, and their interactions. 
