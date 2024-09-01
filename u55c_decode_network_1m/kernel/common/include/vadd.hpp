#include <hls_stream.h>
#include <iostream>
#include <hls_math.h>

#include "constants.hpp"

const unsigned int c_size_min = 1;
const unsigned int c_size_max = BURSTBUFFERSIZE_INT;

// extern "C" {

// void vadd(  
//     ap_uint<32>* table_DDR0,  
//     ap_uint<32>* table_DDR1,  
//     ap_uint<32>* table_DDR2, 
//     // ap_uint<32>* table_DDR3, 
//     ull input_bytes,
//     ull row_num,
//     // ull DDR_table_bytes,
//     int max_vocab_size
//     );
// }

void Network2Stream(
    hls::stream<ap_uint<512> >& s_data_in,
    hls::stream<ap_uint<16> >& s_data_out,
    ull input_bytes,
    int pkgWordCount,
    ull loop_num
) {
    #pragma HLS INLINE off

    std::cout << "Network2Stream: " << std::endl;

    ap_uint<512> data_input = 0;
    ap_uint<16> data_output = 0;

    ull output_count = 0;

    Network2Stream:
    for (int i = 0; i < 2*loop_num*pkgWordCount; i++) {
        for (int j = 0; j < 32; j++) {
            #pragma HLS pipeline II=1
            if (j==0) {
                data_input = s_data_in.read();
            }
            data_output = data_input(16*j+15, 16*j);
            s_data_out.write(data_output);

            output_count++;
            
        }
    }

    std::cout << "      output_count: " << output_count << std::endl;
    
}

