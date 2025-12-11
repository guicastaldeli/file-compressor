#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    uint16_t pair[2];
    uint16_t token;
    int count;
} BytePair;