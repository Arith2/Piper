#include "vadd.hpp"

void vadd(  
    ap_uint<512>* table_DDR0,  
    ap_uint<512>* table_DDR1,
    ap_uint<512>* table_DDR2,
    ap_uint<512>* table_DDR3,
    int row_num_0,
    int row_num_1,
    int row_num_2,
    int row_num_3,
    int row_num_total,
    int max_vocab_size
    )
{
    
#pragma HLS INTERFACE m_axi port=table_DDR0  offset=slave bundle=gmem0 num_read_outstanding=128 max_read_burst_length=256
#pragma HLS INTERFACE m_axi port=table_DDR1  offset=slave bundle=gmem1 num_read_outstanding=128 max_read_burst_length=256
#pragma HLS INTERFACE m_axi port=table_DDR2  offset=slave bundle=gmem2 num_read_outstanding=128 max_read_burst_length=256
#pragma HLS INTERFACE m_axi port=table_DDR3  offset=slave bundle=gmem2 num_read_outstanding=128 max_read_burst_length=256

#pragma HLS INTERFACE s_axilite port=table_DDR0  bundle=control
#pragma HLS INTERFACE s_axilite port=table_DDR1  bundle=control
#pragma HLS INTERFACE s_axilite port=table_DDR2  bundle=control
#pragma HLS INTERFACE s_axilite port=table_DDR3  bundle=control

#pragma HLS INTERFACE s_axilite port=row_num_0 bundle = control
#pragma HLS INTERFACE s_axilite port=row_num_1 bundle = control
#pragma HLS INTERFACE s_axilite port=row_num_2 bundle = control
#pragma HLS INTERFACE s_axilite port=row_num_3 bundle = control
#pragma HLS INTERFACE s_axilite port=row_num_total bundle = control
#pragma HLS INTERFACE s_axilite port=max_vocab_size bundle = control

#pragma HLS INTERFACE s_axilite port=return bundle=control
    
#pragma HLS dataflow

    hls::stream<ap_uint<512> > input_stream_0;
#pragma HLS stream variable=input_stream_0 depth=64
    hls::stream<ap_uint<512> > input_stream_1;
#pragma HLS stream variable=input_stream_1 depth=64
    hls::stream<ap_uint<512> > input_stream_2;
#pragma HLS stream variable=input_stream_2 depth=64

    hls::stream<ap_uint<512> > numerical_stream_0;
#pragma HLS stream variable=numerical_stream_0 depth=64

    hls::stream<ap_uint<512> > categorical_stream_0;
#pragma HLS stream variable=categorical_stream_0 depth=64
    hls::stream<ap_uint<512> > categorical_stream_1;
#pragma HLS stream variable=categorical_stream_1 depth=64

    hls::stream<ap_uint<32> > categorical_stream_split_16_0_0;
#pragma HLS stream variable=categorical_stream_split_16_0_0 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_1;
#pragma HLS stream variable=categorical_stream_split_16_0_1 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_2;
#pragma HLS stream variable=categorical_stream_split_16_0_2 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_3;
#pragma HLS stream variable=categorical_stream_split_16_0_3 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_4;
#pragma HLS stream variable=categorical_stream_split_16_0_4 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_5;
#pragma HLS stream variable=categorical_stream_split_16_0_5 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_6;
#pragma HLS stream variable=categorical_stream_split_16_0_6 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_7;
#pragma HLS stream variable=categorical_stream_split_16_0_7 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_8;
#pragma HLS stream variable=categorical_stream_split_16_0_8 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_9;
#pragma HLS stream variable=categorical_stream_split_16_0_9 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_10;
#pragma HLS stream variable=categorical_stream_split_16_0_10 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_11;
#pragma HLS stream variable=categorical_stream_split_16_0_11 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_12;
#pragma HLS stream variable=categorical_stream_split_16_0_12 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_13;
#pragma HLS stream variable=categorical_stream_split_16_0_13 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_14;
#pragma HLS stream variable=categorical_stream_split_16_0_14 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_0_15;
#pragma HLS stream variable=categorical_stream_split_16_0_15 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_0;
#pragma HLS stream variable=categorical_stream_split_16_1_0 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_1;
#pragma HLS stream variable=categorical_stream_split_16_1_1 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_2;
#pragma HLS stream variable=categorical_stream_split_16_1_2 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_3;
#pragma HLS stream variable=categorical_stream_split_16_1_3 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_4;
#pragma HLS stream variable=categorical_stream_split_16_1_4 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_5;
#pragma HLS stream variable=categorical_stream_split_16_1_5 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_6;
#pragma HLS stream variable=categorical_stream_split_16_1_6 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_7;
#pragma HLS stream variable=categorical_stream_split_16_1_7 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_8;
#pragma HLS stream variable=categorical_stream_split_16_1_8 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_9;
#pragma HLS stream variable=categorical_stream_split_16_1_9 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_10;
#pragma HLS stream variable=categorical_stream_split_16_1_10 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_11;
#pragma HLS stream variable=categorical_stream_split_16_1_11 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_12;
#pragma HLS stream variable=categorical_stream_split_16_1_12 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_13;
#pragma HLS stream variable=categorical_stream_split_16_1_13 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_14;
#pragma HLS stream variable=categorical_stream_split_16_1_14 depth=64
    hls::stream<ap_uint<32> > categorical_stream_split_16_1_15;
#pragma HLS stream variable=categorical_stream_split_16_1_15 depth=64

    hls::stream<ap_uint<32> > categorical_dict_value_16_0_0;
#pragma HLS stream variable=categorical_dict_value_16_0_0 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_1;
#pragma HLS stream variable=categorical_dict_value_16_0_1 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_2;
#pragma HLS stream variable=categorical_dict_value_16_0_2 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_3;
#pragma HLS stream variable=categorical_dict_value_16_0_3 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_4;
#pragma HLS stream variable=categorical_dict_value_16_0_4 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_5;
#pragma HLS stream variable=categorical_dict_value_16_0_5 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_6;
#pragma HLS stream variable=categorical_dict_value_16_0_6 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_7;
#pragma HLS stream variable=categorical_dict_value_16_0_7 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_8;
#pragma HLS stream variable=categorical_dict_value_16_0_8 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_9;
#pragma HLS stream variable=categorical_dict_value_16_0_9 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_10;
#pragma HLS stream variable=categorical_dict_value_16_0_10 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_11;
#pragma HLS stream variable=categorical_dict_value_16_0_11 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_12;
#pragma HLS stream variable=categorical_dict_value_16_0_12 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_13;
#pragma HLS stream variable=categorical_dict_value_16_0_13 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_14;
#pragma HLS stream variable=categorical_dict_value_16_0_14 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_0_15;
#pragma HLS stream variable=categorical_dict_value_16_0_15 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_0;
#pragma HLS stream variable=categorical_dict_value_16_1_0 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_1;
#pragma HLS stream variable=categorical_dict_value_16_1_1 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_2;
#pragma HLS stream variable=categorical_dict_value_16_1_2 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_3;
#pragma HLS stream variable=categorical_dict_value_16_1_3 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_4;
#pragma HLS stream variable=categorical_dict_value_16_1_4 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_5;
#pragma HLS stream variable=categorical_dict_value_16_1_5 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_6;
#pragma HLS stream variable=categorical_dict_value_16_1_6 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_7;
#pragma HLS stream variable=categorical_dict_value_16_1_7 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_8;
#pragma HLS stream variable=categorical_dict_value_16_1_8 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_9;
#pragma HLS stream variable=categorical_dict_value_16_1_9 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_10;
#pragma HLS stream variable=categorical_dict_value_16_1_10 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_11;
#pragma HLS stream variable=categorical_dict_value_16_1_11 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_12;
#pragma HLS stream variable=categorical_dict_value_16_1_12 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_13;
#pragma HLS stream variable=categorical_dict_value_16_1_13 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_14;
#pragma HLS stream variable=categorical_dict_value_16_1_14 depth=64
    hls::stream<ap_uint<32> > categorical_dict_value_16_1_15;
#pragma HLS stream variable=categorical_dict_value_16_1_15 depth=64

    hls::stream<ap_uint<32> > categorical_dict_16_0_0;
#pragma HLS stream variable=categorical_dict_16_0_0 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_1;
#pragma HLS stream variable=categorical_dict_16_0_1 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_2;
#pragma HLS stream variable=categorical_dict_16_0_2 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_3;
#pragma HLS stream variable=categorical_dict_16_0_3 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_4;
#pragma HLS stream variable=categorical_dict_16_0_4 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_5;
#pragma HLS stream variable=categorical_dict_16_0_5 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_6;
#pragma HLS stream variable=categorical_dict_16_0_6 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_7;
#pragma HLS stream variable=categorical_dict_16_0_7 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_8;
#pragma HLS stream variable=categorical_dict_16_0_8 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_9;
#pragma HLS stream variable=categorical_dict_16_0_9 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_10;
#pragma HLS stream variable=categorical_dict_16_0_10 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_11;
#pragma HLS stream variable=categorical_dict_16_0_11 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_12;
#pragma HLS stream variable=categorical_dict_16_0_12 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_13;
#pragma HLS stream variable=categorical_dict_16_0_13 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_14;
#pragma HLS stream variable=categorical_dict_16_0_14 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_0_15;
#pragma HLS stream variable=categorical_dict_16_0_15 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_0;
#pragma HLS stream variable=categorical_dict_16_1_0 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_1;
#pragma HLS stream variable=categorical_dict_16_1_1 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_2;
#pragma HLS stream variable=categorical_dict_16_1_2 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_3;
#pragma HLS stream variable=categorical_dict_16_1_3 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_4;
#pragma HLS stream variable=categorical_dict_16_1_4 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_5;
#pragma HLS stream variable=categorical_dict_16_1_5 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_6;
#pragma HLS stream variable=categorical_dict_16_1_6 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_7;
#pragma HLS stream variable=categorical_dict_16_1_7 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_8;
#pragma HLS stream variable=categorical_dict_16_1_8 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_9;
#pragma HLS stream variable=categorical_dict_16_1_9 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_10;
#pragma HLS stream variable=categorical_dict_16_1_10 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_11;
#pragma HLS stream variable=categorical_dict_16_1_11 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_12;
#pragma HLS stream variable=categorical_dict_16_1_12 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_13;
#pragma HLS stream variable=categorical_dict_16_1_13 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_14;
#pragma HLS stream variable=categorical_dict_16_1_14 depth=64
    hls::stream<ap_uint<32> > categorical_dict_16_1_15;
#pragma HLS stream variable=categorical_dict_16_1_15 depth=64

    hls::stream<ap_uint<512> > categorical_stream_combine_0;
#pragma HLS stream variable=categorical_stream_combine_0 depth=64
    hls::stream<ap_uint<512> > categorical_stream_combine_1;
#pragma HLS stream variable=categorical_stream_combine_1 depth=64

    LoadDDR(
        table_DDR0, 
        table_DDR1, 
        table_DDR2, 
        // table_DDR3, 
        input_stream_0, 
        input_stream_1,
        input_stream_2,
        row_num_0,
        row_num_1,
        row_num_2,
        // row_num_3,
        row_num_total);

// // Numerical features
    NegsToZeroLog(
        input_stream_0,
        numerical_stream_0,
        row_num_total
    );
// Categorical features
    HexToIntModRange(
        input_stream_1,
        categorical_stream_0,
        row_num_total,
        max_vocab_size
    );
    HexToIntModRange(
        input_stream_2,
        categorical_stream_1,
        row_num_total,
        max_vocab_size
    );

    StreamSplit(
        categorical_stream_0,
        categorical_stream_split_16_0_0,
        categorical_stream_split_16_0_1,
        categorical_stream_split_16_0_2,
        categorical_stream_split_16_0_3,
        categorical_stream_split_16_0_4,
        categorical_stream_split_16_0_5,
        categorical_stream_split_16_0_6,
        categorical_stream_split_16_0_7,
        categorical_stream_split_16_0_8,
        categorical_stream_split_16_0_9,
        categorical_stream_split_16_0_10,
        categorical_stream_split_16_0_11,
        categorical_stream_split_16_0_12,
        categorical_stream_split_16_0_13,
        categorical_stream_split_16_0_14,
        categorical_stream_split_16_0_15,
        row_num_total
    );
    StreamSplit(
        categorical_stream_1,
        categorical_stream_split_16_1_0,
        categorical_stream_split_16_1_1,
        categorical_stream_split_16_1_2,
        categorical_stream_split_16_1_3,
        categorical_stream_split_16_1_4,
        categorical_stream_split_16_1_5,
        categorical_stream_split_16_1_6,
        categorical_stream_split_16_1_7,
        categorical_stream_split_16_1_8,
        categorical_stream_split_16_1_9,
        categorical_stream_split_16_1_10,
        categorical_stream_split_16_1_11,
        categorical_stream_split_16_1_12,
        categorical_stream_split_16_1_13,
        categorical_stream_split_16_1_14,
        categorical_stream_split_16_1_15,
            row_num_total
    );

    HashDict(
        categorical_stream_split_16_0_0,
        categorical_dict_value_16_0_0,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_1,
        categorical_dict_value_16_0_1,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_2,
        categorical_dict_value_16_0_2,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_3,
        categorical_dict_value_16_0_3,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_4,
        categorical_dict_value_16_0_4,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_5,
        categorical_dict_value_16_0_5,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_6,
        categorical_dict_value_16_0_6,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_7,
        categorical_dict_value_16_0_7,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_8,
        categorical_dict_value_16_0_8,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_9,
        categorical_dict_value_16_0_9,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_10,
        categorical_dict_value_16_0_10,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_11,
        categorical_dict_value_16_0_11,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_12,
        categorical_dict_value_16_0_12,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_13,
        categorical_dict_value_16_0_13,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_14,
        categorical_dict_value_16_0_14,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_0_15,
        categorical_dict_value_16_0_15,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_0,
        categorical_dict_value_16_1_0,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_1,
        categorical_dict_value_16_1_1,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_2,
        categorical_dict_value_16_1_2,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_3,
        categorical_dict_value_16_1_3,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_4,
        categorical_dict_value_16_1_4,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_5,
        categorical_dict_value_16_1_5,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_6,
        categorical_dict_value_16_1_6,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_7,
        categorical_dict_value_16_1_7,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_8,
        categorical_dict_value_16_1_8,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_9,
        categorical_dict_value_16_1_9,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_10,
        categorical_dict_value_16_1_10,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_11,
        categorical_dict_value_16_1_11,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_12,
        categorical_dict_value_16_1_12,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_13,
        categorical_dict_value_16_1_13,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_14,
        categorical_dict_value_16_1_14,
        row_num_total
    );
    HashDict(
        categorical_stream_split_16_1_15,
        categorical_dict_value_16_1_15,
        row_num_total
    );

    CreateDict(
        categorical_dict_value_16_0_0,
        categorical_dict_16_0_0,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_1,
        categorical_dict_16_0_1,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_2,
        categorical_dict_16_0_2,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_3,
        categorical_dict_16_0_3,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_4,
        categorical_dict_16_0_4,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_5,
        categorical_dict_16_0_5,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_6,
        categorical_dict_16_0_6,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_7,
        categorical_dict_16_0_7,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_8,
        categorical_dict_16_0_8,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_9,
        categorical_dict_16_0_9,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_10,
        categorical_dict_16_0_10,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_11,
        categorical_dict_16_0_11,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_12,
        categorical_dict_16_0_12,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_13,
        categorical_dict_16_0_13,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_14,
        categorical_dict_16_0_14,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_0_15,
        categorical_dict_16_0_15,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_0,
        categorical_dict_16_1_0,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_1,
        categorical_dict_16_1_1,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_2,
        categorical_dict_16_1_2,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_3,
        categorical_dict_16_1_3,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_4,
        categorical_dict_16_1_4,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_5,
        categorical_dict_16_1_5,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_6,
        categorical_dict_16_1_6,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_7,
        categorical_dict_16_1_7,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_8,
        categorical_dict_16_1_8,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_9,
        categorical_dict_16_1_9,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_10,
        categorical_dict_16_1_10,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_11,
        categorical_dict_16_1_11,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_12,
        categorical_dict_16_1_12,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_13,
        categorical_dict_16_1_13,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_14,
        categorical_dict_16_1_14,
        row_num_total
    );
    CreateDict(
        categorical_dict_value_16_1_15,
        categorical_dict_16_1_15,
        row_num_total
    );

    StreamCombine(
        categorical_dict_16_0_0,
        categorical_dict_16_0_1,
        categorical_dict_16_0_2,
        categorical_dict_16_0_3,
        categorical_dict_16_0_4,
        categorical_dict_16_0_5,
        categorical_dict_16_0_6,
        categorical_dict_16_0_7,
        categorical_dict_16_0_8,
        categorical_dict_16_0_9,
        categorical_dict_16_0_10,
        categorical_dict_16_0_11,
        categorical_dict_16_0_12,
        categorical_dict_16_0_13,
        categorical_dict_16_0_14,
        categorical_dict_16_0_15,
        categorical_stream_combine_0,
        row_num_total
    );
    StreamCombine(
        categorical_dict_16_1_0,
        categorical_dict_16_1_1,
        categorical_dict_16_1_2,
        categorical_dict_16_1_3,
        categorical_dict_16_1_4,
        categorical_dict_16_1_5,
        categorical_dict_16_1_6,
        categorical_dict_16_1_7,
        categorical_dict_16_1_8,
        categorical_dict_16_1_9,
        categorical_dict_16_1_10,
        categorical_dict_16_1_11,
        categorical_dict_16_1_12,
        categorical_dict_16_1_13,
        categorical_dict_16_1_14,
        categorical_dict_16_1_15,
        categorical_stream_combine_1,
        row_num_total
    );

// ////////////////////////////
    // ConsumeStream_Num(
    //     numerical_stream_0,
    //     row_num_total
    // );

    // ConsumeStream_Cat(
    //     categorical_stream_combine_0,
    //     categorical_stream_combine_1,
    //     row_num_total
    // );

    StoreDDR(
        numerical_stream_0,
        categorical_stream_combine_0,
        categorical_stream_combine_1,
        table_DDR3,
        row_num_total
    );

    // StoreDDR(numerical_stream_0, table_DDR0, row_num_total);
    // StoreDDR(categorical_stream_combine_0, table_DDR1, row_num_total);
    // StoreDDR(categorical_stream_combine_1, table_DDR2, row_num_total);
    
}
