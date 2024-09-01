#include <hls_stream.h>
#include <iostream>
#include <hls_math.h>

#include "constants.hpp"

const unsigned int c_size_min = 1;
const unsigned int c_size_max = BURSTBUFFERSIZE_INT;

// extern "C" {

// void vadd(  
//     ap_uint<512>* table_DDR0,  
//     // ap_uint<512>* table_DDR1,  
//     // ap_uint<512>* table_DDR2, 
//     ull input_bytes,
//     ull row_num,
//     ull DDR_table_bytes,
//     int max_vocab_size
//     );
// }

void Network2Stream(
    hls::stream<ap_uint<512> >& s_data_in,
    hls::stream<ap_uint<8> >& s_data_out,
    ull input_bytes,
    int pkgWordCount,
    ull loop_num
) {
    #pragma HLS INLINE off

    std::cout << "Network2Stream: " << std::endl;

    ap_uint<512> data_input = 0;
    ap_uint<8> data_output = 0;

    ull output_count = 0;

    // ull input_bytes_per_64 = input_bytes / 64;
    // ull input_bytes_per_64_remain = input_bytes - 64 * input_bytes_per_64;
    // ull loop_num = (input_bytes_per_64_remain > 0) ? (input_bytes_per_64 + 1) : input_bytes_per_64;

    Network2Stream:
    for (int i = 0; i < 2*loop_num*pkgWordCount; i++) {
        for (int j = 0; j < 64; j++) {
            #pragma HLS pipeline II=1
            if (j==0) {
                data_input = s_data_in.read();
            }
            data_output = data_input(8*j+7, 8*j);
            s_data_out.write(data_output);

            output_count++;
            
        }
    }

    std::cout << "      output_count: " << output_count << std::endl;
    
}

void Network2StreamFilter(
    hls::stream<ap_uint<8> >& s_data_in,
    hls::stream<ap_uint<8> >& s_data_out,
    ull input_bytes,
    int pkgWordCount,
    ull loop_num
) {
    #pragma HLS INLINE off

    std::cout << "Network2StreamFilter: " << std::endl;

    ap_uint<8> data_input = 0;
    ap_uint<8> data_output = 0;

    // ull input_bytes_per_64 = input_bytes / 64;
    // ull input_bytes_per_64_remain = input_bytes - 64 * input_bytes_per_64;
    // ull loop_num = (input_bytes_per_64_remain > 0) ? (input_bytes_per_64 + 1) : input_bytes_per_64;

    ull loop_num_per_byte = loop_num * pkgWordCount * 64;
    ull output_bytes_count = 0;

    ull tab_count = 0;
    ull new_line_count = 0;

    Network2StreamFilter:
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < loop_num_per_byte; i++) {
            #pragma HLS pipeline II=1
            data_input = s_data_in.read();

            if (output_bytes_count < input_bytes) {
                s_data_out.write(data_input);
            }

            // if (data_input == 9) {
            //     tab_count++;
            //     // std::cout << " ";
            // }
            // else if (data_input == 10) {
            //     new_line_count++;
            //     // std::cout << std::endl;
            // }
            // else {
            //     // std::cout << std::hex << data_input << std::dec;
            // }

            if (i == (loop_num_per_byte-1)) {
                // std::cout << "  End of one loop, output_bytes_count: " << output_bytes_count << ", input_bytes: " << input_bytes << ", tab_count: " << tab_count << ", new_line_count: " << new_line_count << std::endl;
                output_bytes_count = 0;
            }
            else {
                output_bytes_count++;
            }
        }
    }
    
}

void Stream2Network(
    hls::stream<ap_uint<64> >& stream_in,
    hls::stream<ap_uint<512> >& s_data_out,
    ull row_num
) {
    #pragma HLS INLINE off

    std::cout << "Stream2Network: " << std::endl;

    ap_uint<64> input_data = 0;
    ap_uint<64> end_of_text_data = 0xffffffff00000000;

    Stream2Network:
    do{
        #pragma HLS pipeline II=1
        if (!stream_in.empty()) {
            input_data = stream_in.read();
            if (input_data==end_of_text_data) {
                break;
            }
        }
    } while(true);

    // // int read_loop = 0;
    // // int burstbuffer[BURSTBUFFERSIZE_INT];

    // ap_uint<64> input_data = 0;
    // // ap_uint<32> output_data = 0;
    // // ap_uint<64> end_of_text_data = 0xffffffff00000000;

    // // ull output_bytes = row_num * 40 * 4;
    // ull output_int = row_num * 40;
    // // ull DDR_table_int = DDR_table_bytes / 4;

    // // std::cout << "Row number: " << row_num << std::endl;
    // // std::cout << "Input bytes: " << input_bytes << ", Output bytes: " << output_bytes << std::endl;
    // // std::cout << "Number of Output Integer: " << output_int << std::endl;
    // // std::cout << "DDR_table_bytes: " << DDR_table_bytes << ", DDR_table_int: " << DDR_table_int << std::endl;

    // Stream2Network:
    // for (ull i = 0; i < output_int; i++) {
    //     #pragma HLS pipeline II=1

    //     input_data = stream_in.read();
        
    // }
    
}

