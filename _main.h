#pragma once
#include <stdint.h>

#define COMP_MAGIC 0x434D5052

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t compType;
    uint8_t reserved;
    uint32_t originalSize;
} CompHeader;

int compressFile(const char* inputPath, const char* outputPath);
int decompressFile(const char* inputPath, const char* outputPath);