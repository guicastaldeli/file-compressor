#pragma once
#include <stdint.h>
#include <stddef.h>

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

BytePairCompressor* bpCreate(int maxVocabSize);
void bpDestroy(BytePairCompressor* comp);
void countPairs(
    BytePairCompressor* comp, 
    const uint8_t* data, 
    size_t size
);
uint8_t* bpCompress(
    BytePairCompressor* comp,
    const uint8_t* data, 
    size_t size,
    size_t* outputSize
);
uint8_t* bpDecompress(
    BytePairCompressor* comp, 
    const uint8_t* data,
    size_t size, 
    size_t* outputSize
);