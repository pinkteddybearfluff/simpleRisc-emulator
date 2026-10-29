#ifndef SIMPRISC_CPU_H
#define SIMPRISC_CPU_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define REGISTER_COUNT 16u
#define RA_ID 15
#define SP_ID 14

#define MAX_CYCLES 10000u

//LOGICALLY DEFINED LAYOUT
#define TEXT_ADDR_L 0x0000u
#define TEXT_ADDR_H 0x07FFu
#define DATA_ADDR_L 0x0800u
#define DATA_ADDR_H 0x0BFFu
#define STACK_ADDR_L 0x0C00u
#define STACK_ADDR_H 0x0FFFu

typedef struct memory Memory;

typedef struct cpu CPU;

struct cpu
{
    uint32_t regs[REGISTER_COUNT];
    bool equal_flag;
    bool gt_flag;
    uint32_t ir;
    uint32_t pc;
    bool halted;
    size_t cycle;
};

void cpu_init(CPU* cpu);

void cpu_step(CPU* cpu, Memory* mem);
void cpu_run(CPU* cpu, Memory* mem);

//prints decimal and hexadecimal values within registers
void register_dump(const CPU* cpu);

void print_cpu_state(const CPU* cpu);

#endif //SIMPRISC_CPU_H
