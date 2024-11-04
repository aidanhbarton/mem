#ifndef MEM_H
#define MEM_H
#endif

struct mem_stats {
    int total_blocks;
    int active_blocks;
    int bytes_in_use;
};

void * mem_alloc (int size);
void mem_free (void *b);
void mem_stat (struct mem_stats * s);
