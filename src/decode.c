#include "../src/decode.h"

DecodedInstr decode_instr(const uint32_t raw_instr)
{
    const DecodedInstr decoded_instr = {
        .opcode = (uint8_t)bit_extract(raw_instr, 31, 27),
        .rd = (uint8_t)bit_extract(raw_instr, 25, 22),
        .rs1 = (uint8_t)bit_extract(raw_instr, 21, 18),
        .rs2 = (uint8_t)bit_extract(raw_instr, 17, 14),
        .offset = bit_extract(raw_instr, 26, 0),
        .mode = (uint8_t)bit_extract(raw_instr, 17, 16),
        .imm = (uint16_t)bit_extract(raw_instr, 15, 0),
        .hasImm = bit_extract(raw_instr, 26, 26) & 1,

    };
    return decoded_instr;
}
