#pragma once
#include <stdint.h>
#include <stddef.h>

uint8_t* deltaCompress(
    const uint8_t* data, 
    size_t size, 
    size_t* outputSize
);
uint8_t* deltaDecompress(
    const uint8_t* data, 
    size_t size, 
    size_t* outputSize
);