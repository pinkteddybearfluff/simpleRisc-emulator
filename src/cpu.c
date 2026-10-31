#include "../src/cpu.h"
#include "../src/utils.h"
#include "../src/memory.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "execute.h"

void register_dump(const CPU* cpu)
{
    puts("\e[0;34m");
    for (size_t i = 0; i < REGISTER_COUNT / 2; ++i)
    {
        printf("r%zu:\t%10" PRId32 "\t", i, cpu->regs[i]);
        print_hex_u32(cpu->regs[i]);
        const size_t right_count = REGISTER_COUNT / 2 + i;
        printf("\t|\tr%zu:\t%" PRId32 "\t", right_count, cpu->regs[right_count]);
        print_hex_u32(cpu->regs[right_count]);

        putchar('\n');
    }
    puts("\e[0m");
}

void cpu_init(CPU* cpu)
{
    memset(cpu->regs, 0, sizeof(cpu->regs));
    cpu->equal_flag = false;
    cpu->gt_flag = false;
    cpu->halted = false;
    cpu->ir = 0x00u;
    cpu->pc = 0u;
    cpu->regs[SP_ID] = STACK_ADDR_H + 1; //sp | Downward growing stack
    cpu->cycle = 0u;
}

void cpu_step(CPU* cpu, Memory* mem)
{
    if (!cpu->halted && cpu->cycle < MAX_CYCLES)
    {
        if (cpu->pc > TEXT_ADDR_H || cpu->pc < TEXT_ADDR_L)
        {
            cpu->halted = true;
            fprintf(
                stderr,
                "\e[1;31m cpu error:\e[0m at instruction location: 0x%08" PRIx32
                ", pc trying to access non-text memory\n",
                cpu->pc);
            print_cpu_state(cpu);
            return;
        }
        cpu->ir = mem_read_dword(mem, cpu->pc);
        cpu_status_t cpu_state = execute(cpu, mem, decode_instr(cpu->ir));
        if (cpu_state != CPU_SUCCESS)
        {
            cpu->halted = true;
            fprintf(stderr, "\e[1;31m cpu error:\e[0m at instruction location: 0x%08" PRIx32", ", cpu->pc);
            {
                switch (cpu_state)
                {
                case CPU_ERROR_DIVISION_BY_ZERO:
                    fprintf(stderr, "division by zero\n");
                    break;
                case CPU_ERROR_MEMORY_ACCESS:
                    fprintf(stderr, "cannot read/write to requested memory location\n.");
                    break;
                case CPU_ERROR_INVALID_OPCODE:
                    fprintf(stderr, "invalid instruction opcode\n");
                    break;
                case CPU_ERROR_INVALID_REGISTER:
                    fprintf(stderr, "invalid register\n");
                    break;
                case CPU_ERROR_SHIFT_VALUE:
                    fprintf(stderr, "invalid shift value, shift value cannot exceed 32\n");
                    break;
                }
            }
            return;
        }
        ++cpu->cycle;
    }
}

void cpu_run(CPU* cpu, Memory* mem)
{
    while (!cpu->halted && cpu->cycle < MAX_CYCLES)
    {
        cpu_step(cpu, mem);
        // print_cpu_state(cpu);
    }
}

void print_cpu_state(const CPU* cpu)
{
    puts("\e[0;30m");
    puts("-------------------------Status-----------------------");

    puts("\e[0;31m");
    printf("Running : %s\n", cpu->halted ? "false" : "true");
    puts("\e[0;35m");
    fputs("PC: ", stdout);
    print_hex_u32(cpu->pc);
    fprintf(stdout, "\t%" PRIu32" ", cpu->pc);
    putchar('\n');
    fputs("IR: ",stdout);
    print_hex_u32(cpu->ir);
    putchar('\n');
    printf("\nCycle: %zu\n", cpu->cycle);
    printf("\nEquals flag: %s\tGreater Than flag: %s\n", cpu->equal_flag ? "true" : "false",
           cpu->gt_flag ? "true" : "false");
    register_dump(cpu);
}
