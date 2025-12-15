#pragma once
#include <stdint.h>
#include <stddef.h>

#define WINDOW_SIZE 4096
#define LOOKAHEAD_SIZE 18

typedef struct {
    uint16_t offset;
    uint16_t length;
    uint8_t next;
} Match;


Match findLongestMatch(
    const uint8_t* window, 
    int windowPos,
    const uint8_t* lookahead, 
    int lookaheadLen
);
uint8_t* swCompress(
    const uint8_t* data, 
    size_t size, 
    size_t* outputSize
);
uint8_t* swDecompress(
    const uint8_t* data, 
    size_t size, 
    size_t* outputSize
);