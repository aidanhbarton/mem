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

/* Initialize memory pool */
int Mem_init (){
    if (memHead)
        return 0;

    memHead = (memBlock *)memPool;
    memHead->size = MEM_POOL_CAP - sizeof(memBlock);
    memHead->free = true;
    memHead->next = NULL;
    memHead->data = memPool + sizeof(memBlock);

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
    newBlock->size = block->size - size - sizeof(memBlock);
    newBlock->free = true;
    newBlock->next = block->next;
    newBlock->data = block->data + size + sizeof(memBlock);

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
    printf("coalesce(%d->%d) ", b->size, b->next->size);

    b->size += b->next->size + sizeof(memBlock);
    b->next = b->next->next;
}

void coalescer (){
    printf("\ncoalescer: ");
    memBlock * cur = memHead;
    while (cur->next){
        if (cur->free && cur->next->free){
            coalesce(cur);
            continue;
        }
        cur = cur->next;
    }
    printf("\n\n");
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

/* Free a given pointer to memory */
void Mem_free (void *b){
    if (memHead == NULL)
        Mem_init();

    if (!valid(b))
        return;

    memBlock *block = b - sizeof(memBlock);
    block->free = true;
    coalescer();
}
