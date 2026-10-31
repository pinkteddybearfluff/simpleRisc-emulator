#include "../src/assembler.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "utils.h"
#include "../src/isa.h"

#define MAX_LABELS_LIMIT 2048u

struct labels
{
    char label[MAX_TOKEN_SIZE];
    uint32_t target_addr;
};

typedef enum asm_result
{
    ASM_SUCCESS = 0,
    ASM_ERROR_COMMENT_NOT_TERMINATED,
    ASM_ERROR_FILE_NOT_FOUND,
    ASM_ERROR_INVALID_INSTR,
    ASM_ERROR_EXPECTED_REGISTER,
    ASM_ERROR_EXPECTED_IMMEDIATE,
    ASM_ERROR_IMMEDIATE_RANGE,
    ASM_ERROR_BAD_IMMEDIATE_STRING,
    ASM_ERROR_OPERANDS_NOT_MATCHING,
    ASM_ERROR_LABEL_NOT_FOUND
} asm_result_t;

typedef enum
{
    LABEL_FOUND,
    LABEL_NOT_FOUND
} LabelLookupStatus;

static bool is_delimiter(const char ch)
{
    return ch == ' ' || ch == ',' || ch == '[' || ch == ']' || ch == '\n';
}

LabelLookupStatus get_branch_target(const struct labels label_table[], const char label[], const size_t lc,
                                    uint32_t* label_target_addr)
{
    for (size_t i = 0; i < lc; ++i)
    {
        if (!strcmp(label, label_table[i].label))
        {
            fprintf(stderr, "Label = %s Target = %d\n", label_table[i].label, label_table[i].target_addr);
            *label_target_addr = label_table[i].target_addr;
            return LABEL_FOUND;
        }
    }
    return LABEL_NOT_FOUND;
}

asm_result_t str_to_imm(const char* str, uint16_t* imm, char error_msg[])
{
    errno = 0;
    int64_t integer;
    char* end_ptr;
    if (*str == '0' && (*(str + 1) == 'x' || *(str + 1) == 'X'))
    {
        integer = strtol(str, &end_ptr, 16);
    }
    else if (*str == '0')
    {
        integer = strtol(str, &end_ptr, 8);
    }
    else
    {
        integer = strtol(str, &end_ptr, 10);
    }

    if (*end_ptr != '\0' || end_ptr == str)
    {
        sprintf(error_msg, "invalid immediate form '%s'", str);
        return ASM_ERROR_BAD_IMMEDIATE_STRING;
    }
    const bool range_error = (errno == ERANGE);
    if (integer > (int64_t)INT16_MAX || integer < (int64_t)INT16_MIN || range_error)
    {
        sprintf(error_msg, "immediate '%s' doesn't fit in 16 bit signed integer range", str);
        return ASM_ERROR_IMMEDIATE_RANGE;
    }
    *imm = (uint16_t)((uint64_t)integer & 0xffffu);
    return ASM_SUCCESS;
}

