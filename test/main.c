#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../src/mem.h"

struct _test_mem_block {
    int size;
    char free;
    char *data;
};

int test_cnt = 0;
char * test_name = "";

void pass (){
    printf("%s, %d ... \e[32mPASS\e[0m\n", test_name, test_cnt);
    test_cnt ++;
}

void start (char * name){
    test_name = name;
    test_cnt = 0;
    printf("Starting tests: %s\n", name);
}

void print_stats(struct mem_stats * s) {
    printf("\e[31m\nMem stats:\n\e[0m");
    printf("\e[31mtotal blocks: %d\n\e[0m", s->total_blocks);
    printf("\e[31mactive blocks: %d\n\e[0m", s->active_blocks);
    printf("\e[31mbytes in use: %d\n\e[0m", s->bytes_in_use);
    printf("\n");
}

void test_mem_stat (){
    start("test_mem_stat");
    struct mem_stats *s = malloc(sizeof(struct mem_stats));

    // Fresh, no allocs
    mem_stat(s);

    int expected_total_blocks = 1;
    int expected_active_blocks = 0;
    int expected_bytes_in_use = sizeof(struct _test_mem_block);

    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    // Second stat, no allocs, expecting no change
    mem_stat(s);

    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    free(s);
}

void test_mem_alloc (){
    start("test_mem_alloc");
    struct mem_stats *s = malloc(sizeof(struct mem_stats));


    // one allocs
    int * a = mem_alloc(sizeof(int));
    mem_stat(s);

    int expected_total_blocks = 2;
    int expected_active_blocks = 1;
    int expected_bytes_in_use = sizeof(int) + (expected_total_blocks * sizeof(struct _test_mem_block));

    assert(a != NULL);
    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    // two allocs
    int * b = mem_alloc(sizeof(long));
    mem_stat(s);

    expected_total_blocks = 3;
    expected_active_blocks = 2;
    expected_bytes_in_use = sizeof(long) + sizeof(int) + (expected_total_blocks * sizeof(struct _test_mem_block));

    assert(b != NULL);
    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    // three allocs
    print_stats(s);
    void * c = mem_alloc(1000);
    mem_stat(s);

    expected_total_blocks = 4;
    expected_active_blocks = 3;
    expected_bytes_in_use = (2 * sizeof(int)) + 1000 + (expected_total_blocks * sizeof(struct _test_mem_block));

    assert(c != NULL);
    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();


    // overzised alloc
    void * d = mem_alloc(6000);
    mem_stat(s);

    expected_total_blocks = 4;
    expected_active_blocks = 3;
    expected_bytes_in_use = (2 * sizeof(int)) + 1000 + (expected_total_blocks * sizeof(struct _test_mem_block));

    assert(d == NULL);
    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    free(s);
}

int main() {
    test_mem_stat();
    test_mem_alloc();
    return 0;
}