void Network2StreamFilter(
    hls::stream<ap_uint<16> >& s_data_in,
    hls::stream<ap_uint<16> >& s_data_out,
    ull input_bytes,
    int pkgWordCount,
    ull loop_num
) {
    #pragma HLS INLINE off

    std::cout << "Network2StreamFilter: " << std::endl;

    ap_uint<16> data_input = 0;
    ap_uint<16> data_output = 0;
    ap_uint<16> end_of_text = 0x0303;

    ull loop_num_per_int = loop_num * pkgWordCount * 32 + 1;
    ull output_bytes_count = 0;

    ull tab_count = 0;
    ull new_line_count = 0;

    Network2StreamFilter:
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < loop_num_per_int; i++) {
            #pragma HLS pipeline II=1
            data_input = s_data_in.read();

            if (i == (loop_num_per_int-1)) {
                output_bytes_count = 0;
                s_data_out.write(end_of_text);
            }
            else {
                if (output_bytes_count < input_bytes) {
                    s_data_out.write(data_input);
                    output_bytes_count += 2;
                }
                
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
}

void LoadDDR_UTF8(
    ap_uint<32>* table_DDR0,
    ap_uint<32>* table_DDR1,
    ap_uint<32>* table_DDR2,
    // ap_uint<32>* table_DDR3,
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

    // In host, it is stored in 64-bit
    // Here, read in 32-bit
    ull dram_size_4bytes = total_bytes_per_4 / 2 / 3;
    ull dram_size_4bytes_0 = dram_size_4bytes * 2;
    ull dram_size_4bytes_1 = dram_size_4bytes_0;
    ull dram_size_4bytes_2 = loop_num - 1 - 2 * dram_size_4bytes_0;
    // ull dram_size_4bytes_3= loop_num - 1 - 3 * dram_size_4bytes;

    std::cout << "total_bytes_per_4: " << total_bytes_per_4 << ", total_bytes_per_4_remain: " << total_bytes_per_4_remain << ", loop_num: " << loop_num << ", input_bytes: " << input_bytes << std::endl; 
    std::cout << "dram_size_4bytes_0: " << dram_size_4bytes_0 << ", dram_size_4bytes_1: " << dram_size_4bytes_1 << ", dram_size_4bytes_2: " << dram_size_4bytes_2 << std::endl;

    LoadDDR:
    for (int j = 0; j < 2; j++) {
        for (ull i = 0; i < loop_num; i++) {
            #pragma HLS pipeline II=1
            if (i < dram_size_4bytes_0) {
                ddr_data = table_DDR0[i];
            }
            else if (i < 2 * dram_size_4bytes_0) {
                ddr_data = table_DDR1[i - dram_size_4bytes_0];
            }
            // else if (i < 3 * dram_size_4bytes) {
            //     ddr_data = table_DDR2[i - 2*dram_size_4bytes];
            // }
            else if (i < (loop_num - 1)) {
                ddr_data = table_DDR2[i - 2*dram_size_4bytes_0];
            }
            else {
                ddr_data = end_of_text;
            }
            stream_out.write(ddr_data); 

            // std::cout << "i: " << (2*i) << ", ";
            // for (int k = 0; k < 2; k++) {
            //     std::cout << std::hex << ddr_data(8*k+7, 8*k) << " " << std::dec;
            // }
            // std::cout << std::endl;

            // std::cout << "i: " << (2*i+1) << ", ";
            // for (int k = 2; k < 4; k++) {
            //     std::cout << std::hex << ddr_data(8*k+7, 8*k) << " " << std::dec;
            // }
            // std::cout << std::endl;
        }
    }
}

void Long2Byte(
    hls::stream<ap_uint<32> >& stream_in,
    hls::stream<ap_uint<16> >& stream_out,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "Long2Byte: " << std::endl; 

    ap_uint<32> end_of_text = 0x03030303;
    ap_uint<32> input_data = 0;
    ap_uint<16> sub_input_data[2];

    ull total_bytes_per_4 = input_bytes / 4;
    ull total_bytes_per_4_remain = input_bytes - total_bytes_per_4 * 4;
    ull loop_num = (total_bytes_per_4_remain > 0) ? (total_bytes_per_4 + 2) : (total_bytes_per_4 + 1);

    std::cout << "total_bytes_per_4: " << total_bytes_per_4 << ", total_bytes_per_4_remain: " << total_bytes_per_4_remain << ", loop_num: " << loop_num << ", input_bytes: " << input_bytes << std::endl; 

    Long2Byte:
    for (int j = 0; j < 2; j++) {
        for (ull i = 0; i < loop_num; i++) {
            #pragma HLS pipeline II=2
            input_data = stream_in.read();
            sub_input_data[0] = input_data(15, 0) ;
            sub_input_data[1] = input_data(31, 16);

            stream_out.write(sub_input_data[0]); 
            stream_out.write(sub_input_data[1]); 
        }
    }
}

void ByteFilter(
    hls::stream<ap_uint<16> >& stream_in,
    hls::stream<ap_uint<16> >& stream_out,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "ByteFilter: " << std::endl; 

    ap_uint<16> end_of_text = 0x0303;
    ap_uint<16> input_data = 0;
    ull bytes_count = 0;

    ull total_bytes_per_4 = input_bytes / 4;
    ull total_bytes_per_4_remain = input_bytes - total_bytes_per_4 * 4;
    ull loop_num = (total_bytes_per_4_remain > 0) ? (total_bytes_per_4 + 2) : (total_bytes_per_4 + 1);

    ull input_bytes_2 = (input_bytes & 0x0001 == 1) ? (input_bytes >> 1 + 1) : (input_bytes >> 1);

    std::cout << "total_bytes_per_4: " << total_bytes_per_4 << ", total_bytes_per_4_remain: " << total_bytes_per_4_remain << ", loop_num: " << loop_num << ", input_bytes: " << input_bytes << std::endl; 

    std::cout << "input_bytes_2: " << input_bytes_2 << ", last loop: " << (2*loop_num-1) << std::endl;

    ByteFilter:
    for (int j = 0; j < 2; j++) {
        for (ull i = 0; i < 2*loop_num; i++) {
            #pragma HLS pipeline II=1
            input_data = stream_in.read();
            
            if ((i < input_bytes_2) || (i == (2*loop_num - 1))) {
                stream_out.write(input_data); 
                bytes_count += 1;

                // std::cout << "i: " << i << ", " << std::hex << input_data(7, 0) << ", " << input_data(15, 8) << std::dec << std::endl;
                // std::cout << i << ", ";
            }
        }
        std::cout << std::endl << "bytes_count: " << bytes_count << std::endl;
    }
}

void DecodeASCII(
    hls::stream<ap_uint<16> >& stream_in,
    hls::stream<ap_uint<16> >& stream_out,
    // hls::stream<ap_uint<64> >& categorical_stream_0,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "DecodeASCII: " << std::endl;
    
    ap_uint<16> end_of_text = 0x0303;
    ap_uint<16> end_of_text_next = 0xffff;
    ap_uint<8> horizontal_tab = 9;
    ap_uint<8> new_line = 10;
    ap_uint<8> tab_code = 46;
    ap_uint<8> minus_sign = 45;

    ap_uint<4> feature_dict[16];

    ap_uint<8> output_data_0 = 0;
    ap_uint<8> output_data_1 = 0;
    ap_uint<16> output_data = 0;

    int read_loop = 0;
    int input_counter = 0;

    InitializeDict:
    for (int i = 0; i < 16; i++) {
        #pragma HLS pipeline II=1
        feature_dict[i] = i;
    }

    DecodeASCII:
    do {
        #pragma HLS pipeline II=1

        if (!stream_in.empty()) {
            ap_uint<16> input_data = stream_in.read();

            ap_uint<8> input_data_0 = input_data(7, 0);
            ap_uint<8> input_data_1 = input_data(15, 8);

            // std::cout << input_counter << " " << std::hex << input_data_0 << " " << input_data_1 << " " << std::dec << std::endl;
            // input_counter++;

            if (input_data == end_of_text) {
                stream_out.write(end_of_text_next);
                read_loop++;
            }
            else {
                bool input_tab_0 = (input_data_0 == horizontal_tab || input_data_0 == new_line);
                bool input_tab_1 = (input_data_1 == horizontal_tab || input_data_1 == new_line);

                // std::cout << "      input_tab_0: " << input_tab_0 << ", input_tab_1:  " << input_tab_1 << std::endl;
                
                if (input_tab_0) {
                    output_data_0 = tab_code;
                }
                else {
                    if (input_data_0 >= 48 && input_data_0 <= 57) {
                        output_data_0 = input_data_0 - 48;
                    }
                    // a~f (10~15)
                    else if (input_data_0 >= 97 && input_data_0 <= 102) {
                        output_data_0 = input_data_0 - 87;
                    }
                    else {
                        // minus sign
                        output_data_0 = input_data_0;
                    }
                }

                if (input_tab_1) {
                    output_data_1 = tab_code;
                }
                else {
                    if (input_data_1 >= 48 && input_data_1 <= 57) {
                        output_data_1 = input_data_1 - 48;
                    }
                    // a~f (10~15)
                    else if (input_data_1 >= 97 && input_data_1 <= 102) {
                        output_data_1 = input_data_1 - 87;
                    }
                    else {
                        output_data_1 = input_data_1;
                    }
                }

                output_data(7, 0) = output_data_0;
                output_data(15, 8) = output_data_1;
                stream_out.write(output_data);

            }
            
        }        
    }while(read_loop < 2);

}

void DecodeUTF8(
    hls::stream<ap_uint<16> >& stream_in,
    hls::stream<ap_uint<64> >& stream_out,
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
    
    bool input_flag = false;
    bool negative_flag = false;
    bool output_flag = false;
    
    ap_uint<16> end_of_text = 0xffff;
    ap_uint<8> horizontal_tab = 9;
    ap_uint<8> new_line = 10;
    ap_uint<8> tab_code = 46;
    ap_uint<8> minus_sign = 45;
    ap_uint<32> output_data_32 = 0;
    ap_uint<32> output_data_neg = 0;
    ap_uint<32> output_data_tmp = 0;
    ap_uint<64> output_data_64 = 0;
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;

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
            ap_uint<16> input_data = stream_in.read();

            ap_uint<8> input_data_0 = input_data(7, 0);
            ap_uint<8> input_data_1 = input_data(15, 8);

            // std::cout << std::dec << feature_index << std::hex << ": " << input_data_0 << ", " << input_data_1 << " -> ";

            if (input_data == end_of_text) {
                stream_out.write(end_of_text_data);
                // categorical_stream_0.write(end_of_text_data);
                read_loop++;
                feature_index = 0;
            }
            else {
                bool input_tab_0 = (input_data_0 == tab_code);
                bool input_tab_1 = (input_data_1 == tab_code);
                // tab + tab
                if (input_tab_0 && input_tab_1) {

                    output_data_neg = ~output_data_32 + 1;
                    output_data_64(31, 0) = negative_flag ? output_data_neg : output_data_32;
                    output_data_64(63, 32) = 0;
                    stream_out.write(output_data_64);

                    // if (feature_index < 14) {
                    //     std::cout << std::dec << output_data_64(31, 0) << " " << 0 << std::endl;
                    // }
                    // else {
                    //     std::cout << std::hex << output_data_64(31, 0) << " " << 0 << std::endl;
                    // }

                    negative_flag = false;
                    output_data_32 = 0; 
                    feature_index += 2; 
                }
                // tab + value
                else if (input_tab_0 && !input_tab_1) {
                    // it is possible for minus signal
                    // transfer one output
                    // feature_index += 1;
                    output_data_neg = ~output_data_32 + 1;
                    output_data_64(31, 0) = negative_flag ? output_data_neg : output_data_32;
                    
                    output_data_64(63, 32) = 0xffffffff;
                    stream_out.write(output_data_64);
                    // if (feature_index < 14) {
                    //     std::cout << std::dec << output_data_64(31, 0)  << std::endl;
                    // }
                    // else {
                    //     std::cout << std::hex << output_data_64(31, 0) << std::endl;
                    // }
                    
                    if (input_data_1 == minus_sign) {
                        negative_flag = true;
                        output_data_32 = 0;
                    }
                    else {
                        negative_flag = false;
                        output_data_32 = input_data_1;
                    }
                    // std::cout << "      " << std::hex << output_data_32 << std::endl;
                    
                    feature_index += 1;
                }
                // value + tab
                else if (!input_tab_0 && input_tab_1) {
                    // feature_index += 1;

                    output_data_tmp = (feature_index < 14) ? ap_uint<32>(output_data_32 * 10 + input_data_0) : ap_uint<32>((output_data_32 << 4) + input_data_0);

                    output_data_neg = ~output_data_tmp + 1;
                    output_data_64(31, 0) = negative_flag ? output_data_neg : output_data_tmp;
                    output_data_64(63, 32) = 0xffffffff;

                    stream_out.write(output_data_64);
                    // if (feature_index < 14) {
                    //     std::cout << std::dec << output_data_64(31, 0)  << std::endl;
                    // }
                    // else {
                    //     std::cout << std::hex << output_data_64(31, 0) << std::endl;
                    // }
                    
                    negative_flag = false;
                    output_data_32 = 0;
                    feature_index += 1;
                }
                // value + value
                else {
                    // no output
                    output_data_tmp = (feature_index < 14) ? ap_uint<32>(output_data_32 * 100 + input_data_0 * 10 + input_data_1) : ap_uint<32>((output_data_32 << 8) + (input_data_0 << 4) + input_data_1);

                    // output_data_64(31, 0) = negative_flag ? (~output_data_tmp + 1) : output_data_tmp;
                    // output_data_64(63, 32) = 0xffffffff;

                    // std::cout << std::endl;

                    // std::cout << "      " << std::hex << (output_data_32 << 8) << ", " << (input_data_0 << 4) << ", " << input_data_1 << std::endl;
                    // std::cout << "      " << std::hex << ((output_data_32 << 8) + (input_data_0 << 4)) << std::endl;

                    output_data_32 = output_data_tmp;
                    // std::cout << "      " << std::hex << output_data_32 << std::endl;
                    
                }

                if (feature_index >= 40) {
                    feature_index = feature_index - 40;
                }

                // if (output_flag) {
                //     output_flag = false;
                //     output_data_64(31, 0) = output_data_32;
                //     output_data_64(63, 32) = output_data_tmp;
                //     numerical_stream_0.write(output_data_64);
                // }
            }
            
        }        
    }while(read_loop < 2);

}