asm_result_t get_raw_instr(const InstructionInfo* instr_info, const char tokens[MAX_TOKEN_LIMIT][MAX_TOKEN_SIZE],
                           const bool has_label,
                           const size_t token_no,
                           const uint32_t ic,
                           const struct labels label_table[],
                           const size_t lc,
                           uint32_t* dword,
                           char error_msg[])
{
    *dword = 0;
    *dword = bit_insert(*dword, 31, 27, instr_info->opcode);
    switch (instr_info->format)
    {
    case THREE_ADDRESS:
    case THREE_ADDRESS_I_U:
    case THREE_ADDRESS_I_H:
        if (token_no == 4 + has_label)
        {
            //label: opcode rd rs1 rs2/imm
            //opcode rd rs1 rs2/imm
            const uint8_t reg_d = get_register_no(tokens[1 + has_label]);

            if (instr_info->opcode == OP_LD)
            {
                //ld rd imm rs1
                const uint8_t reg_s1 = get_register_no(tokens[3 + has_label]);
                uint16_t imm = 0;
                const asm_result_t imm_res_status = str_to_imm(tokens[2 + has_label], &imm, error_msg);
                if (imm_res_status != ASM_SUCCESS)
                {
                    return imm_res_status;
                }
                *dword = bit_insert(*dword, 25, 22, reg_d);
                *dword = bit_insert(*dword, 26, 26, 1u);
                *dword = bit_insert(*dword, 15, 0, imm);
                *dword = bit_insert(*dword, 21, 18, reg_s1);
                return ASM_SUCCESS;
            }

            const uint8_t reg_s1 = get_register_no(tokens[2 + has_label]);

            *dword = bit_insert(*dword, 25, 22, reg_d)
                | bit_insert(0u, 21, 18, reg_s1);

            const uint8_t reg_s2 = get_register_no(tokens[3 + has_label]);
            if (reg_s2 < 16 && instr_info->format == THREE_ADDRESS) //R type instruction
            {
                *dword = bit_insert(*dword, 17, 14, reg_s2);
                return ASM_SUCCESS;
            }
            if (reg_s2 < 16)
            {
                sprintf(error_msg, "expected immediate at '%s'", tokens[2 + has_label]);
                return ASM_ERROR_EXPECTED_IMMEDIATE;
            }
            else
            {
                *dword = bit_insert(*dword, 26, 26, 1u);
                uint16_t imm = 0;
                asm_result_t imm_status = str_to_imm(tokens[3 + has_label], &imm, error_msg);
                if (imm_status != ASM_SUCCESS)
                {
                    return imm_status;
                }
                *dword = bit_insert(*dword, 15, 0, imm);
                if (instr_info->format == THREE_ADDRESS)
                {
                    *dword = bit_insert(*dword, 17, 16, 0u);
                }
                else if (instr_info->format == THREE_ADDRESS_I_U)
                {
                    *dword = bit_insert(*dword, 17, 16, 1u);
                }
                else if (instr_info->format == THREE_ADDRESS_I_H)
                {
                    *dword = bit_insert(*dword, 17, 16, 2u);
                }
                return ASM_SUCCESS;
            }
        }
        if (instr_info->opcode == OP_LD && (token_no == 3 + has_label))
        {
            //label: ld rd rs1
            //ld rd rs1
            const uint8_t rd = get_register_no(tokens[1 + has_label]);

            const uint8_t rs1 = get_register_no(tokens[2 + has_label]);

            *dword = bit_insert(*dword, 25, 22, rd);
            *dword = bit_insert(*dword, 26, 26, 1u);
            *dword = bit_insert(*dword, 21, 18, rs1);
            return ASM_SUCCESS;
        }
        sprintf(error_msg, "operands do not match for '%s ... '", tokens[0 + has_label]);
        return ASM_ERROR_OPERANDS_NOT_MATCHING;
    case TWO_ADDRESS:
    case TWO_ADDRESS_I_U:
    case TWO_ADDRESS_I_H:
        if (token_no == 3 + has_label)
        {
            //label: opcode rd rs2/imm
            //opcode rd rs2/imm
            const uint8_t reg_d = get_register_no(tokens[1 + has_label]);

            *dword = bit_insert(*dword, 25, 22, reg_d);

            const uint8_t reg_s2 = get_register_no(tokens[2 + has_label]);
            if (reg_s2 < 16 && instr_info->format == TWO_ADDRESS) //R type instruction
            {
                *dword = bit_insert(*dword, 17, 14, reg_s2);
                return ASM_SUCCESS;
            }
            if (reg_s2 < 16)
            {
                sprintf(error_msg, "expected immediate at '%s'", tokens[2 + has_label]);
                return ASM_ERROR_EXPECTED_IMMEDIATE;
            }
            else
            {
                *dword = bit_insert(*dword, 26, 26, 1u);
                uint16_t imm = 0;

                asm_result_t imm_res_status = str_to_imm(tokens[2 + has_label], &imm, error_msg);
                if (imm_res_status != ASM_SUCCESS)
                {
                    return imm_res_status;
                }

                *dword = bit_insert(*dword, 15, 0, imm);
                if (instr_info->format == TWO_ADDRESS)
                {
                    *dword = bit_insert(*dword, 17, 16, 0u);
                }
                else if (instr_info->format == TWO_ADDRESS_I_U)
                {
                    *dword = bit_insert(*dword, 17, 16, 1u);
                }
                else if (instr_info->format == TWO_ADDRESS_I_H)
                {
                    *dword = bit_insert(*dword, 17, 16, 2u);
                }
                return ASM_SUCCESS;
            }
        }
        sprintf(error_msg, "operands do not match for '%s ... '", tokens[0 + has_label]);
        return ASM_ERROR_OPERANDS_NOT_MATCHING;
    case CMP_ADDRESS_FORMAT:
        if (token_no == 3 + has_label)
        {
            //label: cmp rs1 rs2/imm
            //cmp rs1, rs2/imm
            const uint8_t rs1 = get_register_no(tokens[1 + has_label]);
            *dword = bit_insert(*dword, 21, 18, rs1);

            const uint8_t rs2 = get_register_no(tokens[2 + has_label]);
            if (rs2 < 16) //R type instruction
            {
                *dword = bit_insert(*dword, 17, 14, rs2);
                return ASM_SUCCESS;
            }
            //I type instruction
            *dword = bit_insert(*dword, 26, 26, 1u);
            uint16_t imm = 0;
            const asm_result_t imm_res_status = str_to_imm(tokens[2 + has_label], &imm, error_msg);
            if (imm_res_status != ASM_SUCCESS)
            {
                return imm_res_status;
            }
            *dword = bit_insert(*dword, 15, 0, imm);
            return ASM_SUCCESS;
        }
        sprintf(error_msg, "operands do not match for '%s ... '", tokens[0 + has_label]);
        return ASM_ERROR_OPERANDS_NOT_MATCHING;
    case ST_ADDRESS_FORMAT:
        if (token_no == 4 + has_label)
        {
            //label: st rd imm rs1
            //st rd imm rs1
            const uint8_t rsd = get_register_no(tokens[1 + has_label]);

            uint16_t imm = 0;
            asm_result_t imm_res_status = str_to_imm(tokens[2 + has_label], &imm, error_msg);
            if (imm_res_status != ASM_SUCCESS)
            {
                return imm_res_status;
            }
            const uint8_t rs1 = get_register_no(tokens[3 + has_label]);

            *dword = bit_insert(*dword, 25, 22, rsd);
            *dword = bit_insert(*dword, 26, 26, 1u);
            *dword = bit_insert(*dword, 15, 0, imm);
            *dword = bit_insert(*dword, 21, 18, rs1);
            return ASM_SUCCESS;
        }
        if (token_no == 3 + has_label)
        {
            //label: st rsd rs1
            //st rsd rs1
            const uint8_t rsd = get_register_no(tokens[1 + has_label]);

            const uint8_t rs1 = get_register_no(tokens[2 + has_label]);

            *dword = bit_insert(*dword, 25, 22, rsd);
            *dword = bit_insert(*dword, 26, 26, 1u);
            *dword = bit_insert(*dword, 21, 18, rs1);
            return ASM_SUCCESS;
        }
        sprintf(error_msg, "operands do not match for '%s ... '", tokens[0 + has_label]);
        return ASM_ERROR_OPERANDS_NOT_MATCHING;
    case ONE_ADDRESS_OFFSET:
        if (token_no == 2 + has_label)
        {
            //label: opcode target_label
            //opcode target_label
            uint32_t label_target_addr = 0;
            LabelLookupStatus label_status = get_branch_target(label_table, tokens[1 + has_label], lc,
                                                               &label_target_addr);
            if (label_status == LABEL_NOT_FOUND)
            {
                sprintf(error_msg, "label '%s' not found", tokens[1 + has_label]);
                return ASM_ERROR_LABEL_NOT_FOUND;
            }
            const uint32_t offset = label_target_addr - ic;
            *dword = bit_insert(*dword, 26, 0, offset);
            return ASM_SUCCESS;
        }
        sprintf(error_msg, "operands do not match for '%s ... '", tokens[0 + has_label]);
        return ASM_ERROR_OPERANDS_NOT_MATCHING;
    case ZERO_ADDRESS:
        if (token_no == 1 + has_label)
        {
            //do nothing
            return ASM_SUCCESS;
        }
        sprintf(error_msg, "operands do not match for '%s ... '", tokens[0 + has_label]);
        return ASM_ERROR_OPERANDS_NOT_MATCHING;
    }
    return ASM_SUCCESS;
}