void LoadDDR_UTF8_512(
    ap_uint<512>* table_DDR0,
    hls::stream<ap_uint<512> >& stream_out,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "LoadDDR_UTF8_512: " << std::endl; 

    ap_uint<32> end_of_text = 0x03030303;
    ap_uint<512> ddr_data = 0;

    ull total_bytes_per_64 = input_bytes / 64;
    ull total_bytes_per_64_remain = input_bytes - total_bytes_per_64 * 64;
    ull loop_num = (total_bytes_per_64_remain > 0) ? (total_bytes_per_64 + 1) : total_bytes_per_64;

    std::cout << "total_bytes_per_64: " << total_bytes_per_64 << ", total_bytes_per_64_remain: " << total_bytes_per_64_remain << ", loop_num: " << loop_num << ", input_bytes: " << input_bytes << std::endl; 

    LoadDDR:
    for (int j = 0; j < 2; j++) {
        for (ull i = 0; i < loop_num; i++) {
            #pragma HLS pipeline II=1
            ddr_data = table_DDR0[i];
            stream_out.write(ddr_data); 
            // std::cout << std::hex << ddr_data << std::dec << std::endl;
        }
    }
}

void LoadDDR_UTF8(
    ap_uint<32>* table_DDR0,
    ap_uint<32>* table_DDR1,
    ap_uint<32>* table_DDR2,
    hls::stream<ap_uint<32> >& stream_out,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "LoadDDR_UTF8: " << std::endl; 

    ap_uint<32> end_of_text = 0x03030303;
    ap_uint<32> ddr_data = 0;

    ull total_bytes_per_4 = input_bytes / 4;
    ull total_bytes_per_4_remain = input_bytes - total_bytes_per_4 * 4;
    ull loop_num = (total_bytes_per_4_remain > 0) ? (total_bytes_per_4 + 2) : (total_bytes_per_4 + 1);

    ull dram_size_4bytes = total_bytes_per_4 / 3;
    ull dram_size_4bytes_0 = dram_size_4bytes;
    ull dram_size_4bytes_1 = dram_size_4bytes;
    ull dram_size_4bytes_2 = loop_num - 1 - 2 * dram_size_4bytes;

    std::cout << "total_bytes_per_4: " << total_bytes_per_4 << ", total_bytes_per_4_remain: " << total_bytes_per_4_remain << ", loop_num: " << loop_num << ", input_bytes: " << input_bytes << std::endl; 
    std::cout << "dram_size_4bytes_0: " << dram_size_4bytes_0 << ", dram_size_4bytes_1: " << dram_size_4bytes_1 << ", dram_size_4bytes_2: " << dram_size_4bytes_2 << std::endl;

    LoadDDR:
    for (int j = 0; j < 2; j++) {
        for (ull i = 0; i < loop_num; i++) {
            #pragma HLS pipeline II=1
            if (i < dram_size_4bytes) {
                ddr_data = table_DDR0[i];
            }
            else if (i < 2 * dram_size_4bytes) {
                ddr_data = table_DDR1[i - dram_size_4bytes];
            }
            else if (i < (loop_num - 1)) {
                ddr_data = table_DDR2[i - 2*dram_size_4bytes];
            }
            else {
                ddr_data = end_of_text;
            }
            stream_out.write(ddr_data); 
        }
    }
}

