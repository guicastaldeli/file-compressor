#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * Compress
 */
uint8_t* compress(
    const uint8_t* data, 
    size_t size, 
    size_t* outputSize
) {
    if(size == 0) {
        *outputSize = 0;
        return NULL;
    }
    
    uint8_t* outputBuffer = malloc(size + 1);
    outputBuffer[0] = data[0];

    for(size_t i = 1; i < size; i++) {
        int8_t delta = (int8_t)(data[i] - data[i-1]);
        outputBuffer[i] = (uint8_t)delta;
    }

    *outputSize = size;
    return outputBuffer;
}

/**
 * Decompress
 */
uint8_t* decompress(
    const uint8_t* data,
    size_t size,
    size_t* outputSize
) {
    if(size == 0) {
        *outputSize = 0;
        return NULL;
    }

    uint8_t* outputBuffer = malloc(size);
    outputBuffer[0] = data[0];

    for(size_t i = 1; i < size; i++) {
        int16_t val = (int16_t)outputBuffer[i-1] + (int8_t)data[i];
        outputBuffer[i] = (uint8_t)(val & 0xFF);
    }

    *outputSize = size;
    return outputBuffer;
}

