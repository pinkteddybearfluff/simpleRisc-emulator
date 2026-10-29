#include <stdio.h>

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

    read_and_write_hex("../tests/prg1.asm");

    load_program(&mem, "../tests/out.text");

    print_memory_range(&mem, 0, 8);

    // print_cpu_state(&cpu);

    cpu_run(&cpu, &mem);

    // print_cpu_state(&cpu);

    // register_dump(&cpu);

    return 0;
}