void Long2Byte(
    hls::stream<ap_uint<32> >& stream_in,
    hls::stream<ap_uint<8> >& stream_out,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "Long2Byte: " << std::endl; 

    ap_uint<32> end_of_text = 0x03030303;
    ap_uint<32> input_data = 0;
    ap_uint<8> sub_input_data[4];

    ull total_bytes_per_4 = input_bytes / 4;
    ull total_bytes_per_4_remain = input_bytes - total_bytes_per_4 * 4;
    ull loop_num = (total_bytes_per_4_remain > 0) ? (total_bytes_per_4 + 2) : (total_bytes_per_4 + 1);

    std::cout << "total_bytes_per_4: " << total_bytes_per_4 << ", total_bytes_per_4_remain: " << total_bytes_per_4_remain << ", loop_num: " << loop_num << ", input_bytes: " << input_bytes << std::endl; 

    Long2Byte:
    for (int j = 0; j < 2; j++) {
        for (ull i = 0; i < loop_num; i++) {
            #pragma HLS pipeline II=4
            input_data = stream_in.read();
            sub_input_data[0] = input_data(7, 0)  ;
            sub_input_data[1] = input_data(15, 8) ;
            sub_input_data[2] = input_data(23, 16);
            sub_input_data[3] = input_data(31, 24);
            // sub_input_data[4] = input_data(39, 32);
            // sub_input_data[5] = input_data(47, 40);
            // sub_input_data[6] = input_data(55, 48);
            // sub_input_data[7] = input_data(63, 56);

            stream_out.write(sub_input_data[0]); 
            stream_out.write(sub_input_data[1]); 
            stream_out.write(sub_input_data[2]); 
            stream_out.write(sub_input_data[3]); 
            // stream_out.write(sub_input_data[4]); 
            // stream_out.write(sub_input_data[5]); 
            // stream_out.write(sub_input_data[6]); 
            // stream_out.write(sub_input_data[7]); 
        }
    }
}

