#include <stddef.h>

#include <stdio.h>

#define MEM_POOL_SIZE 4096
char _pool[MEM_POOL_SIZE];

struct _block {
    int size;
    char free;
    void *data;
};

struct mem_stats {
    int total_blocks;
    int active_blocks;
    int bytes_in_use;
};

typedef struct _block _block;

char _pool_initialized = 0;
int _total_blocks = 0;
int _active_blocks = 0;
int _bytes_in_use = 0;


void
_print_mem (){
    _block * cur = (_block *) _pool;
    while (cur < (_block *) (_pool + MEM_POOL_SIZE)){
        printf("[%p](%lx:%d:%d)->", cur, cur->size + sizeof(_block), cur->size, cur->free);
        cur = (_block *) &cur->data + cur->size;
    }
    printf("[%p](END)\n", _pool+MEM_POOL_SIZE);

}

void
_init_block (_block * b, int size){
    b->size = size - sizeof(_block);
    b->free = 1;

    _total_blocks ++;
    _bytes_in_use += sizeof(_block);
}


void
_init_pool (void){
    if (_pool_initialized != 0)
        return;

    _init_block( (_block*) _pool,
                 MEM_POOL_SIZE);

    _pool_initialized = 1;
    return;
}


void
_allocate (_block * b){
    _active_blocks ++;
    _bytes_in_use += b->size;
    b->free = 0;
}


void
_free_block (_block *b){
    _active_blocks --;
    _bytes_in_use -= b->size;
    b->free = 1;
}


void *
_align (int size, _block* b){
    if (size == b->size) {
        _allocate(b);
        return &b->data;
    }

    if (b->size - size <= sizeof(_block) + 1) {
        _allocate(b);
        return &b->data;
    }

    _print_mem();
    _block * new_block = (_block *) &b->data + size;
    _init_block( new_block, 
                 b->size - size);

    b->size = size;

    _allocate(b);
    return &b->data;
}


void
_coalesce (_block *b){
}


void
_coalescer (void){
}


int
_valid (void *b){
    if ((void*)_pool > b || (void*)_pool + MEM_POOL_SIZE < b)
        return -1;

    /*
    memBlock *block = b - sizeof(memBlock);
    memBlock * cur = memHead;
    while (cur){
        if (cur == block)
            return 0;
        cur = cur->next;
    }
    */

    return -1;
}


void *
mem_alloc (int size){
    if (_pool_initialized == 0)
        _init_pool();

    if (MEM_POOL_SIZE - _bytes_in_use < size)
        return NULL;

    _block * cur = (_block *) _pool;
    while (cur < (_block *) (_pool + MEM_POOL_SIZE)){
        if (cur->free && cur->size >= size) {
            return (void *) _align(size, cur);
        }
        cur = (_block *) &cur->data + cur->size;
    }
    return NULL;
}


void
mem_free (void *b){
    if (_pool_initialized == 0)
        return;
    /*
    if (-1 == valid(b))
        return -1;

    memBlock *block = b - sizeof(memBlock);
    free_block(block);
    */
    return;
}


void
mem_stat (struct mem_stats * s){
    if (_pool_initialized == 0)
        _init_pool();

    s->total_blocks = _total_blocks;
    s->active_blocks = _active_blocks;
    s->bytes_in_use = _bytes_in_use;
}


