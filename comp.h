#pragma once
#include <stdint.h>
#include <stddef.h>

typedef enum {
    COMP_NONE = 0,
    COMP_RL,
    COMP_DELTA,
    COMP_SW,
    COMP_BP
} CompressionType;

CompressionType detectBestCompression(const uint8_t* data, size_t size);
uint8_t* compress(
    const uint8_t* data, 
    size_t size, 
    size_t* outputSize, 
    CompressionType* usedType
);
uint8_t* decompress(
    const uint8_t* data, 
    size_t size, 
    size_t* outputSize,
    CompressionType compType
);