void ByteFilter(
    hls::stream<ap_uint<8> >& stream_in,
    hls::stream<ap_uint<8> >& stream_out,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "ByteFilter: " << std::endl; 

    ap_uint<8> end_of_text = 0x03;
    ap_uint<8> input_data = 0;
    ull bytes_count = 0;

    ull total_bytes_per_4 = input_bytes / 4;
    ull total_bytes_per_4_remain = input_bytes - total_bytes_per_4 * 4;
    ull loop_num = (total_bytes_per_4_remain > 0) ? (total_bytes_per_4 + 2) : (total_bytes_per_4 + 1);

    std::cout << "total_bytes_per_4: " << total_bytes_per_4 << ", total_bytes_per_4_remain: " << total_bytes_per_4_remain << ", loop_num: " << loop_num << ", input_bytes: " << input_bytes << std::endl; 

    ByteFilter:
    for (int j = 0; j < 2; j++) {
        for (ull i = 0; i < 4*loop_num; i++) {
            #pragma HLS pipeline II=1
            input_data = stream_in.read();
            
            if ((i < input_bytes) || (i == (4*loop_num - 1))) {
                stream_out.write(input_data); 
                bytes_count += 1;
                // std::cout << i << ", ";
            }
        }
        std::cout << std::endl << "bytes_count: " << bytes_count << std::endl;
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
    
    ull input_bytes_count = 0;
    ull output_count = 0;
    ull new_line_count = 0;
    ull horizontal_tab_count = 0;

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

        if (input_bytes_count == input_bytes) {
            numerical_stream_0.write(end_of_text_data);
            // categorical_stream_0.write(end_of_text_data);
            read_loop++;
            input_bytes_count = 0;
            feature_index = 0;
            output_count++;
            std::cout << "    End of One Loop, output_count: " << output_count << ",new_line_count: " << new_line_count << ", horizontal_tab_count: " <<horizontal_tab_count << std::endl;
        }
        else if (!stream_in.empty()) {
            ap_uint<8> input_data = stream_in.read();
            // std::cout << "input_data: " << input_data << std::endl;
            // input_flag = true;
            // count_wait = 0;
            input_bytes_count++;

            if (input_data == horizontal_tab || input_data == new_line) {
                horizontal_tab_count++;
            }
            else if (input_data == new_line) {
                new_line_count++;
            }

            // if (input_bytes_count == input_bytes) {
            //     numerical_stream_0.write(end_of_text_data);
            //     // categorical_stream_0.write(end_of_text_data);
            //     read_loop++;
            //     input_bytes_count = 0;
            //     feature_index = 0;
            //     output_count++;
            //     std::cout << "    End of One Loop, output_count: " << output_count << ", new_line_count: " << new_line_count << ", horizontal_tab_count: " << horizontal_tab_count << std::endl;
            // }
            if (input_data == horizontal_tab || input_data == new_line) {
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

                output_count++;

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

    ull stream_count = 0;

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

            stream_count++;
            if (read_loop==2) {
                std::cout << "      total stream counts: " << stream_count << std::endl;
            }         
            
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

    ull stream_count = 0;

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

                if (read_loop == 2) {
                    stream_out.write(output_data);

                    stream_count++;
                    std::cout << "read_loop: " << read_loop << ", output stream count: " << stream_count << std::endl;
                }
                
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

                // stream_out.write(output_data);
                // stream_count++;

            }
            // The second loop, output the corresponding vocab
            else {
                if (feature_index < 14) {
                    output_data = input_data;
                }
                else {
                    dict_index = input_data(31, 0);
                    output_feature = feature_dict[cat_index][dict_index];

                    output_data(31, 0) = output_feature;
                    output_data(63, 32) = 0;

                    cat_index = (cat_index == (NUM_CAT-1)) ? 0 : (cat_index + 1);
                }

                feature_index = (feature_index < 39) ? (feature_index + 1) : 0;

                stream_out.write(output_data);    

                stream_count++;        
            }

            
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

void StoreDDR(
    hls::stream<ap_uint<64> >& stream_in,
    ap_uint<32>* table_DDR0,
    ap_uint<32>* table_DDR1,
    ap_uint<32>* table_DDR2,
    ull input_bytes,
    ull row_num,
    ull DDR_table_bytes
) {
    #pragma HLS INLINE off

    std::cout << "StoreDDR: " << std::endl; 

    int read_loop = 0;
    int burstbuffer[BURSTBUFFERSIZE_INT];

    ap_uint<64> input_data = 0;
    ap_uint<32> output_data = 0;
    ap_uint<64> end_of_text_data = 0xffffffff00000000;

    ull output_bytes = row_num * 40 * 4;
    ull output_int = row_num * 40;
    ull DDR_table_int = DDR_table_bytes / 4;

    ull stream_input_count = 0;
    ull ddr_output_count = 0;

    std::cout << "Row number: " << row_num << std::endl;
    std::cout << "Input bytes: " << input_bytes << ", Output bytes: " << output_bytes << std::endl;
    std::cout << "Number of Output Integer: " << output_int << std::endl;
    std::cout << "DDR_table_bytes: " << DDR_table_bytes << ", DDR_table_int: " << DDR_table_int << std::endl;

    // ConsumeStream:
    // for (ull i = 0; i < output_int; i++) {
    //     #pragma HLS pipeline II=1
    //     input_data = stream_in.read();
    //     ddr_output_count++;
        
    // }

    StoreDDR:
    for (ull i = 0; i < output_int; i++) {
        #pragma HLS pipeline II=1
        // int chunk_size = BURSTBUFFERSIZE_INT;
        // if ((i + BURSTBUFFERSIZE_INT) > output_int) chunk_size = output_int - i;

        // for (int j = 0; j < chunk_size; j++) {
        //     // As the outer loop is not a perfect loop
        //     #pragma HLS loop_flatten off
        //     #pragma HLS LOOP_TRIPCOUNT min = c_size_min max = c_size_max
        input_data = stream_in.read();
        if (i < DDR_table_int) {
            table_DDR0[i] = input_data;
        }
        else if (i < 2 * DDR_table_int) {
            table_DDR1[i-DDR_table_int] = input_data;
        }
        else {
            table_DDR2[i-2*DDR_table_int] = input_data;
        }
        ddr_output_count++;
        // }
        
    }

    std::cout << "stream_input_count: " << stream_input_count << ", ddr_output_count: " << ddr_output_count << std::endl;
}

void ConsumeStream(
    hls::stream<ap_uint<64> >& stream_in,
    ull input_bytes,
    ull row_num,
    ull DDR_table_bytes
) {
    #pragma HLS INLINE off

    std::cout << "ConsumeStream: " << std::endl; 

    int read_loop = 0;
    int burstbuffer[BURSTBUFFERSIZE_INT];

    ap_uint<64> input_data = 0;
    ap_uint<32> output_data = 0;
    ap_uint<64> end_of_text_data = 0xffffffff00000000;

    ull output_bytes = row_num * 40 * 4;
    ull output_int = row_num * 40;
    ull DDR_table_int = DDR_table_bytes / 4;

    ull stream_input_count = 0;
    ull ddr_output_count = 0;

    std::cout << "Row number: " << row_num << std::endl;
    std::cout << "Input bytes: " << input_bytes << ", Output bytes: " << output_bytes << std::endl;
    std::cout << "Number of Output Integer: " << output_int << std::endl;
    std::cout << "DDR_table_bytes: " << DDR_table_bytes << ", DDR_table_int: " << DDR_table_int << std::endl;

    ConsumeStream:
    for (ull i = 0; i < output_int; i++) {
        #pragma HLS pipeline II=1

        input_data = stream_in.read();
        
    }
}