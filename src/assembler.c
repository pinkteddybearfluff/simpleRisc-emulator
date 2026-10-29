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


int32_t get_branch_target(const struct labels label_table[], const char label[], const size_t lc)
{
    fprintf(stderr, "Inside get branch target\n");

    for (size_t i = 0; i < lc; ++i)
    {
        size_t length = strlen(label_table[i].label);
        if (!strncmp(label, label_table[i].label, length))
        {
            fprintf(stderr, "Label = %s Target = %d\n", label_table[i].label, label_table[i].target_addr);
            return (int32_t)label_table[i].target_addr;
        }
    }
    return -1;
}

bool str_to_imm(const char* str, uint16_t* imm)
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
        fprintf(stderr, "error: bad immediate string");
        return false;
    }
    const bool range_error = (errno == ERANGE);
    if (range_error)
    {
        fprintf(stderr, "error: range error occurred");
        return false;
    }
    if (integer > (int64_t)INT16_MAX || integer < (int64_t)INT16_MIN)
    {
        fprintf(stderr, "Error: immediate can only be of 2 bytes size\n");
        return false;
    }
    *imm = (uint16_t)((uint64_t)integer & 0xffffu);
    return true;
}

bool get_raw_instr(const InstructionInfo* instr_info, const char tokens[MAX_TOKEN_LIMIT][MAX_TOKEN_SIZE],
                   const bool has_label,
                   const size_t token_no,
                   const uint32_t ic,
                   const struct labels label_table[],
                   const size_t lc,
                   uint32_t* dword)
{
    *dword = 0;
    *dword = bit_insert(*dword, 31, 27, instr_info->opcode);
    switch (instr_info->format)
    {
    case THREE_ADDRESS:
    case THREE_ADDRESS_I_U:
    case THREE_ADDRESS_I_H:
        if ((has_label && token_no == 5) || (!has_label && token_no == 4))
        {
            //label: opcode rd rs1 rs2/imm
            //opcode rd rs1 rs2/imm
            const uint8_t reg_d = get_register_no(tokens[1 + has_label]);

            if (instr_info->opcode == OP_LD)
            {
                //ld rd imm rs1
                const uint8_t reg_s1 = get_register_no(tokens[3 + has_label]);
                uint16_t imm = 0;
                if (!str_to_imm(tokens[2 + has_label], &imm))
                {
                    fprintf(stderr, "error");
                    return false;
                }
                *dword = bit_insert(*dword, 25, 22, reg_d);
                *dword = bit_insert(*dword, 26, 26, 1u);
                *dword = bit_insert(*dword, 15, 0, imm);
                *dword = bit_insert(*dword, 21, 18, reg_s1);
                return true;
            }

            const uint8_t reg_s1 = get_register_no(tokens[2 + has_label]);

            *dword = bit_insert(*dword, 25, 22, reg_d)
                | bit_insert(0u, 21, 18, reg_s1);

            const uint8_t reg_s2 = get_register_no(tokens[3 + has_label]);
            if (reg_s2 < 16 && instr_info->format == THREE_ADDRESS) //R type instruction
            {
                *dword = bit_insert(*dword, 17, 14, reg_s2);
                return true;
            }
            if (reg_s2 < 16)
            {
                fprintf(stderr, "error: expected immediate");
                return false;
            }
            else
            {
                *dword = bit_insert(*dword, 26, 26, 1u);
                uint16_t imm = 0;
                if (!str_to_imm(tokens[3 + has_label], &imm))
                {
                    fprintf(stderr, "error:");
                    return false;
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
                return true;
            }
        }
        return false;
    case TWO_ADDRESS:
    case TWO_ADDRESS_I_U:
    case TWO_ADDRESS_I_H:
        if ((has_label && token_no == 4) || (!has_label && token_no == 3))
        {
            //label: opcode rd rs2/imm
            //opcode rd rs2/imm
            const uint8_t reg_d = get_register_no(tokens[1 + has_label]);

            *dword = bit_insert(*dword, 25, 22, reg_d);

            const uint8_t reg_s2 = get_register_no(tokens[2 + has_label]);
            fprintf(stderr, "%u %s\n", reg_s2, tokens[2 + has_label]);
            if (reg_s2 < 16 && instr_info->format == TWO_ADDRESS) //R type instruction
            {
                *dword = bit_insert(*dword, 17, 14, reg_s2);
                return true;
            }
            if (reg_s2 < 16)
            {
                fprintf(stderr, "error: expected immediate");
                return false;
            }
            else
            {
                *dword = bit_insert(*dword, 26, 26, 1u);
                uint16_t imm = 0;

                if (!str_to_imm(tokens[2 + has_label], &imm))
                {
                    fprintf(stderr, "error");
                    return false;
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
                return true;
            }
        }
        return false;
    case CMP_ADDRESS_FORMAT:
        if ((has_label && token_no == 4) || (!has_label && token_no == 3))
        {
            //label: cmp rs1 rs2/imm
            //cmp rs1, rs2/imm
            const uint8_t rs1 = get_register_no(tokens[1 + has_label]);
            *dword = bit_insert(*dword, 21, 18, rs1);

            const uint8_t rs2 = get_register_no(tokens[2 + has_label]);
            if (rs2 < 16) //R type instruction
            {
                *dword = bit_insert(*dword, 17, 14, rs2);
                return true;
            }
            //I type instruction
            *dword = bit_insert(*dword, 26, 26, 1u);
            uint16_t imm = 0;
            if (!str_to_imm(tokens[2 + has_label], &imm))
            {
                fprintf(stderr, "error");
                return false;
            }
            *dword = bit_insert(*dword, 15, 0, imm);
            return true;
        }
        return false;
    case ST_ADDRESS_FORMAT:
        if ((has_label && token_no == 5) || (!has_label && token_no == 4))
        {
            //label: st rd imm rs1
            //st rd imm rs1
            const uint8_t rsd = get_register_no(tokens[1 + has_label]);

            uint16_t imm = 0;
            if (!str_to_imm(tokens[2 + has_label], &imm))
            {
                fprintf(stderr, "error");
                return false;
            }
            const uint8_t rs1 = get_register_no(tokens[3 + has_label]);

            *dword = bit_insert(*dword, 25, 22, rsd);
            *dword = bit_insert(*dword, 26, 26, 1u);
            *dword = bit_insert(*dword, 15, 0, imm);
            *dword = bit_insert(*dword, 21, 18, rs1);
            return true;
        }
        return false;
    case ONE_ADDRESS_OFFSET:
        if ((has_label && token_no == 3) || (!has_label && token_no == 2))
        {
            //label: opcode target_label
            //opcode target_label
            const int32_t branch_target = get_branch_target(label_table, tokens[1 + has_label], lc);
            if (branch_target < 0)
            {
                return false;
            }
            const uint32_t offset = (uint32_t)branch_target - ic;
            *dword = bit_insert(*dword, 26, 0, offset);
            return true;
        }
        return false;
    case ZERO_ADDRESS:
        if ((has_label && token_no == 2) || (!has_label && token_no == 1))
        {
            //do nothing
            return true;
        }
        return false;
    }
    return true;
}


static size_t get_tokens(const char* line, char tokens[MAX_TOKEN_LIMIT][MAX_TOKEN_SIZE]);

static bool is_delimiter(const char ch)
{
    return ch == ' ' || ch == ',' || ch == '[' || ch == ']' || ch == '\n';
}

static void replace_comment(char line[])
{
    char* p = line;
    while (*p != '\0')
    {
        if (*p == '/' && *(p + 1) == '*')
        {
            *p = ' ';
            *(++p) = ' ';
            ++p;
            while (*p != '*' || *(p + 1) != '/')
                *(p++) = ' ';
            *p = ' ';
            *(++p) = ' ';
        }
        ++p;
    }
}

uint32_t generate_label_table(struct labels label_table[MAX_LABELS_LIMIT], FILE* ist)
{
    uint32_t ic = 0, lc = 0;
    char line[MAX_LINE_SIZE];
    while (fgets(line, MAX_LINE_SIZE, ist) != NULL)
    {
        replace_comment(line);
        char tokens[MAX_TOKEN_LIMIT][MAX_TOKEN_SIZE] = {"\0"};
        const size_t token_no = get_tokens(line, tokens);
        if (token_no)
        {
            const size_t l = strlen(tokens[0]);
            // token is a label
            if (tokens[0][l - 1] == ':')
            {
                //instruction lies on the same line as label
                strncpy(label_table[lc].label, tokens[0], l - 1);
                label_table[lc].label[l - 1] = '\0';
                if (tokens[1][0] != '\0')
                {
                    label_table[lc].target_addr = ic;
                    ++ic;
                }
                else
                {
                    label_table[lc].target_addr = ic;
                }
                ++lc;
            }
            else
            {
                ++ic;
            }
        }
    }

    rewind(ist);

    return lc;
}


bool write_hex_instr(const char mnemonic[], char tokens[MAX_TOKEN_LIMIT][MAX_TOKEN_SIZE],
                     const size_t token_no,
                     const uint32_t ic,
                     struct labels label_table[],
                     const uint32_t lc,
                     FILE* ost)
{
    const InstructionInfo* instr_info = get_instr_info(mnemonic);
    if (!instr_info)
    {
        fprintf(stderr, "SyntaxError: Invalid mnemonic\n");
        return false;
    }
    uint32_t dword;
    if (!get_raw_instr(instr_info, tokens,false, token_no, ic, label_table, lc, &dword))
    {
        return false;
    }
    fprintf(ost, "0x%08"PRIX32"\n", dword);
    return true;
}


bool write_prg_to_text(struct labels label_table[], const uint32_t lc, FILE* ist, FILE* ost)
{
    size_t line_no = 0;
    uint32_t ic = 0; //instruction counter
    char line[MAX_LINE_SIZE];
    while (fgets(line, MAX_LINE_SIZE, ist) != NULL)
    {
        ++line_no;
        replace_comment(line);
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
                    if (!write_hex_instr(tokens[1], tokens, token_no,
                                         ic, label_table, lc, ost))
                    {
                        fprintf(stderr, "error on line %zu", line_no);
                        return false;
                    }

                    ++ic;
                }
            }
            else
            {
                if (!write_hex_instr(tokens[0], tokens, token_no,
                                     ic, label_table, lc, ost))
                {
                    fprintf(stderr, "error on line %zu", line_no);
                    return false;
                }
                ++ic;
            }
        }
    }
    return true;
}

bool read_and_write_hex(const char in_file_path[])
{
    FILE* ist = fopen(in_file_path, "r"); //input stream
    if (!ist)
    {
        fprintf(stderr, "error opening file");
        return false;
    }
    FILE* ost = fopen("../tests/out.text", "w"); //output stream

    struct labels label_table[MAX_LABELS_LIMIT];

    const uint32_t lc = generate_label_table(label_table, ist);

    if (!write_prg_to_text(label_table, lc, ist, ost))
    {
        fprintf(stderr, "Unable to write to hex code to output file\n");
        fclose(ost);
        fclose(ist);
        return false;
    }

    fclose(ost);
    fclose(ist);
    return true;
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