static size_t get_tokens(const char* line, char tokens[MAX_TOKEN_LIMIT][MAX_TOKEN_SIZE])
{
    const char* p = line;
    size_t k = 0;
    while (*p != '\0' && *p != EOF)
    {
        if (is_delimiter(*p))
        {
            ++p;
            continue;
        }
        size_t i = 0;
        char token[MAX_TOKEN_SIZE];
        while (!is_delimiter(*p) && *p != EOF && *p != '\0')
        {
            token[i] = *(p++);
            ++i;
        }

        token[i] = '\0';
        strncpy(tokens[k], token, MAX_TOKEN_SIZE);
        tokens[k][i] = '\0';

        tokens[k][MAX_TOKEN_SIZE - 1] = '\0';
        ++k;
    }
    return k;
}


static asm_result_t replace_comment(char line[])
{
    char* p = line;
    while (*p != '\0' && *p != EOF)
    {
        if (*p == '/' && *(p + 1) == '*')
        {
            *p = ' ';
            *(++p) = ' ';
            ++p;
            while (*p != '*' || *(p + 1) != '/')
            {
                if (*p == '\n' || *p == '\0')
                {
                    return ASM_ERROR_COMMENT_NOT_TERMINATED;
                }
                *(p++) = ' ';
            }
            *p = ' ';
            *(++p) = ' ';
        }
        ++p;
    }
    return ASM_SUCCESS;
}