void DecodeFilter(
    hls::stream<ap_uint<64> >& stream_in,
    hls::stream<ap_uint<64> >& stream_out,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "DecodeFilter: " << std::endl;

    int read_loop = 0;
    
    bool second_output_flag = false;
    
    ap_uint<32> input_data_0 = 0;
    ap_uint<32> input_data_1 = 0;

    ap_uint<32> output_data_32 = 0;
    ap_uint<32> output_data_neg = 0;
    ap_uint<32> output_data_tmp = 0;
    ap_uint<64> output_data_64 = 0;

    ap_uint<64> end_of_text_data = 0xefffffffffffffff;

    int binary_counter = 0;

    DecodeFilter:
    do {
        #pragma HLS pipeline II=1

        if (second_output_flag) {
            second_output_flag = false;
            stream_out.write(0);
            // std::cout << 0 << std::endl;

            // binary_counter++;
        }
        else if (!stream_in.empty()) {
            ap_uint<64> input_data = stream_in.read();

            ap_uint<32> input_data_0 = input_data(31, 0);
            ap_uint<32> input_data_1 = input_data(63, 32);

            if (input_data == end_of_text_data) {
                stream_out.write(end_of_text_data);
                read_loop++;

                // std::cout << "binary_counter: " << binary_counter << std::endl;
            }
            else {
                if (input_data_1 == 0) {
                    second_output_flag = true;
                }
                
                // only (31, 0) is valid
                stream_out.write(input_data);
                // std::cout << std::hex << input_data_0 << std::endl;

                // binary_counter++;
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
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;

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
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;

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

void MapDict(
    hls::stream<ap_uint<64> >& stream_in,
    hls::stream<ap_uint<64> >& stream_out, 
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "MapDict: " << std::endl;

    ap_uint<64> hash_dict[NUM_CAT][DICT_DEPTH/64];
    #pragma HLS bind_storage variable=hash_dict type=RAM_1P impl=URAM latency=1
    #pragma HLS ARRAY_PARTITION variable=hash_dict dim=1 complete

    // int count_stream_in = 0;
    // int count_stream_out = 0;
    int read_loop = 0;
    int cat_index = 0;
    int feature_index = 0;
    ap_uint<32> dict_index = 0;
    ap_uint<32> dict_index_id = 0;
    ap_uint<32> dict_index_offset = 0;
    ap_uint<64> dict_data = 0;
    ap_uint<1> dict_data_selected = 0;
    ap_uint<64> input_data = 0;
    ap_uint<64> output_data = 0;
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;
    ap_uint<64> patch_data = 0xffffffffffffffff;

    // int count_wait = 0;
    // bool input_flag = false;

    InitializeDict:
    for (int i = 0; i < DICT_DEPTH/64; i++) {
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

                    dict_index_id = dict_index / 64;
                    dict_index_offset = dict_index & 0x3f;

                    dict_data = hash_dict[cat_index][dict_index_id];
                    dict_data_selected = dict_data(dict_index_offset, dict_index_offset);

                    std::cout << "    input_data: " << dict_index << ", id: " << dict_index_id << ", offset: " << dict_index_offset << std::endl;
                    std::cout << "          dict_data: " << dict_data;

                    if (dict_data_selected == 0) {
                        dict_data(dict_index_offset, dict_index_offset) = 1;
                        // stream_out.write(input_data);
                        hash_dict[cat_index][dict_index_id] = dict_data;
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

}

void CreateDict(
    hls::stream<ap_uint<64> >& stream_in,
    hls::stream<ap_uint<64> >& stream_out, 
    ap_uint<32>* feature_dict,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "CreateDict: " << std::endl;

    // ap_uint<32> feature_dict[NUM_CAT][DICT_DEPTH];
    // #pragma HLS bind_storage variable=feature_dict type=RAM_2P impl=BRAM latency=1
    // #pragma HLS ARRAY_PARTITION variable=feature_dict dim=1 complete

    ap_uint<32> feature_count[NUM_CAT];
    #pragma HLS ARRAY_PARTITION variable=feature_count dim=1 complete

    ap_uint<64> input_data = 0;
    ap_uint<64> output_data = 0;
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;
    ap_uint<64> patch_data = 0xffffffffffffffff;

    ap_uint<32> output_feature = 0;
    int read_loop = 0;
    int dict_index = 0;
    int cat_index = 0;
    int ddr_index = 0;
    int feature_index = 0;

    ull stream_count = 0;

    InitializeDict:
    for (int j = 0; j < NUM_CAT; j++) {
        #pragma HLS unroll 
        // feature_dict[j][i] = 0;
        feature_count[j] = 0;
    }
    // for (int i = 0; i < DICT_DEPTH; i++) {
    //     #pragma HLS pipeline II=1

    //     for (int j = 0; j < NUM_CAT; j++) {
    //         #pragma HLS unroll 
    //         // feature_dict[j][i] = 0;
    //         feature_count[j] = 0;
    //     }
        
    // }

    CreateDict:
    do {
        #pragma HLS pipeline II=1
        #pragma HLS DEPENDENCE variable=feature_dict inter false

        if (!stream_in.empty()) {
            input_data = stream_in.read();

            if (input_data == end_of_text_data) {
                read_loop++;
                output_data = end_of_text_data;
                cat_index = 0;
                feature_index = 0;

                stream_out.write(end_of_text_data); 

                std::cout << "read_loop: " << read_loop << ", output stream count: " << stream_count << std::endl;
                
            }
            else if (read_loop == 0){
                if (feature_index < 14) {
                    output_data = input_data;
                }
                else {
                    if (input_data != patch_data) {
                        dict_index = input_data(31, 0);
                        ddr_index = cat_index * DICT_DEPTH + dict_index;
                        feature_dict[ddr_index] = feature_count[cat_index];
                        // feature_dict[cat_index][dict_index] = feature_count[cat_index];
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
                    ddr_index = cat_index * DICT_DEPTH + dict_index;
                    output_feature = feature_dict[ddr_index];//feature_dict[cat_index][dict_index];

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
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;

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
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;

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

void ConsumeStream(
    hls::stream<ap_uint<64> >& stream_in,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "ConsumeStream: " << std::endl;

    int read_loop = 0;
    ap_uint<64> input_data = 0;
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;

    ConsumeStream:
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
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;

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
