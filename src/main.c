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

    if (!read_and_write_hex("../tests/prg1.asm"))
    {
        return EXIT_FAILURE;
    }

    load_program(&mem, "../tests/out.text");

    // uint32_t shift_value = 2;
    // const uint32_t mask = ((1u << shift_value) - 1u) << (32u -
    //     shift_value);
    // // cpu->regs[instr.rd] = (cpu->regs[instr.rs1] >> shift_value) | mask;
    // print_bin_u32(mask);
    cpu_run(&cpu, &mem);

    print_memory_range(&mem, STACK_ADDR_H - 19, STACK_ADDR_H);

    return EXIT_SUCCESS;
}
