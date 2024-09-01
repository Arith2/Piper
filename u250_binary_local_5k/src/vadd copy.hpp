#include <hls_stream.h>
#include <iostream>
#include <hls_math.h>

#include "constants.hpp"

#define BURSTBUFFERSIZE 1024

const unsigned int c_size_min = 1;
const unsigned int c_size_max = BURSTBUFFERSIZE;

extern "C" {

void vadd(  
    ap_uint<512>* table_DDR0,  
    ap_uint<512>* table_DDR1,
    ap_uint<512>* table_DDR2,
    int row_num,
    int max_vocab_size
    );
}

void LoadDDR_Num(
    ap_uint<512>* table_DDR,
    hls::stream<ap_uint<512> >& stream_out,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "LoadDDR_Num: " << std::endl;

    int burstbuffer[BURSTBUFFERSIZE];

    // LoadDDR_Num:
    // for (int i = 0; i < row_num; i++) {
    //     #pragma HLS pipeline II=1
    //     ap_uint<512> DDR_data = table_DDR[i];
    //     stream_out.write(DDR_data);  
    // }

    LoadDDR_Num:
    for (int i = 0; i < row_num; i += BURSTBUFFERSIZE) {
        // #pragma HLS pipeline II=1
        int chunk_size = BURSTBUFFERSIZE;
        if ((i + BURSTBUFFERSIZE) > row_num) chunk_size = row_num - i;

        for (int j = 0; j < chunk_size; j++) {
            // As the outer loop is not a perfect loop
            #pragma HLS loop_flatten off
            #pragma HLS LOOP_TRIPCOUNT min = c_size_min max = c_size_max
            burstbuffer[j] = table_DDR[i+j];
        }

        for (int j = 0; j < chunk_size; j++) {
            #pragma HLS pipeline II=1
            stream_out.write(burstbuffer[j]); 
        } 
    }
}

void LoadDDR_Cat(
    ap_uint<512>* table_DDR,
    hls::stream<ap_uint<512> >& stream_out,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "LoadDDR_Cat: " << std::endl;

    // int DDR_index = 0;

    // LoadDDR_Cat:
    // for (int i = 0; i < 2*row_num; i++) {
    //     #pragma HLS pipeline II=1

    //     DDR_index = (i < row_num) ? i : (i - row_num);
    //     ap_uint<512> DDR_data = table_DDR[DDR_index];
    //     stream_out.write(DDR_data); 
    // }
    int burstbuffer[BURSTBUFFERSIZE];

    LoadDDR:
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < row_num; i += BURSTBUFFERSIZE) {
            // #pragma HLS pipeline II=1
            int chunk_size = BURSTBUFFERSIZE;
            if ((i + BURSTBUFFERSIZE) > row_num) chunk_size = row_num - i;

            for (int j = 0; j < chunk_size; j++) {
                // As the outer loop is not a perfect loop
                #pragma HLS loop_flatten off
                #pragma HLS LOOP_TRIPCOUNT min = c_size_min max = c_size_max
                burstbuffer[j] = table_DDR[i+j];
            }

            for (int j = 0; j < chunk_size; j++) {
                #pragma HLS pipeline II=1
                stream_out.write(burstbuffer[j]); 
            }
        }
    }
}

void NegsToZeroLog(
    hls::stream<ap_uint<512> >& stream_in,
    hls::stream<ap_uint<512> >& stream_out,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "NegsToZeroLog: " << std::endl;

    NegsToZeroLog:
    for (int i = 0; i < row_num; i++) {
        #pragma HLS pipeline II=1
        ap_uint<512> input_data = stream_in.read();
        ap_uint<512> output_data;

        for (int j = 0; j < 16; j++) {
            #pragma HLS unroll

            int tmp_value;
            conv conv_value;

            tmp_value = input_data(32*j+31, 32*j);
            // tmp_value < 0 can be replaced by comparing MSB==1
            tmp_value = (tmp_value < 0) ? 0 : tmp_value;

            // conv_value.float32 = hls::logf(tmp_value+1);
            // output_data(32*j+31, 32*j) = conv_value.uint32;
            output_data(32*j+31, 32*j) = tmp_value;

            // std::cout << "log(" << (tmp_value+1) << ")=" << conv_value.float32 << "(" << conv_value.uint32 << ") ";
        }
        // std::cout << std::endl;

        stream_out.write(output_data);  
    }
}

