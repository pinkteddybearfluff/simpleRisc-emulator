#include "../src/memory.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void print_memory_range(const Memory* mem, const uint32_t start_addr, const uint32_t end_addr)
{
    const uint32_t start = (start_addr / 4) * 4;
    const uint32_t end = (end_addr / 4) * 4 + 3;

    puts("\e[0;35m");
    for (uint32_t i = start; i <= end; i += 4)
    {
        print_hex_u32(i);
        putchar(' ');
        print_hex_u8((uint8_t)mem->mem[i]);
        putchar(' ');
        print_hex_u8((uint8_t)mem->mem[i + 1]);
        putchar(' ');
        print_hex_u8((uint8_t)mem->mem[i + 2]);
        putchar(' ');
        print_hex_u8((uint8_t)mem->mem[i + 3]);
        putchar('\n');
    }
    puts("\e[0m");
}

void print_memory_content(const Memory* mem, const uint32_t addr)
{
    uint32_t value = mem_read_dword(mem, addr);
    printf("%8" PRIu32, value);
}

void print_memory_range_2(const Memory* mem, const uint32_t start_addr, const uint32_t end_addr)
{
    const uint32_t start = (start_addr / 4) * 4;
    const uint32_t end = (end_addr / 4) * 4 + 3;

    puts("\e[0;35m");
    for (uint32_t i = start; i <= end; i += 4)
    {
        printf("%" PRIu32 ": ", i);
        print_memory_content(mem, i);
        putchar('\n');
    }
    puts("\e[0m");
}

uint8_t mem_read_byte(const Memory* mem, const uint32_t addr)
{
    assert(addr < MAX_MEM_SIZE);
    return (uint8_t)mem->mem[addr];
}

uint32_t mem_read_dword(const Memory* mem, const uint32_t addr)
{
    assert(addr < addr +3 && addr+3 < MAX_MEM_SIZE);
    const uint32_t value = ((uint32_t)mem_read_byte(mem, addr)) |
        ((uint32_t)mem_read_byte(mem, addr + 1) << 8) |
        ((uint32_t)mem_read_byte(mem, addr + 2) << 16) |
        ((uint32_t)mem_read_byte(mem, addr + 3) << 24);
    return value;
}

void mem_write_byte(Memory* mem, const uint32_t addr, const uint8_t value)
{
    assert(addr< MAX_MEM_SIZE);
    mem->mem[addr] = value;
}

void mem_write_dword(Memory* mem, const uint32_t addr, const uint32_t value)
{
    assert(addr < addr+3 && addr + 3 < MAX_MEM_SIZE);
    mem_write_byte(mem, addr, (uint8_t)value);
    mem_write_byte(mem, addr + 1, (uint8_t)(value >> 8));
    mem_write_byte(mem, addr + 2, (uint8_t)(value >> 16));
    mem_write_byte(mem, addr + 3, (uint8_t)(value >> 24));
}

void mem_init(Memory* mem)
{
    memset(mem->mem, 0, sizeof(mem->mem));
}

void load_program(Memory* mem, const char* file_path)
{
    FILE* ist = fopen(file_path, "r");
    if (!ist)
    {
        perror("error: cannot open file");
        return;
    }

    char line[32];
    uint32_t ic = 0;
    while (fgets(line, 32, ist) != NULL)
    {
        const uint32_t dword = (uint32_t)strtoul(line, NULL, 16);
        mem_write_dword(mem, ic, dword);
        ic += 4;
    }

    fclose(ist);
}
