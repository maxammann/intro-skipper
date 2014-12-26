#pragma once

#include <stdio.h>
#include <assert.h>
#include <zlib.h>
#include <stdlib.h>

#define OUT_CHUNK 16384

struct Strings_ {
    char **strings;
    unsigned int amount;
};

typedef struct Strings_ Strings;

Strings* new_strings(char **strings, unsigned int amount);

int def_strings(Strings *strings, FILE *dest, int level);
