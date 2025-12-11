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
}