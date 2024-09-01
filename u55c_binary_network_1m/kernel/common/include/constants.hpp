#pragma once
#include <ap_int.h>

//////////////////////////////   TEMPLATE START  //////////////////////////////

typedef union
{
    float float32;
    uint32_t uint32;
} conv;

#define NUM_LABEL_KEY 1
#define NUM_NUMERICAL_KEY 13
#define NUM_CATEGORICAL_KEY 26

#define LABEL_WIDTH 4
#define NUMERICAL_WIDTH 4
#define CATEGORICAL_WIDTH 4

#define DICT_DEPTH 1048576

#define MEM_SIZE 1

/////////////////////////   HBM   ///////////////////////// 

#define HBM_BANK0_SIZE  (16*MEM_SIZE)  
#define HBM_BANK1_SIZE  (16*MEM_SIZE)  
#define HBM_BANK2_SIZE  (16*MEM_SIZE)  
#define HBM_BANK3_SIZE  (16*MEM_SIZE)  
#define HBM_BANK4_SIZE  (16*MEM_SIZE)  
#define HBM_BANK5_SIZE  (16*MEM_SIZE)  
#define HBM_BANK6_SIZE  (16*MEM_SIZE)  
#define HBM_BANK7_SIZE  (16*MEM_SIZE)  
#define HBM_BANK8_SIZE  (16*MEM_SIZE)  
#define HBM_BANK9_SIZE  (16*MEM_SIZE)  
#define HBM_BANK10_SIZE (16*MEM_SIZE)
#define HBM_BANK11_SIZE (16*MEM_SIZE)
#define HBM_BANK12_SIZE (16*MEM_SIZE)
#define HBM_BANK13_SIZE (16*MEM_SIZE)
#define HBM_BANK14_SIZE (16*MEM_SIZE)
#define HBM_BANK15_SIZE (16*MEM_SIZE)
#define HBM_BANK16_SIZE (16*MEM_SIZE)
#define HBM_BANK17_SIZE (16*MEM_SIZE)
#define HBM_BANK18_SIZE (16*MEM_SIZE)
#define HBM_BANK19_SIZE (16*MEM_SIZE)
#define HBM_BANK20_SIZE (16*MEM_SIZE)
#define HBM_BANK21_SIZE (16*MEM_SIZE)
#define HBM_BANK22_SIZE (16*MEM_SIZE)
#define HBM_BANK23_SIZE (16*MEM_SIZE)
#define HBM_BANK24_SIZE (16*MEM_SIZE)
#define HBM_BANK25_SIZE (16*MEM_SIZE)
#define HBM_BANK26_SIZE (16*MEM_SIZE)
#define HBM_BANK27_SIZE (16*MEM_SIZE)
#define HBM_BANK28_SIZE (16*MEM_SIZE)
#define HBM_BANK29_SIZE (16*MEM_SIZE)
#define HBM_BANK30_SIZE (16*MEM_SIZE)
#define HBM_BANK31_SIZE (16*MEM_SIZE)

//////////////////////////////   TEMPLATE END  //////////////////////////////

#define FIFO_BATCH_SIZE 8   // cache 8 batches at max in FIFO
//#define BATCH_SIZE 32
#define BATCH_SIZE 32
//#define BATCH_NUM 2
#define BATCH_NUM 1
//#define BATCH_NUM 5000 // NOTE! load access idx to BRAM!
