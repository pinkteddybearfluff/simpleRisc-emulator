#include "../src/utils.h"

#include <assert.h>


#define WIDTH_32 32u
#define WIDTH_16 16u

void print_bin_u32(const uint32_t value)
{
    for (size_t i = WIDTH_32 - 1; i <= WIDTH_32; --i)
    {
        putchar(value >> i & 1u ? '1' : '0');
        if (i % 8 == 0)
        {
            putchar(' ');
        }
    }
}

void print_bin_d32(const int32_t value)
{
    for (size_t i = WIDTH_32 - 1; i <= WIDTH_32; --i)
    {
        putchar((uint32_t)value >> i & 1u ? '1' : '0');
        if (i % 8 == 0)
        {
            putchar(' ');
        }
    }
}


// void print_hex_d32(const int32_t value)
// {
//     printf("%x", value);
// }

void print_bin_u16(const uint16_t value)
{
    for (size_t i = WIDTH_16 - 1; i <= WIDTH_16; --i)
    {
        putchar(value >> i & 1u ? '1' : '0');
        if (i % 8 == 0)
        {
            putchar(' ');
        }
    }
}

void print_bin_d16(const int16_t value)
{
    for (size_t i = WIDTH_16 - 1; i < WIDTH_16; --i)
    {
        putchar((uint16_t)value >> i & 1u ? '1' : '0');
        if (i % 8 == 0)
        {
            putchar(' ');
        }
    }
}


// void print_hex_d16(const int16_t value)
// {
//     printf("%" PRId16, value);
// }

uint32_t bit_extract(const uint32_t bits, const size_t high, const size_t low)
{
    assert(low<= high && high < WIDTH_32);
    size_t width = high - low + 1;
    if (width == WIDTH_32) return bits;
    uint32_t mask = (1u << width) - 1u;
    return bits >> low & mask;
}

// ? WARNING: FIX BUGS
uint32_t bit_insert(uint32_t bits, const size_t high, const size_t low, uint32_t value)
{
    assert(low<=high && high < WIDTH_32);
    size_t width = high - low + 1;
    if (width == WIDTH_32) return value;
    uint32_t mask = ~(((1u << width) - 1) << low);
    value = ((1u << width) - 1) & value;
    bits = bits & mask;
    bits = bits | (value << low);
    return bits;
}

//extend from 16 bits to 32 bits depending on mode
uint32_t sign_extend(const uint16_t bits, const ExMode mode)
{
    // assert(mode == 0 || mode == 1 || mode == 2);
    switch (mode)
    {
    case 0:
        if ((bits >> (WIDTH_16 - 1) & 1u) == 1u)
        {
            const uint32_t mask = 0xffffu << WIDTH_16;
            return mask | bits;
        }
        return bits;
    case 1:
        return bits;
    case 2:
        return (uint32_t)bits << WIDTH_16;
    default:
        return 0;
    }
}
