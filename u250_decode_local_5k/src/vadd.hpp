#include <hls_stream.h>
#include <iostream>
#include <hls_math.h>

#include "constants.hpp"

const unsigned int c_size_min = 1;
const unsigned int c_size_max = BURSTBUFFERSIZE_INT;

extern "C" {

void vadd(  
    ap_uint<32>* table_DDR0,  
    ap_uint<32>* table_DDR1,  
    ap_uint<32>* table_DDR2, 
    // ap_uint<32>* table_DDR3, 
    ull input_bytes,
    ull row_num,
    // ull DDR_table_bytes,
    int max_vocab_size
    );
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

    ull total_bytes_per_8 = input_bytes >> 3;
    ull total_bytes_per_4 = input_bytes >> 2;

    ull loop_num = ((input_bytes & 0x03) > 0) ? (total_bytes_per_4 + 2) : (total_bytes_per_4 + 1);

    ull dram_size_8bytes = total_bytes_per_8 / 3;
    ull dram_size_4bytes = dram_size_8bytes * 2;

    ull dram_size_4bytes_0 = dram_size_4bytes;
    ull dram_size_4bytes_1 = dram_size_4bytes;
    ull dram_size_4bytes_2 = loop_num - 1 - 2 * dram_size_4bytes;

    std::cout << "total_bytes_per_8: " << total_bytes_per_8 << ", total_bytes_per_4: " << total_bytes_per_4 << ", dram_size_8bytes: " << dram_size_8bytes << ", dram_size_4bytes: " << dram_size_4bytes << std::endl;

    std::cout << "loop_num: " << loop_num << ", dram_size_4bytes_0: " << dram_size_4bytes_0 << ", dram_size_4bytes_1: " << dram_size_4bytes_1 << ", dram_size_4bytes_2: " << dram_size_4bytes_2 << std::endl;


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
            // // else if (i < 3 * dram_size_8bytes) {
            // //     ddr_data = table_DDR2[i - 2*dram_size_8bytes];
            // // }
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


void DecodeASCII(
    hls::stream<ap_uint<32> >& stream_in,
    hls::stream<ap_uint<32> >& stream_out,
    // hls::stream<ap_uint<64> >& categorical_stream_0,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "DecodeASCII: " << std::endl;
    
    ap_uint<32> end_of_text = 0x03030303;
    ap_uint<32> end_of_text_next = 0xffffffff;
    ap_uint<8> horizontal_tab = 9;
    ap_uint<8> new_line = 10;
    ap_uint<8> tab_code = 46;
    ap_uint<8> minus_sign = 45;

    ap_uint<4> feature_dict[16];

    ap_uint<8> output_data_0 = 0;
    ap_uint<8> output_data_1 = 0;
    ap_uint<8> output_data_2 = 0;
    ap_uint<8> output_data_3 = 0;
    ap_uint<32> output_data = 0;

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
            ap_uint<32> input_data = stream_in.read();

            ap_uint<8> input_data_0 = input_data(7, 0);
            ap_uint<8> input_data_1 = input_data(15, 8);
            ap_uint<8> input_data_2 = input_data(23, 16);
            ap_uint<8> input_data_3 = input_data(31, 24);

            // std::cout << read_loop << " " << std::hex << input_data_0 << " " << input_data_1 << " " << input_data_2 << " " << input_data_3 << " " << std::dec << std::endl;
            // input_counter++;

            if (input_data == end_of_text) {
                stream_out.write(end_of_text_next);
                output_data_0 = 0;
                read_loop++;
            }
            else {
                bool input_tab_0 = (input_data_0 == horizontal_tab || input_data_0 == new_line);
                bool input_tab_1 = (input_data_1 == horizontal_tab || input_data_1 == new_line);
                bool input_tab_2 = (input_data_2 == horizontal_tab || input_data_2 == new_line);
                bool input_tab_3 = (input_data_3 == horizontal_tab || input_data_3 == new_line);

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

                if (input_tab_2) {
                    output_data_2 = tab_code;
                }
                else {
                    if (input_data_2 >= 48 && input_data_2 <= 57) {
                        output_data_2 = input_data_2 - 48;
                    }
                    // a~f (10~15)
                    else if (input_data_2 >= 97 && input_data_2 <= 102) {
                        output_data_2 = input_data_2 - 87;
                    }
                    else {
                        // minus sign
                        output_data_2 = input_data_2;
                    }
                }

                if (input_tab_3) {
                    output_data_3 = tab_code;
                }
                else {
                    if (input_data_3 >= 48 && input_data_3 <= 57) {
                        output_data_3 = input_data_3 - 48;
                    }
                    // a~f (10~15)
                    else if (input_data_3 >= 97 && input_data_3 <= 102) {
                        output_data_3 = input_data_3 - 87;
                    }
                    else {
                        output_data_3 = input_data_3;
                    }
                }

                output_data(7, 0) = output_data_0;
                output_data(15, 8) = output_data_1;
                output_data(23, 16) = output_data_2;
                output_data(31, 24) = output_data_3;
                stream_out.write(output_data);

            }
            
        }        
    }while(read_loop < 2);

}

