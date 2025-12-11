#include "bp.c"
#include "rl.c"
#include "sliding_window.c"
#include "delta.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

typedef enum {
    COMP_NONE = 0,
    COMP_RL,
    COMP_DELTA,
    COMP_SW,
    COMP_BP
} CompressionType;

CompressionType detectBestCompression(const uint8_t* data, size_t size) {
    if(size < 100) return COMP_NONE;

    int byteFreq[256] = {0};
    int maxFreq = 0;

    for(size_t i = 0; i < size; i++) {
        byteFreq[data[i]]++;
        if(byteFreq[data[i]] > maxFreq) {
            maxFreq = byteFreq[data[i]];
        }
    }

    int textBytes = 0;
    for(int i = 32; i < 127; i++) textBytes += byteFreq[i];
    textBytes += 
        byteFreq['\t'] + 
        byteFreq['\n'] + 
        byteFreq['\r'];
    if(textBytes * 100 / size > 70) {
        return COMP_BP;
    }

    int runCount = 0;
    for(size_t i = 1; i < size; i++) {
        if(data[i] == data[i-1]) runCount++;
    }
    if(runCount * 100 / size > 20) {
        return COMP_RL;
    }

    int smallDeltas = 0;
    for(size_t i = 1; i < size; i++) {
        int delta = abs((int)data[i] - (int)data[i-1]);
        if(delta < 16) smallDeltas++;
    }
    if(smallDeltas * 100 / size > 60) {
        return COMP_DELTA;
    }

    return COMP_SW;
}

/**
 * Compress
 */
uint8_t* compress(
    const uint8_t* data,
    size_t size,
    size_t* outputSize,
    CompressionType* usedType
) {
    if(size == 0) {
        *outputSize = 0;
        usedType = COMP_NONE;
        return NULL;
    }

    CompressionType bestType = detectBestCompression(data, size);
    *usedType = bestType;

    uint8_t* compressed = NULL;
    size_t compressedSize = 0;

    switch(bestType) {
        case COMP_RL:
            compressed = rlCompress(data, size, &compressedSize);
            break;
        case COMP_DELTA:
            compressed = deltaCompress(data, size, &compressedSize);
            break;
        case COMP_SW:
            compressed = swCompress(data, size, &compressedSize);
            break;
        case COMP_BP:
            BytePairCompressor* comp = bpCreate(256);
            countPairs(comp, data, size);
            compressed = bpCompress(comp, data, size, &compressedSize);
            bpDestroy(comp);
            break;
        case COMP_NONE:
        default:
            compressed = malloc(size);
            memcpy(compressed, data, size);
            compressedSize = size;
            break;
    }

    if(compressedSize >= size) {
        free(compressed);
        compressed = malloc(size);
        memcpy(compressed, data, size);
        compressedSize = size;
        *usedType = COMP_NONE;
    }

    *outputSize = compressedSize;
    return compressed;
}