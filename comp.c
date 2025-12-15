#include "comp.h"
#include "bp.h"
#include "rl.h"
#include "sliding_window.h"
#include "delta.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

CompressionType detectBestCompression(const uint8_t* data, size_t size) {
    if(size < 100) return COMP_NONE;
    printf("DEBUG C: detectBestCompression for %zu bytes\n", size);
    
    int isLikelyVideo = 0;
    int isLikelyImage = 0;
    
    if(size > 100) {
        if(data[0] == 0x00 && data[1] == 0x00 && 
            (data[2] == 0x01 || data[2] == 0xBA || data[2] == 0xB3)) {
            isLikelyVideo = 1;
            printf("DEBUG C: Detected likely video format\n");
        }
        if(data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G') {
            isLikelyImage = 1;
            printf("DEBUG C: Detected PNG format\n");
        }
        if(data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) {
            isLikelyImage = 1;
            printf("DEBUG C: Detected JPEG format\n");
        }
    }
    if(isLikelyVideo || isLikelyImage) {
        printf("DEBUG C: Skipping compression for video/image format\n");
        return COMP_NONE;
    }
    
    int byteFreq[256] = {0};
    int maxFreq = 0;
    size_t sampleSize = size > 1000000 ? 1000000 : size;
    
    for(size_t i = 0; i < sampleSize; i++) {
        byteFreq[data[i]]++;
        if(byteFreq[data[i]] > maxFreq) {
            maxFreq = byteFreq[data[i]];
        }
    }

    int textBytes = 0;
    for(int i = 32; i < 127; i++) textBytes += byteFreq[i];
    textBytes += byteFreq['\t'] + byteFreq['\n'] + byteFreq['\r'];
    
    if(textBytes * 100 / sampleSize > 70) {
        printf("DEBUG C: High text content, using Byte Pair compression\n");
        return COMP_BP;
    }

    int runCount = 0;
    for(size_t i = 1; i < sampleSize; i++) {
        if(data[i] == data[i-1]) runCount++;
    }
    if(runCount * 100 / sampleSize > 20) {
        printf("DEBUG C: High run count, using Run-Length compression\n");
        return COMP_RL;
    }

    int smallDeltas = 0;
    for(size_t i = 1; i < sampleSize; i++) {
        int delta = abs((int)data[i] - (int)data[i-1]);
        if(delta < 16) smallDeltas++;
    }
    if(smallDeltas * 100 / sampleSize > 60) {
        printf("DEBUG C: High small deltas, using Delta compression\n");
        return COMP_DELTA;
    }

    printf("DEBUG C: Default to Sliding Window compression\n");
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
        *usedType = COMP_NONE;
        return NULL;
    }

    printf("DEBUG C: compress called with size: %zu bytes (%.2f MB)\n", 
           size, size / (1024.0 * 1024.0));
    
    if(size > 10 * 1024 * 1024) {
        int binaryLikelihood = 0;
        for (int i = 0; i < 100 && i < size; i++) {
            if(data[i] < 32 && data[i] != '\t' && data[i] != '\n' && data[i] != '\r') {
                binaryLikelihood++;
            }
        }
        if(binaryLikelihood > 80) {
            printf("DEBUG C: Large binary file detected, skipping compression\n");
            uint8_t* result = (uint8_t*)malloc(size);
            if(!result) return NULL;
            memcpy(result, data, size);
            *outputSize = size;
            *usedType = COMP_NONE;
            return result;
        }
    }
    
    CompressionType bestType = detectBestCompression(data, size);
    printf("DEBUG C: Best compression type: %d\n", bestType);
    
    if(bestType == COMP_NONE) {
        printf("DEBUG C: Using NO compression\n");
        uint8_t* result = (uint8_t*)malloc(size);
        if(!result) {
            printf("ERROR C: malloc failed for size: %zu\n", size);
            *outputSize = 0;
            *usedType = COMP_NONE;
            return NULL;
        }
        memcpy(result, data, size);
        *outputSize = size;
        *usedType = COMP_NONE;
        return result;
    }
    
    *usedType = bestType;
    uint8_t* compressed = NULL;
    size_t compressedSize = 0;

    switch(bestType) {
        case COMP_RL:
            printf("DEBUG C: Using RL compression\n");
            compressed = rlCompress(data, size, &compressedSize);
            break;
        case COMP_DELTA:
            printf("DEBUG C: Using Delta compression\n");
            compressed = deltaCompress(data, size, &compressedSize);
            break;
        case COMP_SW:
            printf("DEBUG C: Using Sliding Window compression\n");
            compressed = swCompress(data, size, &compressedSize);
            break;
        case COMP_BP: {
            printf("DEBUG C: Using Byte Pair compression\n");
            BytePairCompressor* comp = bpCreate(256);
            countPairs(comp, data, size);
            compressed = bpCompress(comp, data, size, &compressedSize);
            bpDestroy(comp);
            break;
        }
        default:
            printf("ERROR C: Unknown compression type: %d\n", bestType);
            compressed = (uint8_t*)malloc(size);
            if(!compressed) {
                *outputSize = 0;
                *usedType = COMP_NONE;
                return NULL;
            }
            memcpy(compressed, data, size);
            compressedSize = size;
            *usedType = COMP_NONE;
            break;
    }
    if(!compressed) {
        printf("ERROR C: Compression algorithm returned NULL\n");
        *outputSize = 0;
        *usedType = COMP_NONE;
        return NULL;
    }

    printf("DEBUG C: Compressed size: %zu bytes (%.2f MB), ratio: %.2f%%\n", 
           compressedSize, compressedSize / (1024.0 * 1024.0),
           (double)compressedSize / size * 100.0);

    if(compressedSize >= size * 0.98) {
        printf("DEBUG C: Compression not beneficial (<2%% reduction), returning original\n");
        free(compressed);
        uint8_t* result = (uint8_t*)malloc(size);
        if(!result) {
            printf("ERROR C: malloc failed for original size: %zu\n", size);
            *outputSize = 0;
            *usedType = COMP_NONE;
            return NULL;
        }
        memcpy(result, data, size);
        *outputSize = size;
        *usedType = COMP_NONE;
        return result;
    }

    *outputSize = compressedSize;
    return compressed;
}

/**
 * Decompress
 */
uint8_t* decompress(
    const uint8_t* data,
    size_t size,
    size_t* outputSize,
    CompressionType compType
) {
    switch(compType) {
        case COMP_RL:
            return rlDecompress(data, size, outputSize);
        case COMP_DELTA:
            return deltaDecompress(data, size, outputSize);
        case COMP_SW:
            return swDecompress(data, size, outputSize);
        case COMP_BP: {
            BytePairCompressor* comp = bpCreate(256);
            uint8_t* decompressed = bpDecompress(comp, data, size, outputSize);
            bpDestroy(comp);
            return decompressed;
        }
        case COMP_NONE:
        default:
            *outputSize = size;
            uint8_t* output = malloc(size);
            memcpy(output, data, size);
            return output;
    }
}