void DecodeUTF8(
    hls::stream<ap_uint<32> >& stream_in,
    hls::stream<ap_uint<128> >& stream_out,
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
    
    ap_uint<32> end_of_text = 0xffffffff;
    ap_uint<8> horizontal_tab = 9;
    ap_uint<8> new_line = 10;
    ap_uint<8> tab_code = 46;
    ap_uint<8> minus_sign = 45;

    ap_uint<32> output_data_32_0 = 0;
    ap_uint<32> output_data_32_1 = 0;
    ap_uint<32> output_data_32_2 = 0;
    ap_uint<32> output_data_32_3 = 0;

    ap_uint<32> output_data_neg = 0;
    ap_uint<32> output_data_tmp = 0;

    // use to determine the corresponding value is negative or the output is valid
    ap_uint<8> output_data_record_0 = 0;
    ap_uint<8> output_data_record_1 = 0;
    ap_uint<8> output_data_record_2 = 0;
    ap_uint<8> output_data_record_3 = 0;

    ap_uint<128> output_data_128 = 0;
    ap_uint<128> end_of_text_data = 0xffffffffffffffff;

    ap_uint<4> input_tab = 0;

    DecodeUTF8:
    do {
        #pragma HLS pipeline II=1

        if (!stream_in.empty()) {
            ap_uint<32> input_data = stream_in.read();

            ap_uint<32> input_data_0 = input_data(7, 0);
            ap_uint<32> input_data_1 = input_data(15, 8);
            ap_uint<32> input_data_2 = input_data(23, 16);
            ap_uint<32> input_data_3 = input_data(31, 24);

            // std::cout << std::dec << feature_index << std::hex << ": " << input_data_0 << ", " << input_data_1 << " -> ";
            // std::cout << std::hex << input_data_0 << ", " << input_data_1 << ", " << input_data_2 << ", " << input_data_3 << std::endl;

            if (input_data == end_of_text) {
                stream_out.write(end_of_text_data);
                // categorical_stream_0.write(end_of_text_data);
                read_loop++;
                feature_index = 0;
            }
            else {
                input_tab(0, 0) = (input_data_3 == tab_code);
                input_tab(1, 1) = (input_data_2 == tab_code);
                input_tab(2, 2) = (input_data_1 == tab_code);
                input_tab(3, 3) = (input_data_0 == tab_code);

                // std::cout << "  " << std::hex << input_tab << std::endl;

                switch(input_tab) {
                    case 0b0000:
                        
                        output_data_32_0 = (input_data_0==minus_sign) ? ap_uint<32>((input_data_1 << 8) + (input_data_2 << 4) + input_data_3) : ap_uint<32>((output_data_32_0 << 16) + (input_data_0 << 12) + (input_data_1 << 8) + (input_data_2 << 4) + input_data_3);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = (input_data_0==minus_sign) ? 1: output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_flag = false;
                        break;

                    case 0b0001:

                        output_data_32_0 = (input_data_0==minus_sign) ? ap_uint<32>((input_data_1 << 4) + input_data_2) : ap_uint<32>((output_data_32_0 << 12) + (input_data_0 << 8) + (input_data_1 << 4) + input_data_2);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0001;
                        output_data_32_3(4, 4) = (input_data_0==minus_sign) ? 1: output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        // std::cout << "              " << std::hex << output_data_32_0 << ", " << (input_data_0==minus_sign) << std::endl;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = 0;
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        output_data_32_3 = 0;

                        output_flag = true;
                        break;
                    case 0b0010:
                        
                        output_data_32_0 = (input_data_0==minus_sign) ? ap_uint<32>(input_data_1) : ap_uint<32>((output_data_32_0 << 8) + (input_data_0 << 4) + input_data_1);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0001;
                        output_data_32_3(4, 4) = (input_data_0==minus_sign) ? 1: output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = (input_data_3==minus_sign) ? ap_uint<32>(0) : ap_uint<32>(input_data_3);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = (input_data_3==minus_sign) ? 1: 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;
                    case 0b0100:
                        output_data_32_0 = ap_uint<32>((output_data_32_0 << 4) + input_data_0);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0001;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = (input_data_2==minus_sign) ? ap_uint<32>(input_data_3) : ap_uint<32>((input_data_2 << 4) + input_data_3);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = (input_data_2==minus_sign) ? 1: 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;
                    case 0b1000:
                        output_data_32_0 = ap_uint<32>(output_data_32_0);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0001;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = (input_data_1==minus_sign) ? ap_uint<32>((input_data_2 << 4) + input_data_3) : ap_uint<32>((input_data_1 << 8) + (input_data_2 << 4) + input_data_3);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = (input_data_1==minus_sign) ? 1: 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;

                    case 0b0011:
                        output_data_32_0 = (input_data_0==minus_sign) ? ap_uint<32>(input_data_1) : ap_uint<32>((output_data_32_0 << 8) + (input_data_0 << 4) + input_data_1);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0010;
                        output_data_32_3(4, 4) = (input_data_0==minus_sign) ? 1: output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = 0;
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;
                    case 0b0101:
                        output_data_32_0 = ap_uint<32>((output_data_32_0 << 4) + input_data_0);
                        output_data_32_1 = input_data_2;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0010;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = 0;
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;
                    case 0b1001:
                        output_data_32_0 = ap_uint<32>(output_data_32_0);
                        output_data_32_1 = (input_data_1==minus_sign) ? ap_uint<32>(input_data_2) : ap_uint<32>((input_data_1 << 4) + input_data_2);
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0010;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(5, 5) = (input_data_1==minus_sign);
                        output_data_32_3(7, 6) = 0;


                        // std::cout << "              " << std::hex << input_data_1 << ", " << input_data_2 << std::endl;
                        // std::cout << "              " << std::hex << output_data_32_0 << ", " << output_data_32_1 << std::endl;
                        

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = 0;
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;
                    case 0b0110:
                        output_data_32_0 = ap_uint<32>((output_data_32_0 << 4) + input_data_0);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0010;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = (input_data_3==minus_sign) ? ap_uint<32>(0) : ap_uint<32>(input_data_3);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = (input_data_3==minus_sign) ? 1: 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;
                    case 0b1010:
                        output_data_32_0 = ap_uint<32>(output_data_32_0);
                        output_data_32_1 = input_data_1;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0010;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = (input_data_3==minus_sign) ? ap_uint<32>(0) : ap_uint<32>(input_data_3);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = (input_data_3==minus_sign) ? 1: 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;
                    case 0b1100:
                        output_data_32_0 = ap_uint<32>(output_data_32_0);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0010;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = (input_data_2==minus_sign) ? ap_uint<32>(input_data_3) : ap_uint<32>((input_data_2 << 4) + input_data_3);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = (input_data_2==minus_sign) ? 1: 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;

                    case 0b0111:
                        output_data_32_0 = ap_uint<32>((output_data_32_0 << 4) + input_data_0);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0011;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = 0;
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;
                    case 0b1011:
                        output_data_32_0 = output_data_32_0;
                        output_data_32_1 = input_data_1;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0011;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = 0;
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;
                    case 0b1101:
                        output_data_32_0 = output_data_32_0;
                        output_data_32_1 = 0;
                        output_data_32_2 = input_data_2;
                        output_data_32_3(3, 0) = 0b0011;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = 0;
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;
                    case 0b1110:
                        output_data_32_0 = output_data_32_0;
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0011;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = (input_data_3==minus_sign) ? ap_uint<32>(0) : ap_uint<32>(input_data_3);
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = (input_data_3==minus_sign) ? 1 : 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;

                    default:
                        // 0b1111, all are tabs, 4 outputs 
                        output_data_32_0 = output_data_32_0;
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        output_data_32_3(3, 0) = 0b0100;
                        output_data_32_3(4, 4) = output_data_32_3(4, 4);
                        output_data_32_3(7, 5) = 0;

                        output_data_128(31, 0)   = output_data_32_0;
                        output_data_128(63, 32)  = output_data_32_1;
                        output_data_128(95, 64)  = output_data_32_2;
                        output_data_128(127, 96) = output_data_32_3;

                        output_data_32_0 = 0;
                        output_data_32_1 = 0;
                        output_data_32_2 = 0;
                        // (3, 0) denote valid output
                        output_data_32_3(3, 0) = 0;
                        // (7, 4) denote minus sign
                        output_data_32_3(4, 4) = 0;
                        output_data_32_3(7, 5) = 0;

                        output_flag = true;
                        break;
                }

                // if (feature_index >= 40) {
                //     feature_index = feature_index - 40;
                // }

                if (output_flag) {
                    output_flag = false;
                    stream_out.write(output_data_128);
                    // std::cout << "      " << std::hex << input_tab << ": " << input_data_0 << ", " << input_data_1 << ", " << input_data_2 << ", " << input_data_3 << std::endl;
                    // std::cout << "          " << std::hex << output_data_128(31, 0) << ", " << output_data_128(63, 32) << ", " << output_data_128(95, 64) << ", " << output_data_128(99, 96) << ", " << output_data_128(101, 100) << std::endl;
                }
            }
            
        }        
    }while(read_loop < 2);

}

