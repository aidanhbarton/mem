#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../src/mem.h"

int test_cnt = 0;
char * test_name = "";

void
pass (){
    printf("%s, %d ... \e[32mPASS\e[0m\n", test_name, test_cnt);
    test_cnt ++;
}

void
start (char * name){
    test_name = name;
    test_cnt = 0;
    printf("Starting tests: %s\n", name);
}

void
print_stats(struct mem_stats * s) {
    printf("\e[31m\nMem stats:\n\e[0m");
    printf("\e[31mtotal blocks: %d\n\e[0m", s->total_blocks);
    printf("\e[31mactive blocks: %d\n\e[0m", s->active_blocks);
    printf("\e[31mbytes in use: %d\n\e[0m", s->bytes_in_use);
    printf("\n");
}

void
test_mem_stat (){
    start("test_mem_stat");
    struct mem_stats *s = malloc(sizeof(struct mem_stats));

    // before init
    mem_stat(s);

    int expected_total_blocks = 0;
    int expected_active_blocks = 0;
    int expected_bytes_in_use = 0;

    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    // Second stat, after init
    assert(mem_init() == 0);
    mem_stat(s);

    expected_total_blocks = 1;
    expected_active_blocks = 0;
    expected_bytes_in_use = 4;

    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    free(s);

    // after destory
    mem_destroy();
    mem_stat(s);

    expected_total_blocks = 0;
    expected_active_blocks = 0;
    expected_bytes_in_use = 0;

    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();
}

void
test_mem_alloc (){
    start("test_mem_alloc");
    struct mem_stats *s = malloc(sizeof(struct mem_stats));

    // test pre-init
    int * a = mem_alloc(sizeof(int));
    mem_stat(s);

    int expected_total_blocks = 0;
    int expected_active_blocks = 0;
    int expected_bytes_in_use = 0;

    assert(a == NULL);
    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();


    mem_init();

    // one allocs
    int * b = mem_alloc(sizeof(int));
    mem_stat(s);

    expected_total_blocks = 2;
    expected_active_blocks = 1;
    expected_bytes_in_use = sizeof(int) + (expected_total_blocks * 4);

    assert(b != NULL);
    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    // two allocs
    int * c = mem_alloc(sizeof(long));
    mem_stat(s);

    expected_total_blocks = 3;
    expected_active_blocks = 2;
    expected_bytes_in_use = sizeof(long) + sizeof(int) + (expected_total_blocks * 4);

    assert(c != NULL);
    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    // three allocs
    void * d = mem_alloc(1000);
    mem_stat(s);

    expected_total_blocks = 4;
    expected_active_blocks = 3;
    expected_bytes_in_use = sizeof(long) + sizeof(int) + 1000 + (expected_total_blocks * 4);

    assert(d != NULL);
    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();


    // overzised alloc
    void * e = mem_alloc(6000);
    mem_stat(s);

    expected_total_blocks = 4;
    expected_active_blocks = 3;
    expected_bytes_in_use = sizeof(long) + sizeof(int) + 1000 + (expected_total_blocks * 4);

    assert(e == NULL);
    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    free(s);
    mem_destroy();
}

void
test_mem_free (){
    start("test_mem_free");
    struct mem_stats *s = malloc(sizeof(struct mem_stats));

    // Test pre-init
    mem_free(s);
    mem_stat(s);

    int expected_total_blocks = 0;
    int expected_active_blocks = 0;
    int expected_bytes_in_use = 0;

    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    // Test with invalid ptr
    mem_init();
    mem_free(s);
    mem_stat(s);

    expected_total_blocks = 1;
    expected_active_blocks = 0;
    expected_bytes_in_use = 4;

    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    // Test valid, without coalesce
    void * a = mem_alloc(1024);
    void * b = mem_alloc(1024);
    assert(a != NULL);
    assert(b != NULL);
    mem_free(a);
    mem_stat(s);

    expected_total_blocks = 3;
    expected_active_blocks = 1;
    expected_bytes_in_use = 1024 + (expected_total_blocks * 4);

    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    // Test invalid
    mem_free(s);
    mem_stat(s);

    expected_total_blocks = 3;
    expected_active_blocks = 1;
    expected_bytes_in_use = 1024 + (expected_total_blocks * 4);

    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();


    // Test with 1 coalesce
    mem_free(b);
    mem_stat(s);

    expected_total_blocks = 2;
    expected_active_blocks = 0;
    expected_bytes_in_use = (expected_total_blocks * 4);

    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();

    // Test invalid ptrs
    b = mem_alloc(1024);
    assert(b != NULL);
    mem_free(b + 10000);
    mem_free(b - 10000);
    mem_stat(s);

    expected_total_blocks = 2;
    expected_active_blocks = 1;
    expected_bytes_in_use = 1024 + (expected_total_blocks * 4);

    assert(s->total_blocks == expected_total_blocks);
    assert(s->active_blocks == expected_active_blocks);
    assert(s->bytes_in_use == expected_bytes_in_use);
    pass();


    mem_free(b);
    free(s);
    mem_destroy();
}

int main() {

    test_mem_stat();
    test_mem_alloc();
    test_mem_free();

    return 0;
}
