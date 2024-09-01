#include <hls_stream.h>
#include <iostream>
#include <hls_math.h>

#include "constants.hpp"

extern "C" {

void vadd(  
    ap_uint<64>* table_DDR0,  
    ap_uint<64>* table_DDR1,  
    ap_uint<64>* table_DDR2, 
    ull input_bytes,
    int max_vocab_size
    );
}

void LoadDDR_UTF8(
    ap_uint<64>* table_DDR0,
    ap_uint<64>* table_DDR1,
    ap_uint<64>* table_DDR2,
    hls::stream<ap_uint<64> >& stream_out,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "LoadDDR_UTF8: " << std::endl; 

    ull count_index = 0;
    ull ddr_index = 0;
    ull read_loop = 0;
    ap_uint<8> end_of_text = 3;
    ap_uint<8> ddr_data = 0;

    ull dram_size = input_bytes / 3;
    ull dram_size_0 = dram_size;
    ull dram_size_1 = dram_size;
    ull dram_size_2 = input_bytes - 2 * dram_size;

    ull loop_num = input_bytes + 1;
    ull mem_index = 0;
    // std::cout << test_num << ", " << (test_num+1) << std::endl;

    LoadDDR:
    for (int j = 0; j < 2; j++) {
        for (ull i = 0; i < loop_num; i++) {
            #pragma HLS pipeline II=1
            if (i < dram_size) {
                mem_index = i;
                ddr_data = table_DDR0[mem_index];
            }
            else if (i < 2 * dram_size) {
                mem_index = i - dram_size;
                ddr_data = table_DDR1[mem_index];
            }
            else if (i < input_bytes) {
                mem_index = i - 2*dram_size;
                ddr_data = table_DDR2[mem_index];
            }
            else {
                ddr_data = end_of_text;
            }
            stream_out.write(ddr_data); 
        }
    }
}

void DecodeUTF8(
    hls::stream<ap_uint<8> >& stream_in,
    hls::stream<ap_uint<64> >& numerical_stream_0,
    // hls::stream<ap_uint<64> >& categorical_stream_0,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "DecodeUTF8: " << std::endl;

    // Use Count_wait to determine whether the upstream data has been fully consumed
    // int count_wait = 0;
    int feature_index = 0;
    int dict_index = 0;
    int read_loop = 0;
    int output_data = 0;
    bool input_flag = false;
    bool negative_flag = false;
    bool second_output_flag = false;
    
    ap_uint<8> end_of_text = 3;
    ap_uint<8> horizontal_tab = 9;
    ap_uint<8> new_line = 10;
    ap_uint<64> output_data_64 = 0;
    ap_uint<64> end_of_text_data = 0xffffffff00000000;

    ap_uint<4> feature_dict[16];

    InitializeDict:
    for (int i = 0; i < 16; i++) {
        #pragma HLS pipeline II=1
        feature_dict[i] = i;
    }

    DecodeUTF8:
    do {
        #pragma HLS pipeline II=1

        if (!stream_in.empty()) {
            ap_uint<8> input_data = stream_in.read();
            // std::cout << "input_data: " << input_data << std::endl;
            // input_flag = true;
            // count_wait = 0;

            if (input_data == end_of_text) {
                numerical_stream_0.write(end_of_text_data);
                // categorical_stream_0.write(end_of_text_data);
                read_loop++;
                feature_index = 0;
            }
            else if (input_data == horizontal_tab || input_data == new_line) {
                if (feature_index < 14) {
                    if (negative_flag) {
                        negative_flag = false;
                        output_data = ~output_data + 1;
                        // std::cout << "  negative" << std::endl;
                    }
                    output_data_64(31, 0) = output_data;
                    output_data_64(63, 32) = 0;

                    // numerical_stream_0.write(output_data_64);
                    
                    // std::cout << "  feature_index: " << feature_index << ", " << output_data << std::endl;
                }
                else {
                    output_data_64(31, 0) = output_data;
                    output_data_64(63, 32) = 0;

                    // categorical_stream_0.write(output_data_64);
                    // std::cout << "  feature_index: " << feature_index << ", " << std::hex << output_data << std::dec << std::endl;
                }
                numerical_stream_0.write(output_data_64);

                feature_index = (feature_index < 39) ? (feature_index + 1) : 0;
                
                output_data = 0;
            }
            else {
                if (input_data == 45){
                    negative_flag = true;
                    dict_index = 0;
                }
                else {
                    // 0~9
                    if (input_data >= 48 && input_data <= 57) {
                        dict_index = input_data - 48;
                    }
                    // a~f (10~15)
                    else if (input_data >= 97 && input_data <= 102) {
                        dict_index = input_data - 87;
                    }
                    else {
                        // std::cout << "  DATA ERROR, " << input_data << std::endl;
                        dict_index = 0;
                    } 

                    if (feature_index < 14) {
                        output_data = output_data * 10 + feature_dict[dict_index];
                    } 
                    else {
                        output_data = (output_data << 4) | feature_dict[dict_index];
                    }
                }               
            }
        }        
    }while(read_loop < 2);

}

