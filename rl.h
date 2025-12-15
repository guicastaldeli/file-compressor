#pragma once
#include <stdint.h>
#include <stddef.h>

#define MAX_RUN_LENGTH 255

uint8_t* rlCompress(
    const uint8_t* data, 
    size_t size, 
    size_t* outputSize
);
uint8_t* rlDecompress(
    const uint8_t* data, 
    size_t size, 
    size_t* outputSize
);