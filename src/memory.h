#ifndef SIMPRISC_MEMORY_H
#define SIMPRISC_MEMORY_H

#include <stdint.h>

#define MAX_MEM_SIZE 4096

//Follows little endian convention

typedef struct memory Memory;

struct memory
{
    uint32_t mem[MAX_MEM_SIZE];
};

//start and end address included
void print_memory_range(const Memory* mem, uint32_t start_addr, uint32_t end_addr);

void mem_init(Memory* mem);

uint8_t mem_read_byte(const Memory* mem, uint32_t addr);

// dword: 32 bits/4 bytes. Follows little endianess
uint32_t mem_read_dword(const Memory* mem, uint32_t addr);

void mem_write_byte(Memory* mem, uint32_t addr, uint8_t value);

// dword: 32 bits/4bytes. Follows little endianess
void mem_write_dword(Memory* mem, uint32_t addr, uint32_t value);

void load_program(Memory* mem, const char* file_path);

#endif //SIMPRISC_MEMORY_H
