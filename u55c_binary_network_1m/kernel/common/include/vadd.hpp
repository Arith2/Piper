#include <hls_stream.h>
#include <iostream>
#include <hls_math.h>

#include "constants.hpp"

void GenerateTraffic(
    hls::stream<ap_uint<512> >& stream_in,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "GenerateTraffic: " << std::endl;

    ap_uint<512> feature_0_512 = 0;
    ap_uint<512> feature_1_512 = 0;
    ap_uint<512> feature_2_512 = 0;


    GenerateTraffic:
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < row_num; i++) {
            #pragma HLS pipeline II=3
            ap_uint<32> feature_0 = 4 * i;
            ap_uint<32> feature_1 = 4 * i + 1;
            ap_uint<32> feature_2 = 4 * i + 2;

            for (int k = 0; k < 16; k++) {
                #pragma HLS unroll
                feature_0_512(32*k+31, 32*k) = feature_0;
                feature_1_512(32*k+31, 32*k) = feature_1;
                feature_2_512(32*k+31, 32*k) = feature_2;
            }

            stream_in.write(feature_0_512);  
            stream_in.write(feature_1_512);  
            stream_in.write(feature_2_512);  
        }
    }
}

void Network2Stream(
    hls::stream<ap_uint<512> >& s_data_in,
    hls::stream<ap_uint<512> >& input_stream_0,
    hls::stream<ap_uint<512> >& input_stream_1,
    hls::stream<ap_uint<512> >& input_stream_2,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "Network2Stream: " << std::endl;

    int row_num_count = 0;
    int stream_index = 0;

    Network2Stream:
    do{
        #pragma HLS pipeline II=1

        if (!s_data_in.empty()) {
            ap_uint<512> input_data = s_data_in.read();

            if (stream_index==0) {
                input_stream_0.write(input_data);
                stream_index = 1;
            }
            else if (stream_index==1) {
                input_stream_1.write(input_data);
                stream_index = 2;
            }
            else {
                input_stream_2.write(input_data);
                stream_index = 0;

                row_num_count++;
            }
        }
    }while(row_num_count < 2*row_num);
    
}

void Stream2Network(
    hls::stream<ap_uint<512> >& output_stream_0,
    hls::stream<ap_uint<512> >& output_stream_1,
    hls::stream<ap_uint<512> >& output_stream_2,
    hls::stream<ap_uint<512> >& s_data_out,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "Stream2Network: " << std::endl;

    int row_num_count = 0;
    int stream_index = 0;

    Stream2Network:
    do{
        #pragma HLS pipeline II=1
        if (stream_index==0 && !output_stream_0.empty()) {
            ap_uint<512> output_data_0 = output_stream_0.read();
            s_data_out.write(output_data_0);
            stream_index = 1;
        }
        else if (stream_index==1 && !output_stream_1.empty()) {
            ap_uint<512> output_data_1 = output_stream_1.read();
            s_data_out.write(output_data_1);
            stream_index = 2;
        }
        else if (stream_index==2 && !output_stream_2.empty()) {
            ap_uint<512> output_data_2 = output_stream_2.read();
            s_data_out.write(output_data_2);
            stream_index = 0;
            row_num_count++;
        }
    }while(row_num_count < 2*row_num);
    
}

void NegsToZeroLog(
    hls::stream<ap_uint<512> >& stream_in,
    hls::stream<ap_uint<512> >& stream_out,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "NegsToZeroLog: " << std::endl;

    NegsToZeroLog:
    for (int j = 0; j < 2; j++) {
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

                // std::cout << "log(" << (tmp_value+1) << ")=" << conv_value.float32 << "(" <<     conv_value.uint32 << ") ";
            }
            // std::cout << std::endl;

            // if (j==1) {
            //     stream_out.write(output_data);  
            // }
            stream_out.write(output_data);  
        }
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