void DecodeDistribute(
    hls::stream<ap_uint<128> >& stream_in,
    hls::stream<ap_uint<64> >& stream_out_0,
    hls::stream<ap_uint<64> >& stream_out_1,
    hls::stream<ap_uint<64> >& stream_out_2,
    hls::stream<ap_uint<64> >& stream_out_3,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "DecodeDistribute: " << std::endl;

    int read_loop = 0;
    int input_counter = 0;
    int output_counter = 0;
    
    ap_uint<32> input_data_0 = 0;
    ap_uint<32> input_data_1 = 0;
    ap_uint<32> input_data_2 = 0;
    ap_uint<32> input_data_3 = 0;
    ap_uint<128> input_data = 0;

    ap_uint<4> output_meta = 0;
    ap_uint<1> neg_meta_0 = 0;  
    ap_uint<1> neg_meta_1 = 0;  

    ap_uint<32> output_data_32_0 = 0;
    ap_uint<32> output_data_32_1 = 0;
    ap_uint<32> output_data_neg_0 = 0;
    ap_uint<32> output_data_neg_1 = 0;
    ap_uint<32> output_data_tmp = 0;
    ap_uint<64> output_data_64_0 = 0;

    ap_uint<64> end_of_text_data = 0xefffffffffffffff;
    ap_uint<128> end_of_text_input = 0xffffffffffffffff;

    int binary_counter = 0;

    int output_0_counter = 0;
    int output_1_counter = 0;
    int output_2_counter = 0;
    int output_3_counter = 0;

    DecodeDistribute:
    do {
        #pragma HLS pipeline II=1
        if (!stream_in.empty()) {
            input_data = stream_in.read();

            input_counter++;

            input_data_0 = input_data(31, 0);
            input_data_1 = input_data(63, 32);
            input_data_2 = input_data(95, 64);
            input_data_3 = input_data(127, 96);

            output_meta = input_data_3(3, 0);
            neg_meta_0 = input_data_3(4, 4);
            neg_meta_1 = input_data_3(5, 5);

            output_data_neg_0 = ~input_data_0 + 1;
            output_data_neg_1 = ~input_data_1 + 1;

            if (input_data == end_of_text_input) {
                stream_out_0.write(end_of_text_data);
                // stream_out_1.write(end_of_text_data);
                // stream_out_2.write(end_of_text_data);
                // stream_out_3.write(end_of_text_data);
                read_loop++;
                std::cout << "input_counter: " << input_counter << std::endl;

                std::cout << "output_0_counter: " << output_0_counter << std::endl;
                std::cout << "output_1_counter: " << output_1_counter << std::endl;
                std::cout << "output_2_counter: " << output_2_counter << std::endl;
                std::cout << "output_3_counter: " << output_3_counter << std::endl;
            }
            else {
                output_data_32_0 = (neg_meta_0==1) ? output_data_neg_0 : input_data_0;
                output_data_32_1 = (neg_meta_1==1) ? output_data_neg_1 : input_data_1;

                // std::cout << std::hex << output_meta << std::endl;

                output_data_64_0(31, 0) = output_data_32_0;
                output_data_64_0(35, 32) = output_meta;

                if (output_meta == 0b0100) {
                    stream_out_0.write(output_data_64_0);
                    stream_out_1.write(output_data_32_1);
                    stream_out_2.write(0);
                    stream_out_3.write(0);

                    output_0_counter++;
                    output_1_counter++;
                    output_2_counter++;
                    output_3_counter++;
                }
                else if (output_meta == 0b0011) {
                    stream_out_0.write(output_data_64_0);
                    stream_out_1.write(output_data_32_1);
                    stream_out_2.write(input_data_2);

                    output_0_counter++;
                    output_1_counter++;
                    output_2_counter++;
                }
                else if (output_meta == 0b0010) {
                    stream_out_0.write(output_data_64_0);
                    stream_out_1.write(output_data_32_1);

                    output_0_counter++;
                    output_1_counter++;
                }
                else {
                    stream_out_0.write(output_data_64_0);
                    output_0_counter++;
                }
                
                
                // output_counter++;
                // std::cout << std::hex << input_data_0 << std::endl;

                // binary_counter++;
            }
            
        }        
    }while(read_loop < 2);

}

