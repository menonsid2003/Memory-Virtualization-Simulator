#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    unsigned page_number;
    int dirty_bit;
} PageFrame;

void fifo(FILE *tracefile, int nframes, int debug)
{
    // Initialize an array of PageFrames to store the page frames
    PageFrame *page_frames = (PageFrame *)malloc(nframes * sizeof(PageFrame));

    // Initialize variables to keep track of statistics
    int trace_count = 0;
    int total_disk_reads = 0;
    int total_disk_writes = 0;

    // Initialize the page frames
    for (int i = 0; i < nframes; i++)
    {
        page_frames[i].page_number = (unsigned int)-1; // -1 indicates an empty frame
        page_frames[i].dirty_bit = 0;                  // Initialize dirty bit to 0
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

            // Find the index of the oldest page in the frame
            int oldest_frame_index = 0;

            if (page_frames[oldest_frame_index].page_number != (unsigned int)-1)
            {
                if (page_frames[oldest_frame_index].dirty_bit == 1)
                {
                    total_disk_writes++;
                    // Set oldest element back to clean bit to get ready for deletion
                    page_frames[oldest_frame_index].dirty_bit = 0;
                }
            }

            // Move all the frames back by one to reset oldest page
            for (int i = 0; i < nframes - 1; i++)
            {
                page_frames[i] = page_frames[i + 1];
            }

            // set incoming page to the last element
            page_frames[nframes - 1].page_number = addr;
            if (strcmp(rw, "R") == 0)
            {
                // total_disk_reads++;
                page_frames[nframes - 1].dirty_bit = 0;
            }
            else if (strcmp(rw, "W") == 0)
            {
                // Mark the page as dirty
                page_frames[nframes - 1].dirty_bit = 1;
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
