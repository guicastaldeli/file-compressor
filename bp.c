#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    uint16_t pair[2];
    uint16_t token;
    int count;
} BytePair;

typedef struct {
    BytePair* pairs;
    int pairCount;
    int maxPairs;
    uint8_t* dict;
    int dictSize;
} BytePairCompressor;

/**
 * Create
 */
BytePairCompressor* bpCreate(int maxVocabSize) {
    BytePairCompressor* comp = malloc(sizeof(BytePairCompressor));
    comp->pairs = malloc(sizeof(BytePair) * maxVocabSize);
    comp->pairCount = 0;
    comp->maxPairs = maxVocabSize;
    comp->dict = 0;
    return comp;
}

/**
 * Destroy
 */
void bpDestroy(BytePairCompressor* comp) {
    if(comp) {
        free(comp->pairs);
        free(comp->dict);
        free(comp);
    }
}

void countPairs(
    BytePairCompressor* comp,
    const uint8_t* data,
    size_t size
) {
    int pairCounts[65536] = {0};

    for(size_t i = 0; i < size; i++) {
        uint16_t pair = (data[i] << 8) | data[i+1];
        pairCounts[pair]++;
    }
    for(int i = 0; i < 65536 && comp->pairCount < comp->maxPairs; i++) {
        if(pairCounts[i] > 10) {
            comp->pairs[comp->pairCount].pair[0] = i >> 8;
            comp->pairs[comp->pairCount].pair[1] = i & 0xFF;
            comp->pairs[comp->pairCount].token = 256 + comp->pairCount;
            comp->pairs[comp->pairCount].count = pairCounts[i];
            comp->pairCount++;
        }
    }
}

/**
 * Compress
 */
uint8_t* bpCompress(
    BytePairCompressor* comp,
    const uint8_t* data,
    size_t size,
    size_t* outputSize
) {
    if(size < 2) {
        uint8_t* output = malloc(size);
        memcpy(output, data, size);
        *outputSize = size;
        return output;
    }

    uint8_t* outputBuffer = malloc(size * 2);
    size_t outIdx = 0;

    for(size_t i = 0; i < size;) {
        if(i < size - 1) {
            uint8_t b1 = data[i];
            uint8_t b2 = data[i+1];

            int found = 0;
            for(int j = 0; j < comp->pairCount; j++) {
                if(comp->pairs[j].pair[0] == b1 && comp->pairs[j].pair[1] == b2) {
                    outputBuffer[outIdx++] = 0xFF;
                    outputBuffer[outIdx++] = comp->pairs[j].token - 256;
                    i += 2;
                    found = 1;
                    break;
                }
            }
            if(!found) {
                outputBuffer[outIdx++] = data[i++];
            }
        } else {
            outputBuffer[outIdx++] = data[i++];
        }
    }

    *outputSize = outIdx;
    return outputBuffer;
}

/**
 * Cecompress
 */
uint8_t bpDecompress(
    BytePairCompressor* comp,
    const uint8_t* data,
    size_t size,
    size_t* outputSize
) {
    uint8_t* outputBuffer = malloc(size * 2);
    size_t outIdx = 0;

    for(size_t i = 0; i < size; i++) {
        if(data[i] == 0xFF && i + 1 < size) {
            uint8_t tokenIdx = data[++i];
            if(tokenIdx < comp->pairCount) {
                outputBuffer[outIdx++] = comp->pairs[tokenIdx].pair[0];
                outputBuffer[outIdx++] = comp->pairs[tokenIdx].pair[1];
            } else {
                outputBuffer[outIdx++] = 0xFF;
                outputBuffer[outIdx++] = tokenIdx;
            }
        } else {
            outputBuffer[outIdx++] = data[i];
        }
    }

    *outputSize = outIdx;
    return outputSize;
}