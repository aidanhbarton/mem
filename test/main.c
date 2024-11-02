#include "../src/mem.h"

int main() {
    int s = Mem_init();
    if (s != 0)
        return -1;

   Mem_print();
   int* x = (int *)Mem_alloc(sizeof(int));
   *x = 8 + 3;
   Mem_print();
   Mem_free(x + 6);
   Mem_print();
   Mem_free(x);
   Mem_print();
   return 0;
}