void DecodeFilter(
    hls::stream<ap_uint<64> >& stream_in_0,
    hls::stream<ap_uint<64> >& stream_in_1,
    hls::stream<ap_uint<64> >& stream_in_2,
    hls::stream<ap_uint<64> >& stream_in_3,
    hls::stream<ap_uint<64> >& stream_out,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "DecodeFilter: " << std::endl;

    int read_loop = 0;
    int input_counter = 0;
    int output_counter = 0;
    
    bool first_output_flag = false;
    bool second_output_flag = false;
    bool third_output_flag = false;
    bool forth_output_flag = false;
    
    ap_uint<64> input_data_0 = 0;
    ap_uint<64> input_data_1 = 0;
    ap_uint<64> input_data_2 = 0;
    ap_uint<64> input_data_3 = 0;

    ap_uint<64> end_of_text_data = 0xefffffffffffffff;

    ap_uint<4> binary_counter = 0;
    int state = 0;

    int output_0_counter = 0;
    int output_1_counter = 0;
    int output_2_counter = 0;
    int output_3_counter = 0;

    DecodeFilter:
    do {
        #pragma HLS pipeline II=1
        switch(state) {
            case 0:
                if (!stream_in_0.empty()) {
                    input_data_0 = stream_in_0.read();
                    

                    if (input_data_0 == end_of_text_data) {
                        stream_out.write(end_of_text_data);
                        read_loop++;
                        std::cout << "Stop" << std::endl;

                        binary_counter = 0;
                        state = 0;
                    }
                    else {
                        stream_out.write(input_data_0);

                        output_0_counter++;
                        std::cout << "0 -> " << std::hex << input_data_0(31, 0) << std::endl;

                        binary_counter = input_data_0(35, 32);
                        state = (binary_counter > 1) ? 1 : 0;
                    }
                    
                }
                else {
                    state = 0;
                }
                break;
            case 1:
                // if (!stream_in_1.empty()) {
                input_data_1 = stream_in_1.read();
                stream_out.write(input_data_1);

                std::cout << "1 -> " << std::hex << input_data_1 << std::endl;
                output_1_counter++;
                    // std::cout << "output_1_counter: " << output_1_counter << std::endl;
                // } 
                state = (binary_counter > 2) ? 2 : 0;
                break;
            case 2:
                // if (!stream_in_2.empty()) {
                input_data_2 = stream_in_2.read();
                stream_out.write(input_data_2);

                std::cout << "2 -> " << std::hex << input_data_2 << std::endl;
                output_2_counter++;
                    // std::cout << "output_2_counter: " << output_2_counter << std::endl;
                // } 
                state = (binary_counter > 3) ? 3 : 0;
                break;
            case 3:
                // if (!stream_in_3.empty()) {
                input_data_3 = stream_in_3.read();
                stream_out.write(input_data_3);

                std::cout << "3 -> " << std::hex << input_data_3 << std::endl;
                output_3_counter++;
                    // std::cout << "output_3_counter: " << output_3_counter << std::endl;
                // }
                state = 0;
                break;
        }
    } while(read_loop < 2);

}



