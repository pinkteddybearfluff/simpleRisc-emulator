#include "../src/execute.h"

#include <assert.h>

#include "../src/utils.h"


static inline int32_t u32_to_d32(uint32_t value)
{
    return (value > INT32_MAX)
               ? -(int32_t)(UINT32_MAX - value) - 1
               : (int32_t)value;
}

cpu_status_t execute(CPU* cpu, Memory* mem, const DecodedInstr instr)
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
        return CPU_SUCCESS;
    case OP_SUB:
        if (instr.hasImm)
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] - immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] - cpu->regs[instr.rs2];
        cpu->pc += 4;
        return CPU_SUCCESS;
    case OP_MUL:
        {
            int32_t multiplier;
            if (instr.hasImm)
                multiplier = u32_to_d32(immx);
            else
                multiplier = u32_to_d32(cpu->regs[instr.rs2]);
            cpu->regs[instr.rd] = (uint32_t)(u32_to_d32(cpu->regs[instr.rs1]) * multiplier);
            cpu->pc += 4;
            return CPU_SUCCESS;
        }
    case OP_DIV:
        {
            int32_t divisor;
            if (instr.hasImm)
                divisor = u32_to_d32(immx);
            else
                divisor = u32_to_d32(cpu->regs[instr.rs2]);
            if (!divisor)
            {
                return CPU_ERROR_DIVISION_BY_ZERO;
            }
            cpu->regs[instr.rd] = (uint32_t)(u32_to_d32(cpu->regs[instr.rs1]) / divisor);
            cpu->pc += 4;
            return CPU_SUCCESS;
        }
    case OP_MOD:
        {
            int32_t divisor;
            if (instr.hasImm)
                divisor = u32_to_d32(immx);
            else
                divisor = u32_to_d32(cpu->regs[instr.rs2]);
            if (!divisor)
            {
                return CPU_ERROR_DIVISION_BY_ZERO;
            }
            cpu->regs[instr.rd] = (uint32_t)(u32_to_d32(cpu->regs[instr.rs1]) % divisor);
            cpu->pc += 4;
            return CPU_SUCCESS;
        }
    case OP_CMP:
        {
            const int32_t op1 = u32_to_d32(cpu->regs[instr.rs1]);
            int32_t op2;
            if (instr.hasImm)
                op2 = u32_to_d32(immx);
            else
                op2 = u32_to_d32(cpu->regs[instr.rs2]);
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
        return CPU_SUCCESS;
    case OP_AND:
        if (instr.hasImm)
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] & immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] & cpu->regs[instr.rs2];
        cpu->pc += 4;
        return CPU_SUCCESS;
    case OP_OR:
        if (instr.hasImm)
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] | immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] | cpu->regs[instr.rs2];
        cpu->pc += 4;
        return CPU_SUCCESS;
    case OP_NOT:
        if (instr.hasImm)
            cpu->regs[instr.rd] = ~immx;
        else
            cpu->regs[instr.rd] = ~cpu->regs[instr.rs2];
        cpu->pc += 4;
        return CPU_SUCCESS;
    case OP_MOV:
        if (instr.hasImm)
            cpu->regs[instr.rd] = immx;
        else
            cpu->regs[instr.rd] = cpu->regs[instr.rs2];
        cpu->pc += 4;
        return CPU_SUCCESS;
    case OP_LSL:
        if (instr.hasImm)
        {
            if (immx >= 32u)
            {
                return CPU_ERROR_SHIFT_VALUE;
            }
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] << immx;
        }
        else
        {
            if (cpu->regs[instr.rs2] >= 32u)
            {
                return CPU_ERROR_SHIFT_VALUE;
            }
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] << cpu->regs[instr.rs2];
        }
        cpu->pc += 4;
        return CPU_SUCCESS;
    case OP_LSR:
        if (instr.hasImm)
        {
            if (immx >= 32u)
            {
                return CPU_ERROR_SHIFT_VALUE;
            }
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] >> immx;
        }
        else
        {
            if (cpu->regs[instr.rs2] >= 32u)
            {
                return CPU_ERROR_SHIFT_VALUE;
            }
            cpu->regs[instr.rd] = cpu->regs[instr.rs1] >> cpu->regs[instr.rs2];
        }
        cpu->pc += 4;
        return CPU_SUCCESS;
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
            if (shift_value >= 32u)
            {
                return CPU_ERROR_SHIFT_VALUE;
            }

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
            return CPU_SUCCESS;
        }
    case OP_NOP:
        cpu->pc += 4;
        return CPU_SUCCESS;
    case OP_LD:
        if (
            (cpu->regs[instr.rs1] + immx < DATA_ADDR_L) ||
            (cpu->regs[instr.rs1] + immx > STACK_ADDR_H))
        {
            return CPU_ERROR_MEMORY_ACCESS;
        }

        cpu->regs[instr.rd] = mem_read_dword(mem, cpu->regs[instr.rs1] + immx);
        cpu->pc += 4;
        return CPU_SUCCESS;
    case OP_ST:
        if (
            (cpu->regs[instr.rs1] + immx < DATA_ADDR_L) ||
            (cpu->regs[instr.rs1] + immx > STACK_ADDR_H))
        {
            return CPU_ERROR_MEMORY_ACCESS;
        }
        mem_write_dword(mem, cpu->regs[instr.rs1] + immx, cpu->regs[instr.rd]);
        cpu->pc += 4;
        return CPU_SUCCESS;
    case OP_BEQ:
        if (cpu->equal_flag)cpu->pc += offset;
        else cpu->pc += 4;
        return CPU_SUCCESS;
    case OP_BGT:
        if (cpu->gt_flag)cpu->pc += offset;
        else cpu->pc += 4;
        return CPU_SUCCESS;
    case OP_B:
        cpu->pc += offset;
        return CPU_SUCCESS;
    case OP_CALL:
        cpu->regs[RA_ID] = cpu->pc + 4;
        cpu->pc += offset;
        return CPU_SUCCESS;
    case OP_RET:
        cpu->pc = cpu->regs[RA_ID];
        return CPU_SUCCESS;
    case OP_HALT:
        cpu->halted = true;
        return CPU_SUCCESS;
    default:
        return CPU_ERROR_INVALID_OPCODE;
    }
}