//rewinds the input stream
asm_result_t generate_label_table(struct labels label_table[MAX_LABELS_LIMIT], uint32_t* lc, FILE* ist)
{
    size_t line_no = 0;
    *lc = 0;
    uint32_t ic = 0;
    char line[MAX_LINE_SIZE];
    while (fgets(line, MAX_LINE_SIZE, ist) != NULL)
    {
        ++line_no;
        if (replace_comment(line) != ASM_SUCCESS)
        {
            fprintf(stderr, "comment not terminated on line %zu\n", line_no);
            return ASM_ERROR_COMMENT_NOT_TERMINATED;
        }
        char tokens[MAX_TOKEN_LIMIT][MAX_TOKEN_SIZE] = {"\0"};
        const size_t token_no = get_tokens(line, tokens);
        if (token_no)
        {
            const size_t l = strlen(tokens[0]);
            // token is a label
            if (tokens[0][l - 1] == ':')
            {
                //instruction lies on the same line as label
                strncpy(label_table[*lc].label, tokens[0], l - 1);
                label_table[*lc].label[l - 1] = '\0';
                if (tokens[1][0] != '\0')
                {
                    label_table[*lc].target_addr = ic;
                    ++ic;
                }
                else
                {
                    label_table[*lc].target_addr = ic;
                }
                ++*lc;
            }
            else
            {
                ++ic;
            }
        }
    }

    rewind(ist);
    return ASM_SUCCESS;
}


asm_result_t write_hex_instr(const char mnemonic[], char tokens[MAX_TOKEN_LIMIT][MAX_TOKEN_SIZE],
                             const size_t token_no,
                             const uint32_t ic,
                             bool has_label,
                             struct labels label_table[],
                             const uint32_t lc,
                             FILE* ost,
                             char error_msg[])
{
    const InstructionInfo* instr_info = get_instr_info(mnemonic);
    if (!instr_info)
    {
        sprintf(error_msg, "invalid instruction '%s'", mnemonic);
        return ASM_ERROR_INVALID_INSTR;
    }
    uint32_t dword;
    const asm_result_t raw_instr_status =
        get_raw_instr(instr_info, tokens, has_label, token_no, ic, label_table, lc, &dword, error_msg);
    if (raw_instr_status != ASM_SUCCESS)
    {
        return raw_instr_status;
    }
    fprintf(ost, "0x%08" PRIx32 "\n", dword);
    return ASM_SUCCESS;
}


