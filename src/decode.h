#ifndef SIMPRISC_DECODE_H
#define SIMPRISC_DECODE_H

#include <stdbool.h>

#include "utils.h"
#include "../src/isa.h"

/*******************************************
 *
 * opcode       [31:27]     5 bits
 * I            [26]
 * rd           [25:22]     4 bits
 * rs1          [21:18]     4 bits
 * rs2          [17:14]     4 bits
 * imm field    [17:0]      18 bits
        * mode  [17:16]     2 bits
        * imm   [15:0]      16 bits

 * offset       [26:0]      27 bits
 *
 ********************************************/


typedef struct decoded_instr DecodedInstr;

struct decoded_instr
{
    Opcode opcode;
    uint8_t rd;
    uint8_t rs1;
    uint8_t rs2;
    uint32_t offset;
    uint8_t mode; // modifier bits
    uint16_t imm;
    bool hasImm;
};

DecodedInstr decode_instr(uint32_t raw_instr);

#endif //SIMPRISC_DECODE_H
