Sidharth Menon (U75158684)
Submitted: 6/27/23
Memory Virtualization
Project 2

Files required to compile the program:
- memsim.c : main interface and takes user input to call each memory management function
- fifo.c   : implements First-in First-Out functionality for the given frame count
- lru.c    : implements Least Recently Used functinality for the given frame count
- vms.c    : implements Segmented-Fifo functionality for the given frame count and percentage
- memsim.h : contains call functions to each functionality file for usage in memsim.c and vms.c

- Makefile : can call 'make' in cmd to automatically compile all files into a 'memsim' file that can be ran in cmd

Please ensure any trace files being tested are included in the same directory.


Compilation steps:
- Call the 'make' command to create a 'memsim' file
- Or "gcc -std=c99 -o memsim memsim.c fifo.c lru.c vms.c"

Run the program:
- memsim tracefile nframes policy [if vms p value] quiet/debug

Example: with file 'bzip.trace' and example nframes count 64
- fifo : memsim bzip.trace 64 fifo quiet
- lru  : memsim bzip.trace 64 lru quiet
For vms there is an additional percent input that determines what percentage of the primary buffer 
    must be split into the secondary buffer.
- vms  : memsim bzip.trace 64 vms 25 quiet