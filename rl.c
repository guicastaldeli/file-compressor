#include "rl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * Compress
 */
uint8_t* rlCompress(const uint8_t* data, size_t size, size_t* outputSize) {
    if(size == 0) {
        *outputSize = 0;
        return NULL;
    }

    printf("DEBUG RL: Compressing %zu bytes\n", size);
    
    uint8_t* outputBuffer = (uint8_t*)malloc(size * 2);
    if (!outputBuffer) {
        printf("ERROR RL: malloc failed for size: %zu\n", size * 2);
        *outputSize = 0;
        return NULL;
    }
    
    size_t outIdx = 0;
    size_t i = 0;
    
    while(i < size) {
        uint8_t current = data[i];
        size_t runLength = 1;
        
        while(i + runLength < size && 
              runLength < MAX_RUN_LENGTH && 
              data[i + runLength] == current) {
            runLength++;
        }
        
        if(runLength > 3 || current == 0xFF) {
            if (outIdx + 3 >= size * 2) {
                printf("ERROR RL: Output buffer overflow\n");
                free(outputBuffer);
                *outputSize = 0;
                return NULL;
            }
            outputBuffer[outIdx++] = 0xFF;
            outputBuffer[outIdx++] = (uint8_t)runLength;
            outputBuffer[outIdx++] = current;
            i += runLength;
        } else {
            if (outIdx + runLength >= size * 2) {
                printf("ERROR RL: Output buffer overflow\n");
                free(outputBuffer);
                *outputSize = 0;
                return NULL;
            }
            for(size_t j = 0; j < runLength; j++) {
                if(current == 0xFF) {
                    outputBuffer[outIdx++] = 0xFF;
                    outputBuffer[outIdx++] = 0x01;
                }
                outputBuffer[outIdx++] = current;
            }
            i += runLength;
        }
    }
    
    *outputSize = outIdx;
    
    uint8_t* finalBuffer = (uint8_t*)realloc(outputBuffer, outIdx);
    if (!finalBuffer && outIdx > 0) {
        finalBuffer = outputBuffer;
    }
    
    printf("DEBUG RL: Compressed %zu -> %zu bytes\n", size, outIdx);
    return finalBuffer ? finalBuffer : outputBuffer;
}

/**
 * Decompress
 */
uint8_t* rlDecompress(
    const uint8_t* data,
    size_t size,
    size_t* outputSize
) {
    if(size == 0) {
        *outputSize = 0;
        return NULL;
    }

    uint8_t* outputBuffer = malloc(size * 256);
    size_t outIdx = 0;

    size_t i = 0;
    while(i < size) {
        if(data[i] == 0xFF && i + 2 < size) {
            uint8_t runLength = data[i+1];
            uint8_t val = data[i+2];
            for(int j = 0; j < runLength; j++) {
                outputBuffer[outIdx++] = val;
            }

            i += 3;
        } else {
            outputBuffer[outIdx++] = data[i++];
        }
    }

    *outputSize = outIdx;
    return outputBuffer;
}