template <const int ID>
void HashDict(
    hls::stream<ap_uint<32> >& stream_in,
    hls::stream<ap_uint<32> >& stream_out,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "HashDict: " << ID << std::endl;

    ap_uint<64> hash_dict[DICT_DEPTH/64];
    #pragma HLS bind_storage variable=hash_dict type=RAM_1P impl=URAM latency=1

    int count_stream_in = 0;
    int count_stream_out = 0;
    ap_uint<32> input_data;
    ap_uint<32> patch_data = 0xffffffff;

    ap_uint<32> input_data_id = 0;
    ap_uint<32> input_data_offset = 0;

    ap_uint<64> dict_data = 0;
    ap_uint<1> dict_data_selected = 0;

    InitializeDict:
    for (int i = 0; i < DICT_DEPTH/64; i++) {
        #pragma HLS pipeline II=1
        hash_dict[i] = 0;
    }

    HashDict:
    do {
        #pragma HLS pipeline II=2

        if (count_stream_in < row_num) {
            input_data = stream_in.read();

            input_data_id = input_data / 64;
            input_data_offset = input_data & 0x3f;

            std::cout << "    input_data: " << input_data << ", id: " << input_data_id << ", offset: " << input_data_offset << std::endl;
            
            dict_data = hash_dict[input_data_id];
            dict_data_selected = dict_data(input_data_offset, input_data_offset);

            std::cout << "          dict_data: " << dict_data;

            if (dict_data_selected == 0) {

                dict_data(input_data_offset, input_data_offset) = 1;

                hash_dict[input_data] = dict_data;
                stream_out.write(input_data);

                std::cout << ", change to " << dict_data << std::endl;
            }
            else {
                stream_out.write(patch_data);

                std::cout << ", no change" << std::endl;
            }

            
        }
        else {
            input_data = stream_in.read();
            stream_out.write(input_data);
        }

        count_stream_in++;

    }while(count_stream_in < (2*row_num));
    
}

template <const int ID>
void HashDictBypass(
    hls::stream<ap_uint<32> >& stream_in,
    hls::stream<ap_uint<32> >& stream_out,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "HashDictBypass: " << ID << std::endl;

    int count_stream_in = 0;
    int count_stream_out = 0;
    ap_uint<32> input_data;

    HashDictBypass:
    do {
        #pragma HLS pipeline II=1
        if (!stream_in.empty()) {
            input_data = stream_in.read();
            stream_out.write(input_data);
            count_stream_in++;
        }
    }while(count_stream_in < (2*row_num));
    
}

template <const int ID>
void CreateDict(
    hls::stream<ap_uint<32> >& stream_in,
    hls::stream<ap_uint<32> >& stream_out,
    ap_uint<32>* feature_dict,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "CreateDict: " << ID << std::endl;

    // ap_uint<32> feature_dict[DICT_DEPTH];
    // #pragma HLS bind_storage variable=feature_dict type=RAM_1P impl=BRAM latency=1
    ap_uint<32> patch_data = 0xffffffff;
    ap_uint<32> output_feature = 0;
    int feature_count = 0;

    CreateDict:
    for (int i = 0; i < 2*row_num; i++) {
        #pragma HLS pipeline II=1
        #pragma HLS DEPENDENCE variable=feature_dict inter false
        ap_uint<32> input_data = stream_in.read();
        
        if (i < row_num && input_data!=patch_data) {
            feature_dict[input_data] = feature_count;
            feature_count++;
            stream_out.write(input_data);
            // std::cout << input_data << ": " << i << std::endl;;
        }
        else if (i >= row_num) {
            output_feature = feature_dict[input_data];
            stream_out.write(output_feature);
            // std::cout << input_data << "->" << output_feature << std::endl;
        }
        else {
            // discard invalid data;
            stream_out.write(input_data);
        }
        
        
    }
    
}

template <const int ID>
void CreateDictBypass(
    hls::stream<ap_uint<32> >& stream_in,
    hls::stream<ap_uint<32> >& stream_out,
    int row_num
) {
    #pragma HLS INLINE off

    std::cout << "CreateDictBypass: " << ID << std::endl;

    ap_uint<32> patch_data = 0xffffffff;
    ap_uint<32> output_feature = 0;
    int feature_count = 0;

    CreateDictBypass:
    for (int i = 0; i < 2*row_num; i++) {
        #pragma HLS pipeline II=1
        ap_uint<32> input_data = stream_in.read();
        
        // if (i >= row_num) {
        //     stream_out.write(input_data);
        // }       
        stream_out.write(input_data); 
        
    }
    
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
    for (int i = 0; i < 2*row_num; i++) {
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
