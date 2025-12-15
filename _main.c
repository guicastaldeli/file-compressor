#include "_main.h"
#include "comp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

/**
 * Compress
 */
int compressFile(const char* inputPath, const char* outputPath) {
    FILE* in = fopen(inputPath, "rb");
    if(!in) {
        printf("ERROR: Cannot open input file: %s\n", inputPath);
        return -1;
    }

    fseek(in, 0, SEEK_END);
    size_t fileSize = ftell(in);
    fseek(in, 0, SEEK_SET);

    uint8_t* data = malloc(fileSize);
    if(!data) {
        printf("ERROR: Memory allocation failed for size: %zu\n", fileSize);
        fclose(in);
        return -1;
    }

    size_t readSize = fread(data, 1, fileSize, in);
    fclose(in);
    
    if(readSize != fileSize) {
        printf("ERROR: Read incomplete: %zu != %zu\n", readSize, fileSize);
        free(data);
        return -1;
    }

    size_t compressedSize;
    CompressionType compType;
    uint8_t* compressed = compress(
        data,
        fileSize,
        &compressedSize,
        &compType
    );

    if(!compressed) {
        printf("ERROR: Compression failed\n");
        free(data);
        return -3;
    }

    CompHeader header;
    header.magic = COMP_MAGIC;
    header.version = 0x0001;
    header.compType = compType;
    header.reserved = 0;
    header.originalSize = (uint32_t)fileSize;

    FILE* out = fopen(outputPath, "wb");
    if(!out) {
        printf("ERROR: Cannot open output file: %s\n", outputPath);
        free(data);
        free(compressed);
        return -2;
    }

    fwrite(&header, sizeof(CompHeader), 1, out);
    fwrite(compressed, 1, compressedSize, out);
    fclose(out);

    double ratio = (double)compressedSize / fileSize;
    printf("Compressed %s: %zu -> %zu bytes (%.1f%%)\n",
           inputPath, fileSize, compressedSize, ratio * 100);

    free(data);
    free(compressed);
    return 0;
}

/**
 * Decompress
 */
int decompressFile(const char* inputPath, const char* outputPath) {
    FILE* in = fopen(inputPath, "rb");
    if(!in) return -1;

    CompHeader header;
    fread(&header, sizeof(CompHeader), 1, in);
    if(header.magic != COMP_MAGIC) {
        fclose(in);
        return -2;
    }

    fseek(in, 0, SEEK_END);
    size_t totalSize = ftell(in);
    fseek(in, sizeof(CompHeader), SEEK_SET);
    size_t compressedSize = totalSize - sizeof(CompHeader);

    uint8_t* compressed = malloc(compressedSize);
    fread(compressed, 1, compressedSize, in);
    fclose(in);

    size_t decompressedSize;
    uint8_t* decompressed = decompress(
        compressed,
        compressedSize,
        &decompressedSize,
        (CompressionType)header.compType
    );
    if (decompressedSize != header.originalSize) {
        printf("Warning: Size mismatch! Expected %u, got %zu\n",
               header.originalSize, decompressedSize);
    }

    FILE* out = fopen(outputPath, "wb");
    if(!out) {
        free(compressed);
        free(decompressed);
        return -3;
    }

    fwrite(decompressed, 1, decompressedSize, out);
    fclose(out);

    printf("Decompressed %s: %zu -> %zu bytes\n",
           inputPath, compressedSize, decompressedSize);

    free(compressed);
    free(decompressed);
    return 0;
}

int main(int argc, char* argv[]) {
    if(argc != 4) {
        printf("Hello\n");
        printf("  %s compress <input> <output>\n", argv[0]);
        printf("  %s decompress <input> <output>\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "compress") == 0) {
        return compressFile(argv[2], argv[3]);
    } else if (strcmp(argv[1], "decompress") == 0) {
        return decompressFile(argv[2], argv[3]);
    } else {
        printf("Unknown command: %s\n", argv[1]);
        return 1;
    }
}