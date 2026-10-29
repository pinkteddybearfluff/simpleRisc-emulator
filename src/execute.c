#include "../src/execute.h"

#include <assert.h>

#include "../src/utils.h"

bool execute(CPU* cpu, Memory* mem, const DecodedInstr instr)
{
    const uint32_t immx = sign_extend(instr.imm, instr.mode);

    //sign extend offset from 27 bits to 32 bits
    const uint32_t offset_mask = ((uint32_t)0x20 - 1u) << 27;
    const uint32_t offset = (instr.offset >> 26 & 1u)
                                ? (offset_mask | instr.offset) * 4
                                : instr.offset * 4;
    switch (instr.opcode)
    {
    case OP_ADD:
        if (instr.hasImm)
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] + immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] + cpu->regs[instr.rs2];
        cpu->pc += 4;
        return true;
    case OP_SUB:
        if (instr.hasImm)
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] - immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] - cpu->regs[instr.rs2];
        cpu->pc += 4;
        return true;
    case OP_MUL:
        if (instr.hasImm)
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] * immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] * cpu->regs[instr.rs2];
        cpu->pc += 4;
        return true;
    case OP_DIV:
        if (instr.hasImm)
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] / immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] / cpu->regs[instr.rs2];
        cpu->pc += 4;
        return true;
    case OP_MOD:
        if (instr.hasImm)
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] % immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] % cpu->regs[instr.rs2];
        cpu->pc += 4;
        return true;
    case OP_CMP:
        {
            int32_t op1 = (cpu->regs[instr.rs1] > INT32_MAX)
                              ? -(int32_t)(UINT32_MAX - cpu->regs[instr.rs1]) - 1
                              : (int32_t)cpu->regs[instr.rs1];
            int32_t op2;
            if (instr.hasImm)
                op2 = (immx > INT32_MAX)
                          ? -(int32_t)(UINT32_MAX - immx) - 1
                          : (int32_t)immx;
            else
                op2 = (cpu->regs[instr.rs2] > INT32_MAX)
                          ? -(int32_t)(UINT32_MAX - cpu->regs[instr.rs2]) - 1
                          : (int32_t)cpu->regs[instr.rs2];
            if (op1 == op2)
            {
                cpu->equal_flag = true;
                cpu->gt_flag = false;
            }
            else if (op1 < op2)
            {
                cpu->equal_flag = false;
                cpu->gt_flag = false;
            }
            else
            {
                cpu->equal_flag = false;
                cpu->gt_flag = true;
            }
        }
        cpu->pc += 4;
        return true;
    case OP_AND:
        if (instr.hasImm)
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] & immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] & cpu->regs[instr.rs2];
        cpu->pc += 4;
        return true;
    case OP_OR:
        if (instr.hasImm)
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] | immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] | cpu->regs[instr.rs2];
        cpu->pc += 4;
        return true;
    case OP_NOT:
        if (instr.hasImm)
            cpu->regs[instr.rd] = ~immx;
        else
            cpu->regs[instr.rd] = ~cpu->regs[instr.rs2];
        cpu->pc += 4;
        return true;
    case OP_MOV:
        if (instr.hasImm)
            cpu->regs[instr.rd] = immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs2];
        cpu->pc += 4;
        return true;
    case OP_LSL:
        if (instr.hasImm)
        {
            assert(immx < 32u);
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] << immx;
        }
        else
        {
            assert(cpu->regs[instr.rs2] < 32u);
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] << cpu->regs[instr.rs2];
        }
        cpu->pc += 4;
        return true;
    case OP_LSR:
        if (instr.hasImm)
        {
            assert(immx < 32);
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] >> immx;
        }
        else
        {
            assert(cpu->regs[instr.rs2] < 32u);
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] >> cpu->regs[instr.rs2];
        }
        cpu->pc += 4;
        return true;
    case OP_ASR:
        const bool sign_bit = (cpu->regs[instr.rs1] >> 31) & 1u;
        if (instr.hasImm)
        {
            assert(immx < 32u);
            const uint32_t mask = (bit_insert(0x00, immx, immx, 1) - 1) << (32u -
                immx);
            cpu->regs[instr.rd] = (cpu->regs[instr.rs1] >> immx) | mask;
        }
        else
        {
            assert(cpu->regs[instr.rs2] < 32u);
            const uint32_t mask = (bit_insert(0x00, cpu->regs[instr.rs2], cpu->regs[instr.rs2], 1) - 1) << (32u -
                cpu->regs[instr.rs2]);
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] >> cpu->regs[instr.rs2];
        }
        cpu->pc += 4;
        return true;
    case OP_NOP:
        cpu->pc += 4;
        return true;
    case OP_LD:
        assert(instr.rs1 + immx< DATA_ADDR_H && instr.rs1 >= DATA_ADDR_L);
        cpu->regs[instr.rd] = mem->mem[instr.rs1 + immx];
        return true;
    case OP_ST:
        assert(instr.rs1 + immx<= DATA_ADDR_H && instr.rs1 >= DATA_ADDR_L);
        mem->mem[instr.rs1 + immx] = cpu->regs[instr.rd];
        return true;
    case OP_BEQ:
        assert(cpu->pc + offset <= TEXT_ADDR_H && cpu->pc + offset>= TEXT_ADDR_L);
        if (cpu->equal_flag)cpu->pc += offset;
        else cpu->pc += 4;
        return true;
    case OP_BGT:
        assert(cpu->pc + offset <= TEXT_ADDR_H && cpu->pc + offset>= TEXT_ADDR_L);
        if (cpu->gt_flag)cpu->pc += offset;
        else cpu->pc += 4;
        return true;
    case OP_B:
        assert(cpu->pc + offset <= TEXT_ADDR_H && cpu->pc + offset>= TEXT_ADDR_L);
        cpu->pc += offset;
        return true;
    case OP_CALL:
        assert(cpu->pc + offset <= TEXT_ADDR_H && cpu->pc +offset>= TEXT_ADDR_L);
        assert(cpu->pc + 4 <= TEXT_ADDR_H && cpu->pc +4>= TEXT_ADDR_L);
        cpu->regs[RA_ID] = cpu->pc + 4;
        cpu->pc += offset;
        return true;
    case OP_RET:
        cpu->pc = cpu->regs[RA_ID];
        return true;
    case OP_HALT:
        cpu->halted = true;
        return true;
    }
}
