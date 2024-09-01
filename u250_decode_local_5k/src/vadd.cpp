#include "vadd.hpp"

void vadd(  
    ap_uint<32>* table_DDR0,  
    ap_uint<32>* table_DDR1,  
    ap_uint<32>* table_DDR2, 
    // ap_uint<32>* table_DDR3, 
    ull input_bytes,
    ull row_num,
    // ull DDR_table_bytes,
    int max_vocab_size
    )
{
    
#pragma HLS INTERFACE m_axi port=table_DDR0  offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=table_DDR1  offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port=table_DDR2  offset=slave bundle=gmem2
// #pragma HLS INTERFACE m_axi port=table_DDR3  offset=slave bundle=gmem3

#pragma HLS INTERFACE s_axilite port=table_DDR0  bundle=control
#pragma HLS INTERFACE s_axilite port=table_DDR1  bundle=control
#pragma HLS INTERFACE s_axilite port=table_DDR2  bundle=control
// #pragma HLS INTERFACE s_axilite port=table_DDR3  bundle=control

#pragma HLS INTERFACE s_axilite port=input_bytes bundle = control
#pragma HLS INTERFACE s_axilite port=row_num bundle = control
// #pragma HLS INTERFACE s_axilite port=DDR_table_bytes bundle = control
#pragma HLS INTERFACE s_axilite port=max_vocab_size bundle = control

#pragma HLS INTERFACE s_axilite port=return bundle=control
    
#pragma HLS dataflow

    hls::stream<ap_uint<32> > ddr_stream_32[1];
#pragma HLS stream variable=ddr_stream_32 depth=256
    hls::stream<ap_uint<16> > ddr_stream_16[1];
#pragma HLS stream variable=ddr_stream_16 depth=256
    hls::stream<ap_uint<16> > ddr_stream[1];
#pragma HLS stream variable=ddr_stream depth=256

    hls::stream<ap_uint<32> > ascii_stream[1];
#pragma HLS stream variable=ascii_stream depth=256

    hls::stream<ap_uint<128> > decode_stream[1];
#pragma HLS stream variable=decode_stream depth=256

    hls::stream<ap_uint<64> > decode_stream_dist[4];
#pragma HLS stream variable=decode_stream_dist depth=256

    hls::stream<ap_uint<64> > numerical_stream[1];
#pragma HLS stream variable=numerical_stream depth=256

    hls::stream<ap_uint<64> > parse_stream[1];
#pragma HLS stream variable=parse_stream depth=256
    hls::stream<ap_uint<64> > numerical_stream_log[1];
#pragma HLS stream variable=numerical_stream_log depth=256

    hls::stream<ap_uint<64> > categorical_stream[1];
#pragma HLS stream variable=categorical_stream depth=256

    hls::stream<ap_uint<64> > categorical_stream_int[1];
#pragma HLS stream variable=categorical_stream_int depth=256

    hls::stream<ap_uint<64> > categorical_dict_value[1];
#pragma HLS stream variable=categorical_dict_value depth=256

    hls::stream<ap_uint<64> > categorical_dict_output[1];
#pragma HLS stream variable=categorical_dict_output depth=256

    // Fill missing value
    // Map ASCII code into integer
    LoadDDR_UTF8(
        table_DDR0, 
        table_DDR1,
        table_DDR2,
        // table_DDR3,
        ddr_stream_32[0], 
        input_bytes
    );

    // Long2Byte(
    //     ddr_stream_32[0], 
    //     ddr_stream_16[0],
    //     input_bytes
    // );

    // ByteFilter(
    //     ddr_stream_16[0], 
    //     ddr_stream[0],
    //     input_bytes
    // );

    DecodeASCII(
        ddr_stream_32[0], 
        ascii_stream[0],
        input_bytes
    );

    DecodeUTF8(
        ascii_stream[0],
        decode_stream[0],
        // categorical_stream[0],
        input_bytes
    );

    DecodeDistribute(
        decode_stream[0],
        decode_stream_dist[0],
        decode_stream_dist[1],
        decode_stream_dist[2],
        decode_stream_dist[3],
        input_bytes
    );

    DecodeFilter(
        decode_stream_dist[0],
        decode_stream_dist[1],
        decode_stream_dist[2],
        decode_stream_dist[3],
        parse_stream[0],
        // categorical_stream[0],
        input_bytes
    );

    ParseHex(
        parse_stream[0],
        numerical_stream[0],
        input_bytes
    );

    // Numerical features
    // Log operation
    Neg2ZeroLog(
        numerical_stream[0],
        numerical_stream_log[0],
        input_bytes
    );

    // Categorical features
    // Hex to Integer Modulo
    Hex2IntMod(
        // categorical_stream[0],
        numerical_stream_log[0],
        categorical_stream_int[0],
        input_bytes,
        max_vocab_size
    );
    MapDict(
        categorical_stream_int[0],
        categorical_dict_value[0], 
        input_bytes
    );
    CreateDict(
        categorical_dict_value[0],
        categorical_dict_output[0], 
        input_bytes
    );

    // StoreDDR(
    //     categorical_dict_output[0], 
    //     table_DDR0, 
    //     table_DDR1, 
    //     table_DDR2, 
    //     input_bytes,
    //     row_num
    //     // DDR_table_bytes
    // );

    ConsumeStream(
        categorical_dict_output[0], 
        input_bytes
    );
    
}
