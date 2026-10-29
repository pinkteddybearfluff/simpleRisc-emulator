#include "../src/isa.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

const InstructionInfo* get_instr_info(const char* str)
{
    for (size_t i = 0; i < sizeof(instruction_table) / sizeof(instruction_table[0]); ++i)
    {
        if (!strcmp(str, instruction_table[i].mnemonic))
        {
            return &instruction_table[i];
        }
    }
    return NULL;
}

void print_instr_info(const InstructionInfo* instr)
{
    printf("%s ", instr->mnemonic);
    switch (instr->format)
    {
    case THREE_ADDRESS:
        puts("three address format");
        break;
    case THREE_ADDRESS_I_U:
        puts("three address format immediate u");
        break;
    case THREE_ADDRESS_I_H:
        puts("three address format immediate h");
        break;
    case TWO_ADDRESS:
        puts("two address format");

        break;
    case TWO_ADDRESS_I_U:
        puts("two address format immediate u");

        break;
    case TWO_ADDRESS_I_H:
        puts("two address format immediate h");

        break;
    case CMP_ADDRESS_FORMAT:
        puts("cmp address format");
        break;
    case ST_ADDRESS_FORMAT:
        puts("st address format");

        break;
    case ONE_ADDRESS_OFFSET:
        puts("one address offset");

        break;
    case ZERO_ADDRESS:
        puts("zero address format");

        break;
    }
}

uint8_t get_register_no(const char* str)
{
    for (uint8_t i = 0; i < 16; ++i)
    {
        if (!strcmp(str, registers_str[i]))
        {
            return i;
        }
    }
    if (!strcmp(str, "sp"))
    {
        return 14;
    }
    if (!strcmp(str, "ra"))
    {
        return 15;
    }
    return (uint8_t)-1;
}
