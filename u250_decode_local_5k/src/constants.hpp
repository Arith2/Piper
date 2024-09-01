#pragma once
#include <ap_int.h>

//////////////////////////////   TEMPLATE START  //////////////////////////////
typedef unsigned long long ull;

typedef union
{
    float float32;
    uint32_t uint32;
} conv;

#define NUM_LABEL_KEY 1
#define NUM_NUMERICAL_KEY 13
#define NUM_CAT 26

#define LABEL_WIDTH 4
#define NUMERICAL_WIDTH 4
#define CATEGORICAL_WIDTH 4

#define DICT_DEPTH 8192

#define MEM_SIZE 1

// ull SINGLE_BUFFER_LIMIT = (unsigned long long)(4*1024*1024*1024);
#define SINGLE_BUFFER_LIMIT 4294967296U

#define BURSTBUFFERSIZE 4096
#define BURSTBUFFERSIZE_INT 1024