bool write_prg_to_text(struct labels label_table[], const uint32_t lc, FILE* ist, FILE* ost)
{
    size_t line_no = 0;
    uint32_t ic = 0; //instruction counter
    char line[MAX_LINE_SIZE];
    while (fgets(line, MAX_LINE_SIZE, ist) != NULL)
    {
        ++line_no;
        if (replace_comment(line) != ASM_SUCCESS)
        {
            fprintf(stderr, "error: comment not terminated on line %zu\n", line_no);
            return ASM_ERROR_COMMENT_NOT_TERMINATED;
        }
        char tokens[MAX_TOKEN_LIMIT][MAX_TOKEN_SIZE] = {"\0"};
        const size_t token_no = get_tokens(line, tokens);
        if (token_no)
        {
            const size_t l = strlen(tokens[0]);
            // token is a label
            if (tokens[0][l - 1] == ':')
            {
                //instruction lies on the same line as label
                if (tokens[1][0] != '\0')
                {
                    char error_msg[256];
                    const asm_result_t instr_write_status = write_hex_instr(tokens[1], tokens, token_no,
                                                                            ic,true, label_table, lc, ost, error_msg);
                    if (instr_write_status != ASM_SUCCESS)
                    {
                        switch (instr_write_status)
                        {
                        case ASM_ERROR_COMMENT_NOT_TERMINATED:
                        case ASM_ERROR_INVALID_INSTR:
                        case ASM_ERROR_EXPECTED_REGISTER:
                        case ASM_ERROR_EXPECTED_IMMEDIATE:
                        case ASM_ERROR_IMMEDIATE_RANGE:
                        case ASM_ERROR_BAD_IMMEDIATE_STRING:
                        case ASM_ERROR_OPERANDS_NOT_MATCHING:
                        case ASM_ERROR_LABEL_NOT_FOUND:
                            fprintf(stderr, "error: %s, line: %zu\n", error_msg, line_no);
                        }
                        return instr_write_status;
                    }

                    ++ic;
                }
            }
            else
            {
                char error_msg[256];
                const asm_result_t instr_write_status = write_hex_instr(tokens[0], tokens, token_no,
                                                                        ic, false, label_table, lc, ost, error_msg);
                if (instr_write_status != ASM_SUCCESS)
                {
                    switch (instr_write_status)
                    {
                    case ASM_ERROR_COMMENT_NOT_TERMINATED:
                    case ASM_ERROR_INVALID_INSTR:
                    case ASM_ERROR_EXPECTED_REGISTER:
                    case ASM_ERROR_EXPECTED_IMMEDIATE:
                    case ASM_ERROR_IMMEDIATE_RANGE:
                    case ASM_ERROR_BAD_IMMEDIATE_STRING:
                    case ASM_ERROR_OPERANDS_NOT_MATCHING:
                    case ASM_ERROR_LABEL_NOT_FOUND:
                        fprintf(stderr, "error: %s, line: %zu\n", error_msg, line_no);
                    }
                    return instr_write_status;
                }
                ++ic;
            }
        }
    }
    return ASM_SUCCESS;
}

bool read_and_write_hex(const char file_path[])
{
    FILE* ist = fopen(file_path, "r"); //input stream
    if (!ist)
    {
        fprintf(stderr, "error: file %s not found", file_path);
        return false;
    }
    FILE* ost = fopen("../tests/out.text", "w"); //output stream

    struct labels label_table[MAX_LABELS_LIMIT];

    uint32_t lc;
    const asm_result_t lt_result = generate_label_table(label_table, &lc, ist);
    if (lt_result != ASM_SUCCESS)
    {
        fclose(ost);
        fclose(ist);
        return false;
    }

    asm_result_t file_write_status = write_prg_to_text(label_table, lc, ist, ost);
    if (file_write_status != ASM_SUCCESS)
    {
        fprintf(stderr, "Hex code not written to output file\n");
        fclose(ost);
        fclose(ist);
        return false;
    }

    fclose(ost);
    fclose(ist);
    return true;
}

