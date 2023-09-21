#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memsim.h"

typedef struct
{
    unsigned page_number;
    int dirty_bit;
    int lru_counter;
} PageFrame;

void segmented_fifo(FILE *tracefile, int nframes, int p, int debug)
{
    // if 0 or 100 p do fifo or lru calls respectively
    if (p == 100)
    {
        lru(tracefile, nframes, debug);
        return;
    }
    else if (p == 0)
    {
        fifo(tracefile, nframes, debug);
        return;
    }

    // Calculate the number of frames for primary and secondary buffers
    int secondary_frames = nframes * p / 100;
    int primary_frames = nframes - secondary_frames;

    // Initialize an array of PageFrames to store the page frames
    PageFrame *primary_fifo = (PageFrame *)malloc(primary_frames * sizeof(PageFrame));
    PageFrame *secondary_lru = (PageFrame *)malloc(secondary_frames * sizeof(PageFrame));

    // Initialize variables to keep track of statistics
    int trace_count = 0;
    int total_disk_reads = 0;
    int total_disk_writes = 0;
    int counter = 0;

    // Initialize the primary FIFO frames
    for (int i = 0; i < primary_frames; i++)
    {
        primary_fifo[i].page_number = -1; // -1 indicates an empty frame
        primary_fifo[i].dirty_bit = 0;    // Initialize dirty bit to 0
        primary_fifo[i].lru_counter = -1; // Initialize last used counter to -1
    }

    // Initialize the secondary LRU frames
    for (int i = 0; i < secondary_frames; i++)
    {
        secondary_lru[i].page_number = -1; // -1 indicates an empty frame
        secondary_lru[i].dirty_bit = 0;    // Initialize dirty bit to 0
        secondary_lru[i].lru_counter = -1; // Initialize last used counter to -1
    }

    // Read each line from the trace file
    char line[100];
    while (fgets(line, sizeof(line), tracefile) != NULL)
    {
        unsigned addr;
        char rw[2];
        sscanf(line, "%x %s", &addr, rw);
        addr /= 4096;

        // Check if the page is already in the primary FIFO frames
        int primary_found = 0;
        int primary_index = -1;
        for (int i = 0; i < primary_frames; i++)
        {
            if (primary_fifo[i].page_number == addr)
            {
                primary_found = 1;
                primary_index = i;
                break;
            }
        }

        // Check if the page is already in the secondary LRU frames
        int secondary_found = 0;
        int secondary_index = -1;
        int secondary_counter = 0;
        for (int i = 0; i < secondary_frames; i++)
        {
            secondary_counter++;
            if (secondary_lru[i].page_number == addr)
            {
                secondary_found = 1;
                secondary_index = i;
                break;
            }
        }

        if (!primary_found)
        {
            if (counter < primary_frames)
            {
                total_disk_reads++;

                // Add the page to the back of the primary FIFO
                primary_fifo[counter].page_number = addr;
                primary_fifo[counter].dirty_bit = (strcmp(rw, "W") == 0) ? 1 : 0;
                counter++;
            }
            else
            {
                // If the page is found in the secondary frame, then send it back to fifo.
                if (secondary_found)
                {
                    if (strcmp(rw, "W") == 0)
                    {
                        secondary_lru[secondary_index].dirty_bit = 1;
                    }

                    PageFrame secondtoprimary = secondary_lru[secondary_index];
                    secondtoprimary.dirty_bit = secondary_lru[secondary_index].dirty_bit;
                    PageFrame primarytosecond = primary_fifo[0];
                    primarytosecond.dirty_bit = primary_fifo[0].dirty_bit;

                    // Shift the primary FIFO frames to the left to remove the front page
                    for (int i = 0; i < primary_frames - 1; i++)
                    {
                        primary_fifo[i] = primary_fifo[i + 1];
                    }

                    // Replace the page in the secondary LRU with the front page of the primary FIFO
                    secondary_lru[secondary_index].page_number = primarytosecond.page_number;
                    secondary_lru[secondary_index].dirty_bit = primarytosecond.dirty_bit;
                    secondary_lru[secondary_index].lru_counter = 0;

                    // Add the new page to the back of the primary FIFO
                    primary_fifo[primary_frames - 1].page_number = secondtoprimary.page_number;
                    primary_fifo[primary_frames - 1].dirty_bit = secondtoprimary.dirty_bit;
                }
                if (!secondary_found)
                {
                    total_disk_reads++;
                    // Move the front page of the primary FIFO to the secondary LRU
                    int lru_index = 0;

                    int min_lru_counter = secondary_lru[0].lru_counter;

                    for (int i = 1; i < secondary_frames; i++)
                    {
                        if (secondary_lru[i].lru_counter > min_lru_counter || secondary_lru[i].lru_counter < 0)
                        {
                            lru_index = i;
                            min_lru_counter = secondary_lru[i].lru_counter;
                        }
                    }

                    if (secondary_lru[lru_index].dirty_bit == 1)
                    {
                        total_disk_writes++;
                    }
                    // Replace the page in the secondary LRU with the front page of the primary FIFO
                    secondary_lru[lru_index].page_number = primary_fifo[0].page_number;
                    secondary_lru[lru_index].dirty_bit = primary_fifo[0].dirty_bit;
                    secondary_lru[lru_index].lru_counter = 0;

                    // Shift the primary FIFO frames to the left to remove the front page
                    for (int i = 0; i < primary_frames - 1; i++)
                    {
                        primary_fifo[i] = primary_fifo[i + 1];
                    }

                    // Add the new page to the back of the primary FIFO
                    primary_fifo[primary_frames - 1].page_number = addr;
                    primary_fifo[primary_frames - 1].dirty_bit = (strcmp(rw, "W") == 0) ? 1 : 0;
                }
            }
        }
        else
        {
            //  The page is already in the primary FIFO

            // Mark the page as dirty if a write command is encountered
            if (strcmp(rw, "W") == 0)
            {
                primary_fifo[primary_index].dirty_bit = 1;
            }
        }

        // Increment lru counter
        for (int i = 0; i < secondary_frames; i++)
        {
            secondary_lru[i].lru_counter++;
        }

        // Increment trace count
        trace_count++;

        // Print debug information if enabled
        if (debug)
        {
            printf("Page: %x, Operation: %s\n", addr, rw);
            printf("Primary FIFO Frames: ");
            for (int i = 0; i < primary_frames; i++)
            {
                if (primary_fifo[i].page_number != (unsigned int)-1)
                {
                    printf("%x(%c)(%d) ", primary_fifo[i].page_number, primary_fifo[i].dirty_bit ? 'D' : 'C', primary_fifo[i].lru_counter);
                }
                else
                {
                    printf("- ");
                }
            }
            printf("\nSecondary LRU Frames: ");
            for (int i = 0; i < secondary_frames; i++)
            {
                if (secondary_lru[i].page_number != (unsigned int)-1)
                {
                    printf("%x(%c)(%d) ", secondary_lru[i].page_number, secondary_lru[i].dirty_bit ? 'D' : 'C', secondary_lru[i].lru_counter);
                }
                else
                {
                    printf("- ");
                }
            }
            printf("\n\n");
            printf("Memory Frames: %d\n", nframes);
            printf("Primary FIFO Frames: %d\nSecondary LRU Frames: %d\n", primary_frames, secondary_frames);
            printf("Trace Count: %d\nRead Count: %d\nWrite Count: %d\n", trace_count, total_disk_reads, total_disk_writes);
            printf("\n\n");
        }
    }

    // Print info
    if (!debug)
    {
        printf("Memory Frames: %d\n", nframes);
        printf("Trace Count: %d\nRead Count: %d\nWrite Count: %d\n", trace_count, total_disk_reads, total_disk_writes);
    }

    // Free dynamically allocated memory
    free(primary_fifo);
    free(secondary_lru);
}
