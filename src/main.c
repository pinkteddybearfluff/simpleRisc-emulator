#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "assembler.h"
#include "../src/utils.h"
#include "../src/cpu.h"

int main(void)
{
    CPU cpu;
    Memory mem;

    cpu_init(&cpu);
    mem_init(&mem);

    if (!read_and_write_hex("../tests/fill_n_rev_arr.asm"))
    {
        return EXIT_FAILURE;
    }
    load_program(&mem, "../tests/out.bin");

    cpu_run(&cpu, &mem);

    print_cpu_state(&cpu);

    print_memory_range_2(&mem, DATA_ADDR_L, DATA_ADDR_L + 10 * 4);

    return EXIT_SUCCESS;
}
