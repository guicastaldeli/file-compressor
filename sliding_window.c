#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
) {
    Match best = { 0, 0, lookahead[0] };
    int maxOffset = 
        windowPos < WINDOW_SIZE ? 
        windowPos : WINDOW_SIZE;
    
    for(int offset = 1; offset <= maxOffset; offset++) {
        int matchLen = 0;
        while(
            matchLen < lookaheadLen &&
            matchLen < LOOKAHEAD_SIZE &&
            window[windowPos - offset + matchLen] == lookahead[matchLen]
        ) {
            matchLen++;
        }
        if(matchLen > best.length) {
            best.offset = offset;
            best.length = matchLen;
            if(matchLen < lookaheadLen) {
                best.next = lookahead[matchLen];
            } else {
                best.next = 0;
            }
        }
    }

    return best;
}

/**
 * Compress
 */
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

    uint8_t window[WINDOW_SIZE] = {0};
    int windowPos = 0;

    size_t i = 0;
    while(i < size) {
        int lookaheadLen = size - i;
        if(lookaheadLen > LOOKAHEAD_SIZE) {
            lookaheadLen = LOOKAHEAD_SIZE;
        }

        Match match = findLongestMatch(
            window,
            windowPos,
            data + i,
            lookaheadLen
        );
        if(match.length > 2) {
            outputBuffer[outIdx++] = 0xFE;
            outputBuffer[outIdx++] = (match.offset >> 8) & 0xFF;
            outputBuffer[outIdx++] = match.offset & 0xFF;
            outputBuffer[outIdx++] = match.length;
            outputBuffer[outIdx++] = match.next;

            for(int j = 0; j < match.length + 1; j++) {
                window[windowPos] = data[i + j];
                windowPos = (windowPos + 1) % WINDOW_SIZE;
            }

            i += match.length + 1;
        } else {
            outputBuffer[outIdx++] = data[i];
            window[windowPos] = data[i];
            windowPos = (windowPos + 1) % WINDOW_SIZE;
            i++;
        }
    }

    *outputSize = outIdx;
    return outputBuffer;
}

/**
 * Decompress
 */
uint8_t* decompress(
    const uint8_t* data,
    size_t size,
    size_t* outputSize
) {
    uint8_t* outputBuffer = malloc(size * 10);
    size_t outIdx = 0;

    uint8_t window[WINDOW_SIZE] = {0};
    int windowPos = 0;

    size_t i = 0;
    while(i < size) {
        if(data[i] == 0xFE && i + 4 < size) {
            uint16_t offset = (data[i+1] << 8) | data[i+2];
            uint8_t length = data[i+3];
            uint8_t nextChair = data[i+4];

            for(int j = 0; j < length; j++) {
                int srcPos = (windowPos - offset + j) % WINDOW_SIZE;
                if(srcPos < 0) srcPos += WINDOW_SIZE;

                outputBuffer[outIdx] = window[srcPos];
                window[windowPos] = outputBuffer[outIdx];
                windowPos = (windowPos + 1) % WINDOW_SIZE;
                outIdx++;
            }

            if(nextChair != 0) {
                outputBuffer[outIdx] = nextChair;
                window[windowPos] = nextChair;
                window[windowPos] = nextChair;
                windowPos = (windowPos + 1) % WINDOW_SIZE;
                outIdx++;
            }

            i += 5;
        } else {
            outputBuffer[outIdx] = data[i];
            window[windowPos] = data[i];
            windowPos = (windowPos + 1) % WINDOW_SIZE;
            outIdx++;
            i++;
        }
    }

    *outputSize = outIdx;
    return outputBuffer;
}