void HexToIntModRange(
    hls::stream<ap_uint<512> >& stream_in,
    hls::stream<ap_uint<512> >& stream_out,
    int row_num,
    int max_vocab_size
) {
    #pragma HLS INLINE off

    std::cout << "HexToIntModRange: " << std::endl;

    HexToIntModRange:
    for (int i = 0; i < 2*row_num; i++) {
        #pragma HLS pipeline II=1
        ap_uint<512> input_data = stream_in.read();
        ap_uint<512> output_data;

        for (int j = 0; j < 16; j++) {
            #pragma HLS unroll
            uint32_t tmp_input = input_data(32*j+31, 32*j);
            uint32_t tmp_output = tmp_input % max_vocab_size;
            output_data(32*j+31, 32*j) = tmp_output;

            // std::cout << tmp_input << "%" << max_vocab_size << "=" << tmp_output << " ";
        }
        // std::cout << std::endl;

        stream_out.write(output_data);  
    }
}

void StreamSplit(
    hls::stream<ap_uint<512> >& stream_in,
    hls::stream<ap_uint<32> >& stream_out_0,
    hls::stream<ap_uint<32> >& stream_out_1,
    hls::stream<ap_uint<32> >& stream_out_2,
    hls::stream<ap_uint<32> >& stream_out_3,
    hls::stream<ap_uint<32> >& stream_out_4,
    hls::stream<ap_uint<32> >& stream_out_5,
    hls::stream<ap_uint<32> >& stream_out_6,
    hls::stream<ap_uint<32> >& stream_out_7,
    hls::stream<ap_uint<32> >& stream_out_8,
    hls::stream<ap_uint<32> >& stream_out_9,
    hls::stream<ap_uint<32> >& stream_out_10,
    hls::stream<ap_uint<32> >& stream_out_11,
    hls::stream<ap_uint<32> >& stream_out_12,
    hls::stream<ap_uint<32> >& stream_out_13,
    hls::stream<ap_uint<32> >& stream_out_14,
    hls::stream<ap_uint<32> >& stream_out_15,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "StreamSplit: " << std::endl;

    StreamSplit:
    for (int i = 0; i < 2*row_num; i++) {
        #pragma HLS pipeline II=1
        ap_uint<512> input_data = stream_in.read();

        stream_out_0.write(input_data(31, 0));
        stream_out_1.write(input_data(63, 32));
        stream_out_2.write(input_data(95, 64));
        stream_out_3.write(input_data(127, 96));
        stream_out_4.write(input_data(159, 128));
        stream_out_5.write(input_data(191, 160));
        stream_out_6.write(input_data(223, 192));
        stream_out_7.write(input_data(255, 224));
        stream_out_8.write(input_data(287, 256));
        stream_out_9.write(input_data(319, 288));
        stream_out_10.write(input_data(351, 320));
        stream_out_11.write(input_data(383, 352));
        stream_out_12.write(input_data(415, 384));
        stream_out_13.write(input_data(447, 416));
        stream_out_14.write(input_data(479, 448));
        stream_out_15.write(input_data(511, 480));
    }
}

// template <const int ID>
void HashDict(
    hls::stream<ap_uint<32> >& stream_in,
    hls::stream<ap_uint<32> >& stream_out,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "HashDict: " << std::endl;

    ap_uint<1> hash_dict[DICT_DEPTH];
    #pragma HLS bind_storage variable=hash_dict type=RAM_1P impl=BRAM latency=1

    int count_stream_in = 0;
    int count_stream_out = 0;
    ap_uint<32> input_data;
    ap_uint<32> patch_data = 0xffffffff;

    InitializeDict:
    for (int i = 0; i < DICT_DEPTH; i++) {
        #pragma HLS pipeline II=1
        hash_dict[i] = 0;
    }

    HashDictAndPatch:
    do {

        if (count_stream_in < row_num) {
            input_data = stream_in.read();
            if (hash_dict[input_data]== 0) {
                hash_dict[input_data] = 1;
                stream_out.write(input_data);
            }
            else {
                stream_out.write(patch_data);
            }
        }
        else {
            input_data = stream_in.read();
            stream_out.write(input_data);
        }

        count_stream_in++;

    }while(count_stream_in < (2*row_num));
    
}

void CreateDict(
    hls::stream<ap_uint<32> >& stream_in,
    hls::stream<ap_uint<32> >& stream_out,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "CreateDict: " << std::endl;

    ap_uint<32> feature_dict[DICT_DEPTH];
    #pragma HLS bind_storage variable=feature_dict type=RAM_1P impl=BRAM latency=1
    ap_uint<32> patch_data = 0xffffffff;
    ap_uint<32> output_feature = 0;
    int feature_count = 0;
    // int count_stream_in = 0;

    // CreateDict:
    // do {
    //     ap_uint<32> input_data = stream_in.read();
    //     if (input_data!=patch_data) {
    //         feature_dict[input_data] = i;
    //         std::cout << input_data << ": " << i << std::endl;;
    //     }
    //     count_stream_in++;
    // }while(count_stream_in < (2*row_num));

    CreateDict:
    for (int i = 0; i < 2*row_num; i++) {
        #pragma HLS pipeline II=1
        ap_uint<32> input_data = stream_in.read();
        
        if (i < row_num && input_data!=patch_data) {
            feature_dict[input_data] = feature_count;
            feature_count++;
            // std::cout << input_data << ": " << i << std::endl;;
        }
        else if (i >= row_num) {
            output_feature = feature_dict[input_data];
            stream_out.write(output_feature);
            // std::cout << input_data << "->" << output_feature << std::endl;
        }
        else {
            // discard invalid data;
        }
        
        
    }

    // MapVocab:
    // for (int i = 0; i < row_num; i++) {
    //     #pragma HLS pipeline II=1
    //     stream_out.write(0);
    // }
    
}

