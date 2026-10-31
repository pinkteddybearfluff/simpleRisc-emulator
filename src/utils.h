#ifndef SIMPRISC_UTILS_H
#define SIMPRISC_UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "../src/isa.h"

static const char* bold_black = "\x1b[1;30m";
static const char* bold_red = "\x1b[1;31m";
static const char* bold_green = "\x1b[1;32m";
static const char* bold_yellow = "\x1b[1;33m";
static const char* bold_blue = "\x1b[1;34m";
static const char* bold_purple = "\x1b[1;35m";
static const char* bold_cyan = "\x1b[1;36m";
static const char* bold_white = "\x1b[1;37m";

static const char* black = "\x1b[0;30m";
static const char* red = "\x1b[0;31m";
static const char* green = "\x1b[0;32m";
static const char* yellow = "\x1b[0;33m";
static const char* blue = "\x1b[0;34m";
static const char* purple = "\x1b[0;35m";
static const char* cyan = "\x1b[0;36m";
static const char* white = "\x1b[0;37m";

static const char* rst_str = "\x1b[0m";


void print_bin_d16(int16_t value);
void print_bin_u16(uint16_t value);
// void print_hex_d16(const int16_t value);

static inline void print_hex_u16(const uint16_t value)
{
    printf("0x%04" PRIX16, value);
}

void print_bin_d32(int32_t value);
void print_bin_u32(uint32_t value);
// void print_hex_d32(const int32_t value);

static inline void print_hex_u8(const uint8_t value)
{
    printf("%02" PRIX8, value);
}

static inline void print_hex_u32(const uint32_t value)
{
    printf("0x%08" PRIX32, value);
}

//high and low are included
uint32_t bit_extract(uint32_t bits, size_t high, size_t low);
//high and low are included
uint32_t bit_insert(uint32_t bits, size_t high, size_t low, uint32_t value);

uint32_t sign_extend(uint16_t bits, ExMode mode);

#endif //SIMPRISC_UTILS_H