void Neg2ZeroLog(
    hls::stream<ap_uint<64> >& stream_in,
    hls::stream<ap_uint<64> >& stream_out,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "Neg2ZeroLog: " << std::endl;

    // Use Count_wait to determine whether the upstream data has been fully consumed
    int read_loop = 0;
    int feature_index = 0;
    // int output_data = 0;
    // bool input_flag = false;

    ap_uint<64> input_data = 0;
    ap_uint<64> output_data = 0;
    ap_uint<64> end_of_text_data = 0xffffffff00000000;

    Neg2ZeroLog:
    do {
        #pragma HLS pipeline II=1

        if (!stream_in.empty()) {
            input_data = stream_in.read();
            // std::cout << "input_data: " << input_data << std::endl;
            // input_flag = true;
            if (input_data == end_of_text_data) {
                output_data = end_of_text_data;
                read_loop++;
                feature_index = 0;
            }
            else {
                if (feature_index < 14) {
                    int tmp_value = input_data(31, 0);
                    tmp_value = (tmp_value < 0) ? 0 : tmp_value;
                    // conv conv_value;
                    // conv_value.float32 = hls::logf(tmp_value+1);
                    output_data(31, 0) = tmp_value;
                    output_data(63, 32) = 0;
                    // output_data(63, 32) = conv_value.uint32;
                }
                else {
                    output_data = input_data;
                }

                feature_index = (feature_index < 39) ? (feature_index + 1) : 0;
            }
    
            stream_out.write(output_data);  
            
        }
        
    }while(read_loop < 2);

    // Neg2ZeroLog:
    // for (int i = 0; i < input_bytes; i++) {
    //     #pragma HLS pipeline II=1
    //     ap_uint<512> input_data = stream_in.read();
    //     ap_uint<512> output_data;

    //     for (int j = 0; j < 16; j++) {
    //         #pragma HLS unroll

    //         int tmp_value;
    //         conv conv_value;

    //         tmp_value = input_data(32*j+31, 32*j);
    //         // tmp_value < 0 can be replaced by comparing MSB==1
    //         tmp_value = (tmp_value < 0) ? 0 : tmp_value;
    //         conv_value.float32 = hls::logf(tmp_value+1);
    //         output_data(32*j+31, 32*j) = conv_value.uint32;

    //         // std::cout << "log(" << (tmp_value+1) << ")=" << conv_value.float32 << "(" << conv_value.uint32 << ") ";
    //     }
    //     // std::cout << std::endl;

    //     stream_out.write(output_data);  
    // }
}

