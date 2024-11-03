#include <stdio.h>
#include <stdbool.h>

#define MEM_POOL_CAP 4096
char memPool[MEM_POOL_CAP];

struct memBlock {
    int size;
    bool free;
    struct memBlock *next;
    char *data;
};

typedef struct memBlock memBlock;

memBlock * memHead = NULL;

void initBlock(memBlock * b, int size, memBlock * n) {
    b->size = size;
    b->free = true;
    b->next = n;
    b->data = (char*)(b + sizeof(memBlock));
}

/* Initialize memory pool */
int Mem_init (){
    if (memHead)
        return 0;

    memHead = (memBlock *)memPool;
    initBlock(  memHead, 
                MEM_POOL_CAP - sizeof(memBlock), 
                NULL);
    return 0;
}

/* Print the state of the memory pool */
void Mem_print (){
    if (memHead == NULL) {
        printf("Pool uninitialized...");
        return;
    }

    printf("Pool:\n");
    memBlock * cur = memHead;
    printf("[%d:%d](%p)->", cur->size, cur->free, cur->data);
    int numBlocks = 1;
    int totMemAlloced = 0;
    if (!cur->free) {
        totMemAlloced += cur->size;
    }

    while (cur->next) {
        cur = cur->next;
        printf("[%d:%d](%p)->", cur->size, cur->free, cur->data);
        numBlocks += 1;
        if (!cur->free) {
            totMemAlloced += cur->size;
        }
    }
    printf("NULL\n");
    printf("num blocks: %d\nalloced: %d / %d\n\n", numBlocks, totMemAlloced, MEM_POOL_CAP);
}

char * allocate (memBlock* block){
    block->free = false;
    return block->data;
}

char * fit (int size, memBlock* block){
    if (size == block->size)
        return allocate(block);

    if (block->size - size <= sizeof(memBlock))
        return allocate(block);

    memBlock * newBlock = (memBlock *) block->data + size;
    initBlock(  newBlock, 
                block->size - size - sizeof(memBlock), 
                block->next);

    block->size = size;
    block->next = newBlock;

    return allocate(block);
}

/*  Allocate `size` bytes of memory from the pool,
    Returns a pointer to the newly allocated memory. */
void * Mem_alloc (int size){
    if (memHead == NULL)
        Mem_init();

    memBlock * cur = memHead;
    while (cur){
        if (cur->free && cur->size >= size)
            return (void *) fit(size, cur);
        cur = cur->next;
    }
    return NULL;
}

void coalesce (memBlock *b){
    b->size += b->next->size + sizeof(memBlock);
    b->next = b->next->next;
}

void coalescer (){
    memBlock * cur = memHead;
    while (cur->next){
        if (cur->free && cur->next->free){
            coalesce(cur);
            continue;
        }
        cur = cur->next;
    }
}

bool valid(void *b) {
    if ((void*)memPool > b || (void*)memPool + MEM_POOL_CAP < b)
        return false;

    memBlock *block = b - sizeof(memBlock);
    memBlock * cur = memHead;
    while (cur){
        if (cur == block)
            return true;
        cur = cur->next;
    }

    return false;
}

void freeBlock(memBlock *b) {
    b->free = true;
}

/* Free a given pointer to memory */
void Mem_free (void *b){
    if (memHead == NULL) {
        Mem_init();
        return;
    }

    if (!valid(b))
        return;

    memBlock *block = b - sizeof(memBlock);
    freeBlock(block);
    coalescer();
}
