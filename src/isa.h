#ifndef SIMPRISC_ISA_H
#define SIMPRISC_ISA_H

#include <stddef.h>
#include <stdint.h>

#define INSTRUCTION_WIDTH 32u

#define OPCODE_WIDTH 5u
#define OPCODE_SHIFT 27u

enum opcode : uint8_t
{
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,

    OP_CMP,

    OP_AND, OP_OR, OP_NOT,

    OP_MOV,

    OP_LSL, OP_LSR, OP_ASR,

    OP_NOP,

    OP_LD, OP_ST,

    OP_BEQ, OP_BGT, OP_B, OP_CALL, OP_RET,

    OP_HALT,

    OP_MAX_SIZE
};

static const char* opcodes_str[OP_MAX_SIZE] = {
    "add", "sub", "div", "mod", "cmp", "and", "or", "not", "mov", "lsl", "lsr", "asr", "nop", "halt", "ld", "st", "beq",
    "bgt", "b", "call", "ret"
};

typedef enum opcode Opcode;

typedef enum
{
    SIGN_EXTEND = 0u,
    ZERO_EXTEND = 1u,
    UPPER_16 = 2u,
} ExMode; //Extension Mode


typedef enum instruction_format
{
    THREE_ADDRESS, //3 address format register or immediate with default sign extension
    THREE_ADDRESS_I_U, //3 address immediate with zero extension
    THREE_ADDRESS_I_H, //3 address immediate with immediate occupying upper 16 bits of 32 bits value

    TWO_ADDRESS,
    TWO_ADDRESS_I_U,
    TWO_ADDRESS_I_H,

    CMP_ADDRESS_FORMAT,

    ST_ADDRESS_FORMAT,

    ONE_ADDRESS_OFFSET,

    ZERO_ADDRESS
} InstructionFormat;

typedef struct instruction_info
{
    const char* mnemonic;
    Opcode opcode;
    InstructionFormat format;
} InstructionInfo;

static const InstructionInfo instruction_table[] = {
    {"add", OP_ADD, THREE_ADDRESS},
    {"addu", OP_ADD, THREE_ADDRESS_I_U},
    {"addh", OP_ADD, THREE_ADDRESS_I_H},

    {"sub", OP_SUB, THREE_ADDRESS},
    {"subu", OP_SUB, THREE_ADDRESS_I_U},
    {"subh", OP_SUB, THREE_ADDRESS_I_H},

    {"mul", OP_MUL, THREE_ADDRESS},
    {"mulu", OP_MUL, THREE_ADDRESS_I_U},
    {"mulh", OP_MUL, THREE_ADDRESS_I_H},

    {"div", OP_DIV, THREE_ADDRESS},
    {"divu", OP_DIV, THREE_ADDRESS_I_U},
    {"divh", OP_DIV, THREE_ADDRESS_I_H},

    {"mod", OP_MOD, THREE_ADDRESS},
    {"modu", OP_MOD, THREE_ADDRESS_I_U},
    {"modh", OP_MOD, THREE_ADDRESS_I_H},

    {"cmp", OP_CMP, CMP_ADDRESS_FORMAT},

    {"and", OP_AND, THREE_ADDRESS},
    {"andu", OP_AND, THREE_ADDRESS_I_U},
    {"andh", OP_AND, THREE_ADDRESS_I_H},

    {"or", OP_OR, THREE_ADDRESS},
    {"oru", OP_OR, THREE_ADDRESS_I_U},
    {"orh", OP_OR, THREE_ADDRESS_I_H},

    {"not", OP_NOT, TWO_ADDRESS},
    {"notu", OP_NOT, TWO_ADDRESS_I_U},
    {"noth", OP_NOT, TWO_ADDRESS_I_H},

    {"mov", OP_MOV, TWO_ADDRESS},
    {"movu", OP_MOV, TWO_ADDRESS_I_U},
    {"movh", OP_MOV, TWO_ADDRESS_I_H},

    {"lsl", OP_LSL, THREE_ADDRESS},
    {"lslu", OP_LSL, THREE_ADDRESS},
    {"lslh", OP_LSL, THREE_ADDRESS},

    {"lsr", OP_LSR, THREE_ADDRESS},
    {"lsru", OP_LSR, THREE_ADDRESS},
    {"lsrh", OP_LSR, THREE_ADDRESS},

    {"asr", OP_ASR, THREE_ADDRESS},
    {"asru", OP_ASR, THREE_ADDRESS},
    {"asrh", OP_ASR, THREE_ADDRESS},

    {"nop", OP_NOP, ZERO_ADDRESS},

    {"halt", OP_HALT, ZERO_ADDRESS},

    {"ld", OP_LD, THREE_ADDRESS},

    {"st", OP_ST, ST_ADDRESS_FORMAT},
    {"beq", OP_BEQ, ONE_ADDRESS_OFFSET},
    {"bgt", OP_BGT, ONE_ADDRESS_OFFSET},
    {"b", OP_B, ONE_ADDRESS_OFFSET},
    {"call", OP_CALL, ONE_ADDRESS_OFFSET},

    {"ret", OP_RET, ZERO_ADDRESS},


};

static const char* registers_str[] = {
    //                                                                                       sp     ra
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

const InstructionInfo* get_instr_info(const char* str);

void print_instr_info(const InstructionInfo* instr);

uint8_t get_register_no(const char* str);

#endif //SIMPRISC_ISA_H