void Hex2IntMod(
    hls::stream<ap_uint<64> >& stream_in,
    hls::stream<ap_uint<64> >& stream_out,
    ull input_bytes,
    int max_vocab_size
) {
    #pragma HLS INLINE off

    std::cout << "Hex2IntMod: " << std::endl;

    int read_loop = 0;
    int feature_index = 0;
    // bool input_flag = false;

    ap_uint<64> input_data = 0;
    ap_uint<64> output_data = 0;
    ap_uint<64> end_of_text_data = 0xffffffff00000000;

    Hex2IntMod:
    do {
        #pragma HLS pipeline II=1

        if (!stream_in.empty()) {
            input_data = stream_in.read();
            // input_flag = true;

            if (input_data == end_of_text_data) {
                output_data = end_of_text_data;
                read_loop++;
                feature_index = 0;
            }
            else {
                if (feature_index < 14) {
                    output_data = input_data;
                }
                else {
                    uint32_t tmp_input = input_data(31, 0);
                    uint32_t tmp_output = tmp_input % max_vocab_size;

                    output_data(31, 0) = tmp_output;
                    output_data(63, 32) = 0;
                }

                feature_index = (feature_index < 39) ? (feature_index + 1) : 0;
            }

            stream_out.write(output_data);  
            
        }
        
    }while(read_loop < 2);

    // Hex2IntMod:
    // for (int i = 0; i < 2*input_bytes; i++) {
    //     #pragma HLS pipeline II=1
    //     ap_uint<512> input_data = stream_in.read();
    //     ap_uint<512> output_data;

    //     for (int j = 0; j < 16; j++) {
    //         #pragma HLS unroll
    //         uint32_t tmp_input = input_data(32*j+31, 32*j);
    //         uint32_t tmp_output = tmp_input % max_vocab_size;
    //         output_data(32*j+31, 32*j) = tmp_output;

    //         // std::cout << tmp_input << "%" << max_vocab_size << "=" << tmp_output << " ";
    //     }
    //     // std::cout << std::endl;

    //     stream_out.write(output_data);  
    // }
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
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "StreamSplit: " << std::endl;

    StreamSplit:
    for (int i = 0; i < 2*input_bytes; i++) {
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
void MapDict(
    hls::stream<ap_uint<64> >& stream_in,
    hls::stream<ap_uint<64> >& stream_out, 
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "MapDict: " << std::endl;

    ap_uint<1> hash_dict[NUM_CAT][DICT_DEPTH];
    #pragma HLS bind_storage variable=hash_dict type=RAM_2P impl=BRAM latency=1
    #pragma HLS ARRAY_PARTITION variable=hash_dict dim=1 complete

    // int count_stream_in = 0;
    // int count_stream_out = 0;
    int read_loop = 0;
    int dict_index = 0;
    int cat_index = 0;
    int feature_index = 0;
    ap_uint<64> input_data = 0;
    ap_uint<64> output_data = 0;
    ap_uint<64> end_of_text_data = 0xffffffff00000000;
    ap_uint<64> patch_data = 0xffffffffffffffff;

    // int count_wait = 0;
    // bool input_flag = false;

    InitializeDict:
    for (int i = 0; i < DICT_DEPTH; i++) {
        #pragma HLS pipeline II=1

        for (int j = 0; j < NUM_CAT; j++) {
            #pragma HLS unroll 
            hash_dict[j][i] = 0;
        }
        
    }

    MapDictAndPatch:
    do {
        #pragma HLS pipeline II=2
        if (!stream_in.empty()) {
            input_data = stream_in.read();
            // input_flag = true;

            if (input_data == end_of_text_data) {
                read_loop++;
                output_data = end_of_text_data;
                feature_index = 0;
            }
            else if (read_loop == 0){
                if (feature_index < 14) {
                    output_data = input_data;
                }
                else {
                    dict_index = input_data(31, 0);

                    if (hash_dict[cat_index][dict_index]== 0) {
                        hash_dict[cat_index][dict_index] = 1;
                        // stream_out.write(input_data);
                        output_data = input_data;
                    }
                    else {
                        // stream_out.write(patch_data);
                        output_data = patch_data;
                    }

                    cat_index = (cat_index == (NUM_CAT-1)) ? 0 : (cat_index + 1);
                }

                feature_index = (feature_index < 39) ? (feature_index + 1) : 0;
            }
            else {
                output_data = input_data;
            }

            stream_out.write(output_data);            
            
        }

    }while(read_loop < 2);


    

    // MapDictAndPatch:
    // do {

    //     if (count_stream_in < input_bytes) {
    //         input_data = stream_in.read();
    //         if (hash_dict[input_data]== 0) {
    //             hash_dict[input_data] = 1;
    //             stream_out.write(input_data);
    //         }
    //         else {
    //             stream_out.write(patch_data);
    //         }
    //     }
    //     else {
    //         input_data = stream_in.read();
    //         stream_out.write(input_data);
    //     }

    //     count_stream_in++;

    // }while(count_stream_in < (2*input_bytes));
    
}

void CreateDict(
    hls::stream<ap_uint<64> >& stream_in,
    hls::stream<ap_uint<64> >& stream_out, 
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "CreateDict: " << std::endl;

    ap_uint<32> feature_dict[NUM_CAT][DICT_DEPTH];
    #pragma HLS bind_storage variable=feature_dict type=RAM_2P impl=BRAM latency=1
    #pragma HLS ARRAY_PARTITION variable=feature_dict dim=1 complete

    ap_uint<32> feature_count[NUM_CAT];
    #pragma HLS ARRAY_PARTITION variable=feature_count dim=1 complete

    ap_uint<64> input_data = 0;
    ap_uint<64> output_data = 0;
    ap_uint<64> end_of_text_data = 0xffffffff00000000;
    ap_uint<64> patch_data = 0xffffffffffffffff;

    ap_uint<32> output_feature = 0;
    int read_loop = 0;
    int dict_index = 0;
    int cat_index = 0;
    int feature_index = 0;

    InitializeDict:
    for (int i = 0; i < DICT_DEPTH; i++) {
        #pragma HLS pipeline II=1

        for (int j = 0; j < NUM_CAT; j++) {
            #pragma HLS unroll 
            feature_dict[j][i] = 0;
            feature_count[j] = 0;
        }
        
    }

    CreateDict:
    do {
        #pragma HLS pipeline II=1

        if (!stream_in.empty()) {
            input_data = stream_in.read();

            if (input_data == end_of_text_data) {
                read_loop++;
                output_data = end_of_text_data;
                cat_index = 0;
                feature_index = 0;
            }
            else if (read_loop == 0){
                if (feature_index < 14) {
                    output_data = input_data;
                }
                else {
                    if (input_data != patch_data) {
                        dict_index = input_data(31, 0);
                        feature_dict[cat_index][dict_index] = feature_count [cat_index];
                        feature_count[cat_index]++;
                    }

                    cat_index = (cat_index == (NUM_CAT-1)) ? 0 : (cat_index + 1);
                    output_data = input_data;
                }

                feature_index = (feature_index < 39) ? (feature_index + 1) : 0;

            }
            // The second loop, output the corresponding vocab
            else {
                if (feature_index < 14) {
                    output_data = input_data;
                }
                else {
                    dict_index = input_data(31, 0);
                    output_feature = feature_dict[cat_index][dict_index];

                    output_data(31, 0) = input_data(31, 0);
                    output_data(63, 32) = output_feature;

                    cat_index = (cat_index == (NUM_CAT-1)) ? 0 : (cat_index + 1);
                }

                feature_index = (feature_index < 39) ? (feature_index + 1) : 0;

            }

            stream_out.write(output_data);            
            
        }

    }while(read_loop < 2);  
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
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "StreamCombine: " << std::endl;

    StreamCombine:
    for (ull i = 0; i < input_bytes; i++) {
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
    hls::stream<ap_uint<64> >& stream_in,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "ConsumeStream_Num: " << std::endl;

    // int count_wait = 0;
    int feature_index = 0;
    // bool input_flag = false;
    int read_loop = 0;

    ap_uint<64> input_data = 0;
    ap_uint<64> output_data = 0;
    ap_uint<64> end_of_text_data = 0xffffffff00000000;

    ConsumeStream_Num:
    do {
        #pragma HLS pipeline II=1

        if (!stream_in.empty()) {

            input_data = stream_in.read();

            if (input_data == end_of_text_data) {
                read_loop++;
            }
        }
    }while(read_loop < 2);
}

void ConsumeStream_Cat(
    hls::stream<ap_uint<64> >& stream_in,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "ConsumeStream_Cat: " << std::endl;

    int read_loop = 0;
    int feature_index = 0;

    ap_uint<64> input_data = 0;
    ap_uint<64> output_data = 0;
    ap_uint<64> end_of_text_data = 0xffffffff00000000;

    ConsumeStream_Cat:
    do {
        #pragma HLS pipeline II=1
        
        if (!stream_in.empty()) {

            input_data = stream_in.read();

            if (input_data == end_of_text_data) {
                read_loop++;
            }
        }
    }while(read_loop < 2);

}

void ConsumeStream_byte(
    hls::stream<ap_uint<8> >& stream_in,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "ConsumeStream_byte: " << std::endl;

    int read_loop = 0;

    ap_uint<8> input_data = 0;
    ap_uint<8> output_data = 0;
    ap_uint<8> end_of_text = 3;


    ConsumeStream_byte:
    do {
        #pragma HLS pipeline II=1
        
        if (!stream_in.empty()) {

            input_data = stream_in.read();

            if (input_data == end_of_text) {
                read_loop++;
            }
        }
    }while(read_loop < 2);

}


