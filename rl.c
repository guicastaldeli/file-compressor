#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_RUN_LENGTH 255

uint8_t* compress(
    const uint8_t* data,
    size_t size,
    size_t* outputSize
) {
    if(size == 0) {
        *outputSize = 0;
        return NULL;
    }

    uint8_t* outputBuffer = malloc(size * 2);
    size_t outIdx = 0;

    size_t i = 0;
    while(i < size) {
        uint8_t current = data[i];
        size_t runLength = 1;

        while(
            i + runLength < size &&
            runLength < MAX_RUN_LENGTH &&
            data[i + runLength] == current
        ) {
            runLength++;
        }

        if(runLength > 3 || current == 0xFF) {
            outputBuffer[outIdx++] = 0xFF;
            outputBuffer[outIdx++] = runLength;
            outputBuffer[outIdx++] = current;
            i += runLength;
        } else {
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
    return outputBuffer;
}