#ifndef MEMSIM_H
#define MEMSIM_H

#include <stdio.h>

void fifo(FILE *tracefile, int nframes, int debug);
void lru(FILE *tracefile, int nframes, int debug);
void segmented_fifo(FILE *tracefile, int nframes, int p, int debug);

#endif // MEMSIM_H
