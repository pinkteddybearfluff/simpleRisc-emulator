#include "../src/execute.h"

#include <assert.h>

#include "../src/utils.h"

static inline int32_t u32_to_d32(uint32_t value)
{
    return (value > INT32_MAX)
               ? -(int32_t)(UINT32_MAX - value) - 1
               : (int32_t)value;
}

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
        {
            int32_t multiplier;
            if (instr.hasImm)
                multiplier = u32_to_d32(immx);
            else
                multiplier = u32_to_d32(cpu->regs[instr.rs2]);
            cpu->regs[instr.rd] = (uint32_t)(u32_to_d32(cpu->regs[instr.rs1]) * multiplier);
            cpu->pc += 4;
            return true;
        }
    case OP_DIV:
        {
            int32_t divisor;
            if (instr.hasImm)
                divisor = u32_to_d32(immx);
            else
                divisor = u32_to_d32(cpu->regs[instr.rs2]);
            cpu->regs[instr.rd] = (uint32_t)(u32_to_d32(cpu->regs[instr.rs1]) / divisor);
            cpu->pc += 4;
            return true;
        }
    case OP_MOD:
        {
            int32_t divisor;
            if (instr.hasImm)
                divisor = u32_to_d32(immx);
            else
                divisor = u32_to_d32(cpu->regs[instr.rs2]);
            cpu->regs[instr.rd] = (uint32_t)(u32_to_d32(cpu->regs[instr.rs1]) % divisor);
            cpu->pc += 4;
            return true;
        }
    case OP_CMP:
        {
            const int32_t op1 = u32_to_d32(cpu->regs[instr.rs1]);
            int32_t op2;
            if (instr.hasImm)
                op2 = u32_to_d32(immx);
            else
                op2 = u32_to_d32(cpu->regs[instr.rs2]);
            printf("op1: %d op2: %d\n", op1, op2);
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
        {
            const bool sign_bit = (cpu->regs[instr.rs1] >> 31) & 1u;
            uint32_t shift_value;
            if (instr.hasImm)
            {
                shift_value = immx;
            }
            else
            {
                shift_value = cpu->regs[instr.rs2];
            }
            assert(shift_value<32u);

            if (sign_bit)
            {
                const uint32_t mask = ((1u << shift_value) - 1u) << (32u -
                    shift_value);
                cpu->regs[instr.rd] = (cpu->regs[instr.rs1] >> shift_value) | mask;
            }
            else
            {
                cpu->regs[instr.rd] = cpu->regs[instr.rs1] >> shift_value;
            }

            cpu->pc += 4;
            return true;
        }
    case OP_NOP:
        cpu->pc += 4;
        return true;
    case OP_LD:
        fprintf(stderr, "Address: %u + %d\n", cpu->regs[instr.rs1], immx);
        assert(
            (cpu->regs[instr.rs1] + immx< DATA_ADDR_H && cpu->regs[instr.rs1] + immx >= DATA_ADDR_L) ||
            (cpu->regs[instr.rs1] + immx< STACK_ADDR_H &&cpu->regs[instr.rs1] + immx >= STACK_ADDR_L));

        cpu->regs[instr.rd] = mem_read_dword(mem, cpu->regs[instr.rs1] + immx);
        cpu->pc += 4;
        return true;
    case OP_ST:
        assert(
            (cpu->regs[instr.rs1] + immx< DATA_ADDR_H && cpu->regs[instr.rs1] + immx >= DATA_ADDR_L) ||
            (cpu->regs[instr.rs1] + immx< STACK_ADDR_H &&cpu->regs[instr.rs1] + immx >= STACK_ADDR_L));
        // mem->mem[instr.rs1 + immx] = cpu->regs[instr.rd];
        mem_write_dword(mem, cpu->regs[instr.rs1] + immx, cpu->regs[instr.rd]);
        cpu->pc += 4;
        return true;
    case OP_BEQ:
        fprintf(stderr, "%d + %d = %d\n", cpu->pc, offset, cpu->pc + offset);
        assert(cpu->pc + offset <= TEXT_ADDR_H && cpu->pc + offset >= TEXT_ADDR_L);
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