void ParseHex(
    hls::stream<ap_uint<64> >& stream_in,
    hls::stream<ap_uint<64> >& stream_out,
    ull input_bytes
) {
    #pragma HLS INLINE off

    std::cout << "ParseHex: " << std::endl;

    // Use Count_wait to determine whether the upstream data has been fully consumed
    int read_loop = 0;
    int feature_index = 0;
    // int output_data = 0;
    // bool input_flag = false;

    ap_uint<64> input_data = 0;
    ap_uint<64> output_data = 0;
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;

    ap_uint<4> hex_value_array[8];

    ap_uint<32> input_data_neg = 0;
    ap_uint<32> input_data_32 = 0;
    ap_uint<32> output_data_tmp = 0;

    ParseHex:
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
                if (feature_index == 0) {
                    std::cout << std::endl;
                }

                if (feature_index < 14) {
                    input_data_32 = input_data(31, 0);
                    input_data_neg = ~input_data_32 + 1;

                    if (input_data_32(31, 31) == 1) {
                        for (int k = 0; k < 8; k++) {
                            #pragma HLS unroll
                            hex_value_array[k] = input_data_neg(4*k+3, 4*k);
                        }
                    }
                    else {
                        for (int k = 0; k < 8; k++) {
                            #pragma HLS unroll
                            hex_value_array[k] = input_data_32(4*k+3, 4*k);
                        }
                    }

                    output_data_tmp = 0;
                    for (int k = 7; k >= 0; k--) {
                        #pragma HLS unroll
                        output_data_tmp = output_data_tmp * 10 + hex_value_array[k];
                    }
                    

                    output_data(31, 0) = (input_data_32(31, 31) == 1) ? ap_uint<32>(~output_data_tmp+1) : output_data_tmp;
                    output_data(63, 32) = 0;

                    int tmp_value = output_data(31, 0);
                    std::cout << std::dec << tmp_value << " ";
                }
                else {
                    output_data = input_data;

                    std::cout << std::hex << input_data(31, 0) << " ";
                }

                
                feature_index = (feature_index < 39) ? (feature_index + 1) : 0;
            }
    
            stream_out.write(output_data);  
            
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
                if (feature_index==0) {
                    std::cout << std::endl;
                }

                if (feature_index == 0) {
                    int tmp_value = input_data(31, 0);

                    output_data(31, 0) = tmp_value;
                    output_data(63, 32) = 0;
                }
                else if (feature_index < 14) {
                    int tmp_value = input_data(31, 0);
                    
                    tmp_value = (tmp_value < 0) ? 0 : tmp_value;
                    conv conv_value;
                    conv_value.float32 = hls::logf(tmp_value+1);
                    output_data(31, 0) = conv_value.uint32;//tmp_value;
                    output_data(63, 32) = 0;
                    // output_data(63, 32) = conv_value.uint32;

                    std::cout << std::dec << tmp_value << "->" << conv_value.float32 << ", ";
                }
                else {
                    output_data = input_data;
                    // std::cout << std::hex << input_data(31, 0) << " ";
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
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;
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
    ap_uint<64> end_of_text_data = 0xefffffffffffffff;
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
    int feature_id = 0;
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
            else {
                if (feature_id == 0) {
                    std::cout << std::dec << input_data(31, 0) << " ";
                }
                else if (feature_id < 14) {
                    conv conv_value;
                    conv_value.uint32 = input_data(31, 0);
                    std::cout << std::dec << conv_value.float32 << " ";
                }
                else {
                    std::cout << std::dec << input_data(31, 0) << " ";
                }

                if (feature_id < 39) {
                    feature_id++;
                }
                else {
                    feature_id = 0;
                    std::cout << std::endl;
                }

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
