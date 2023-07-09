#include <stdint.h>

void do_smth_weird(void)
{
    uint64_t my_stack_var[2048];

    for (int i = 0; i < 2048; i++)
        my_stack_var[i] = 42;

    return;
}

void kmain(void)
{
    uint64_t a[1024];

    for(;;)
    {
        do_smth_weird();
    }
}