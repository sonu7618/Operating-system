/*=========================================================
  File: task2_paging.c

  Description:
  This program simulates virtual memory paging using
  FIFO and LRU page replacement algorithms.
  It compares page faults and page hits for both methods.

  Compile:
      gcc task2_paging.c -o task2_paging

  Run:
      ./task2_paging
==========================================================*/

#include <stdio.h>

#define NUM_FRAMES 4
#define PAGE_SIZE_KB 4

/* Page reference sequence */
int pages[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
int total_pages = sizeof(pages) / sizeof(pages[0]);

/* Display current frame contents */
void displayFrames(int frames[])
{
    printf("[ ");
    for (int i = 0; i < NUM_FRAMES; i++)
    {
        if (frames[i] == -1)
            printf("- ");
        else
            printf("%d ", frames[i]);
    }
    printf("]");
}

/* FIFO Page Replacement */
void fifo()
{
    int frames[NUM_FRAMES];
    int pointer = 0;
    int hits = 0, faults = 0;

    for (int i = 0; i < NUM_FRAMES; i++)
        frames[i] = -1;

    printf("\n========== FIFO PAGE REPLACEMENT ==========\n");
    printf("Step\tPage\tResult\tFrames\n");

    for (int i = 0; i < total_pages; i++)
    {
        int page = pages[i];
        int found = 0;

        for (int j = 0; j < NUM_FRAMES; j++)
        {
            if (frames[j] == page)
            {
                found = 1;
                break;
            }
        }

        if (found)
        {
            hits++;
            printf("%2d\t%2d\tHit\t", i + 1, page);
        }
        else
        {
            faults++;
            frames[pointer] = page;
            pointer = (pointer + 1) % NUM_FRAMES;

            printf("%2d\t%2d\tFault\t", i + 1, page);
        }

        displayFrames(frames);
        printf("\n");
    }

    printf("\nTotal References : %d\n", total_pages);
    printf("Page Hits        : %d\n", hits);
    printf("Page Faults      : %d\n", faults);
    printf("Hit Ratio        : %.2f%%\n", (hits * 100.0) / total_pages);
    printf("Fault Ratio      : %.2f%%\n", (faults * 100.0) / total_pages);
}

/* LRU Page Replacement */
void lru()
{
    int frames[NUM_FRAMES];
    int recent[NUM_FRAMES];
    int hits = 0, faults = 0;

    for (int i = 0; i < NUM_FRAMES; i++)
    {
        frames[i] = -1;
        recent[i] = -1;
    }

    printf("\n========== LRU PAGE REPLACEMENT ==========\n");
    printf("Step\tPage\tResult\tFrames\n");

    for (int i = 0; i < total_pages; i++)
    {
        int page = pages[i];
        int index = -1;

        for (int j = 0; j < NUM_FRAMES; j++)
        {
            if (frames[j] == page)
            {
                index = j;
                break;
            }
        }

        if (index != -1)
        {
            hits++;
            recent[index] = i;
            printf("%2d\t%2d\tHit\t", i + 1, page);
        }
        else
        {
            faults++;

            int target = -1;

            /* Check for empty frame */
            for (int j = 0; j < NUM_FRAMES; j++)
            {
                if (frames[j] == -1)
                {
                    target = j;
                    break;
                }
            }

            /* Replace least recently used page */
            if (target == -1)
            {
                target = 0;

                for (int j = 1; j < NUM_FRAMES; j++)
                {
                    if (recent[j] < recent[target])
                        target = j;
                }
            }

            frames[target] = page;
            recent[target] = i;

            printf("%2d\t%2d\tFault\t", i + 1, page);
        }

        displayFrames(frames);
        printf("\n");
    }

    printf("\nTotal References : %d\n", total_pages);
    printf("Page Hits        : %d\n", hits);
    printf("Page Faults      : %d\n", faults);
    printf("Hit Ratio        : %.2f%%\n", (hits * 100.0) / total_pages);
    printf("Fault Ratio      : %.2f%%\n", (faults * 100.0) / total_pages);
}

int main()
{
    printf("Reference String: ");

    for (int i = 0; i < total_pages; i++)
        printf("%d ", pages[i]);

    printf("\n");

    fifo();
    lru();

    printf("\n========== COMPARISON ==========\n");
    printf("Both FIFO and LRU used the same reference string.\n");
    printf("Compare the page hits and page faults above to see which performs better.\n");

    return 0;
}