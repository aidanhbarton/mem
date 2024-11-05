#include <stdint.h>

#include <stddef.h>     // for: NULL
#include <stdio.h>      // for: printf
#include <sys/mman.h>   // for: mmap
#include <unistd.h>     // for getpagesize

struct mem_stats {
    int total_blocks;
    int active_blocks;
    int bytes_in_use;
};

struct _block {
    uint32_t meta;
    char data;
};

typedef struct _block _block;

uint32_t _total_blocks = 0;
uint32_t _active_blocks = 0;
uint32_t _bytes_in_use = 0;

void * _pool = NULL;
uint32_t _page_size = 0;


void
_parse_meta (_block * b, uint32_t *size, uint8_t *active){
    //printf("reading: %p meta: 0x%08x\n", b, b->meta);
    *size = b->meta >> 1;
    *active = b->meta & 0x01;

    //printf("\tresult: %d, %d\n", *size, *active);
}


void
_write_meta (_block * b, uint32_t size, uint8_t active){
    uint32_t meta = size << 1;
    meta = meta | active;
    b->meta = meta;

    //printf("writing: b: %p, 0x%08x\n", b, b->meta);
}


void
_block_init (_block * b, uint32_t size){
    //printf("new block: %p, %d\n", b, size);
    //printf("\tloc: %d/%d\n", (int) ((void*)b - _pool), _page_size);

    _write_meta(b, size - sizeof(uint32_t), 0x00);
    _total_blocks ++;
    _bytes_in_use += sizeof(uint32_t);
}


void
_print (){
    _block * cur = (_block *) _pool;
    uint32_t size;
    uint8_t active;
    while (cur < (_block*) (_pool + _page_size)){
        _parse_meta(cur, &size, &active);
        printf("[%d](%d)->", size, active);

        cur = (_block *) ((void*)&cur->data + size);
    }
    printf("(END)\n");
}


void
_coalesce (_block * b, uint32_t size){
    void *n = ((void * )&b->data + size);
    if (n > (void *)_pool + _page_size)
        return;

    uint32_t n_size; uint8_t n_actv;
    _parse_meta((_block *)n, &n_size, &n_actv);

    if (n_actv == 1)
        return;

    _write_meta(b, size + n_size + sizeof(uint32_t), 0);
    _total_blocks --;
    _bytes_in_use -= sizeof(uint32_t);
}


void
_free (_block * b){
    uint32_t size = b->meta >> 1;
    _write_meta(b, size, 0);
    _active_blocks --;
    _bytes_in_use -= size;

    _coalesce(b, size);
}


void
_allocate (_block * b){
    uint32_t size = b->meta >> 1;
    _write_meta(b, size, 1);
    _active_blocks ++;
    _bytes_in_use += size;
}


void *
_fit (_block * b, uint32_t alloc_size){
    uint32_t b_size; uint8_t b_actv;
    _parse_meta(b, &b_size, &b_actv);

    if (b_size == alloc_size){
        _allocate(b);
        return &b->data;
    }

    if (b_size - alloc_size <= sizeof(uint32_t) + 1) {
        _allocate(b);
        return &b->data;
    }

    _block *new_b = (_block *) ((void *)b + alloc_size + sizeof(uint32_t));
    _block_init(new_b, b_size - alloc_size);

    _write_meta(b, alloc_size, 0);
    _allocate(b);
    return &b->data;

}


int
_valid (void *b){
    if (b < (void *) _pool)
        return -1;

    if (b > (void *) _pool + _page_size)
        return -1;

    void * found = NULL;
    uint32_t cur_size; uint8_t cur_actv;
    _block * cur = (void *) _pool;
    while (cur < (_block*) (_pool + _page_size)){
        _parse_meta(cur, &cur_size, &cur_actv);
        if (cur == b){
            found = cur;
            break;
        }

        cur = (_block *) ((void*)&cur->data + cur_size);
    }

    if (found == NULL)
        return -1;

    return 0;
}


int
mem_init (){
    _page_size = getpagesize();

    _pool = mmap(   NULL, 
                    _page_size, 
                    PROT_READ | PROT_WRITE, 
                    MAP_ANONYMOUS | MAP_PRIVATE, 
                    -1, 
                    0);

    if (_pool == (void *) -1) {
        perror("mmap");
        return -1;
    }

    _block_init(    _pool,
                    _page_size);

    return 0;
}


int
mem_destroy (){
    _total_blocks = 0;
    _active_blocks = 0;
    _bytes_in_use = 0;

    if (munmap(_pool, _page_size) == -1) {
        perror("munmap");
        return -1;
    }

    _pool = NULL;
    return 0;
}


void *
mem_alloc (uint32_t size){
    if (_pool == NULL)
        return NULL;

    void * addr = NULL;

    if (_page_size - _bytes_in_use < size)
        return addr;

    _block * cur = _pool;
    while (cur < (_block*) (_pool + _page_size)){
        uint32_t cur_size; uint8_t cur_actv;
        _parse_meta(cur, &cur_size, &cur_actv);

        if (cur_actv == 0 && size <= cur_size){
            addr = _fit(cur, size);
            break;
        }

        cur = (_block *) ((void*) &cur->data + cur_size);
    }

    return addr;
}


void
mem_free (void *b){
    if (_pool == NULL || b == NULL)
        return;

    b -= sizeof(uint32_t);

    if (_valid(b) == -1)
        return;

    _free((_block *) b);
}


void
mem_stat (struct mem_stats * s){
    s->total_blocks = _total_blocks;
    s->active_blocks = _active_blocks;
    s->bytes_in_use = _bytes_in_use;
}
