#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    unsigned page_number;
    int dirty_bit;
    int last_used;
} PageFrame;

void lru(FILE *tracefile, int nframes, int debug)
{
    // Initialize an array of PageFrames to store the page frames
    PageFrame *page_frames = (PageFrame *)malloc(nframes * sizeof(PageFrame));

    // Initialize variables to keep track of statistics
    int trace_count = 0;
    int total_disk_reads = 0;
    int total_disk_writes = 0;
    int counter = 0;

    // Initialize the page frames
    for (int i = 0; i < nframes; i++)
    {
        page_frames[i].page_number = -1; // -1 indicates an empty frame
        page_frames[i].dirty_bit = 0;    // Initialize dirty bit to 0
        page_frames[i].last_used = 0;    // Initialize last used counter to 0
    }

    // Read each line from the trace file
    char line[100];
    while (fgets(line, sizeof(line), tracefile) != NULL)
    {
        unsigned addr;
        char rw[2];
        sscanf(line, "%x %s", &addr, rw);
        addr /= 4096;

        // Check if the page is already in a frame
        int page_found = 0;
        int page_index = -1;
        for (int i = 0; i < nframes; i++)
        {
            if (page_frames[i].page_number == addr)
            {
                page_found = 1;
                page_index = i;
                break;
            }
        }

        if (!page_found)
        {
            // Page fault occurred
            total_disk_reads++;

            if (counter < nframes)
            {
                // There are still empty frames available
                page_frames[counter].page_number = addr;
                page_frames[counter].dirty_bit = (strcmp(rw, "W") == 0) ? 1 : 0;
                page_frames[counter].last_used = trace_count;
                counter++;
            }
            else
            {
                // Find the index of the page with the least recently used counter
                int lru_index = 0;
                int min_last_used = page_frames[0].last_used;

                for (int i = 1; i < nframes; i++)
                {
                    if (page_frames[i].last_used < min_last_used)
                    {
                        lru_index = i;
                        min_last_used = page_frames[i].last_used;
                    }
                }

                if (page_frames[lru_index].dirty_bit == 1)
                {
                    total_disk_writes++;
                }

                // Replace the page with the new page
                page_frames[lru_index].page_number = addr;
                page_frames[lru_index].dirty_bit = (strcmp(rw, "W") == 0) ? 1 : 0;
                page_frames[lru_index].last_used = trace_count;
            }
        }
        else
        {
            // Page is already in a frame

            // Mark the page as dirty if a write command is encountered
            if (strcmp(rw, "W") == 0)
            {
                page_frames[page_index].dirty_bit = 1;
            }

            // Update the last used counter
            page_frames[page_index].last_used = trace_count;
        }

        // Increment trace count
        trace_count++;

        // Print debug information if enabled
        if (debug)
        {
            printf("Page: %x, Operation: %s\n", addr, rw);
            printf("Page Frames: ");
            for (int i = 0; i < nframes; i++)
            {
                if (page_frames[i].page_number != (unsigned int)-1)
                {
                    printf("%x(%c) ", page_frames[i].page_number, page_frames[i].dirty_bit ? 'D' : 'C');
                }
                else
                {
                    printf("- ");
                }
            }
            printf("\n\n");
            printf("Memory Frames: %d\n", nframes);
            printf("Trace Count: %d\nRead Count: %d\nWrite Count: %d\n", trace_count, total_disk_reads, total_disk_writes);
            printf("\n\n");
        }
    }

    // Print statistics
    if (!debug)
    {
        printf("Memory Frames: %d\n", nframes);
        printf("Trace Count: %d\nRead Count: %d\nWrite Count: %d\n", trace_count, total_disk_reads, total_disk_writes);
    }

    // Free dynamically allocated memory
    free(page_frames);
}