// template <const int ID>
void StreamCombine(
    hls::stream<ap_uint<32> >& stream_in_0,
    hls::stream<ap_uint<32> >& stream_in_1,
    hls::stream<ap_uint<32> >& stream_in_2,
    hls::stream<ap_uint<32> >& stream_in_3,
    hls::stream<ap_uint<32> >& stream_in_4,
    hls::stream<ap_uint<32> >& stream_in_5,
    hls::stream<ap_uint<32> >& stream_in_6,
    hls::stream<ap_uint<32> >& stream_in_7,
    hls::stream<ap_uint<32> >& stream_in_8,
    hls::stream<ap_uint<32> >& stream_in_9,
    hls::stream<ap_uint<32> >& stream_in_10,
    hls::stream<ap_uint<32> >& stream_in_11,
    hls::stream<ap_uint<32> >& stream_in_12,
    hls::stream<ap_uint<32> >& stream_in_13,
    hls::stream<ap_uint<32> >& stream_in_14,
    hls::stream<ap_uint<32> >& stream_in_15,
    hls::stream<ap_uint<512> >& stream_out,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "StreamCombine: " << std::endl;

    StreamCombine:
    for (int i = 0; i < row_num; i++) {
        #pragma HLS pipeline II=1
        ap_uint<512> output_data;

        output_data(31, 0)    = stream_in_0.read();
        output_data(63, 32)   = stream_in_1.read();
        output_data(95, 64)   = stream_in_2.read();
        output_data(127, 96)  = stream_in_3.read();
        output_data(159, 128) = stream_in_4.read();
        output_data(191, 160) = stream_in_5.read();
        output_data(223, 192) = stream_in_6.read();
        output_data(255, 224) = stream_in_7.read();
        output_data(287, 256) = stream_in_8.read();
        output_data(319, 288) = stream_in_9.read();
        output_data(351, 320) = stream_in_10.read();
        output_data(383, 352) = stream_in_11.read();
        output_data(415, 384) = stream_in_12.read();
        output_data(447, 416) = stream_in_13.read();
        output_data(479, 448) = stream_in_14.read();
        output_data(511, 480) = stream_in_15.read();

        stream_out.write(output_data);
    }
}


void ConsumeStream_Num(
    hls::stream<ap_uint<512> >& stream_in_num,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "ConsumeStream_Num: " << std::endl;

    Load_DDR:
    for (int i = 0; i < row_num; i++) {
        #pragma HLS pipeline II=1
        ap_uint<512> DDR_data_num = stream_in_num.read();
    }
}

 
void ConsumeStream_Cat(
    hls::stream<ap_uint<512> >& stream_in_cat_0,
    hls::stream<ap_uint<512> >& stream_in_cat_1,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "ConsumeStream_Cat: " << std::endl;

    Load_DDR:
    for (int i = 0; i < row_num; i++) {
        #pragma HLS pipeline II=1
        ap_uint<512> DDR_data_cat_0 = stream_in_cat_0.read();
        ap_uint<512> DDR_data_cat_1 = stream_in_cat_1.read();
    }
}


void StoreDDR(
    hls::stream<ap_uint<512> >& stream_in,
    ap_uint<512>* table_DDR,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "StoreDDR: " << std::endl;

    int burstbuffer[BURSTBUFFERSIZE];

    StoreDDR:
    for (int i = 0; i < row_num; i += BURSTBUFFERSIZE) {
        // #pragma HLS pipeline II=1
        int chunk_size = BURSTBUFFERSIZE;
        if ((i + BURSTBUFFERSIZE) > row_num) chunk_size = row_num - i;

        for (int j = 0; j < chunk_size; j++) {
            #pragma HLS pipeline II=1
            burstbuffer[j] = stream_in.read();
        }

        for (int j = 0; j < chunk_size; j++) {
            // As the outer loop is not a perfect loop
            #pragma HLS loop_flatten off
            #pragma HLS LOOP_TRIPCOUNT min = c_size_min max = c_size_max
            table_DDR[i+j] = burstbuffer[j];
        }
    }

}