/*
 * Copyright (c) 2020, Systems Group, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "ap_axi_sdata.h"
#include <ap_fixed.h>
#include "ap_int.h" 
#include "../../../../common/include/communication.hpp"
#include "../../../../common/include/vadd.hpp"
#include "hls_stream.h"


void traffic_gen(ap_uint<64> expectedByteCnt, hls::stream<ap_uint<512> >& s_data_in)
{
// #pragma HLS dataflow
#pragma HLS INLINE off

    ap_uint<64> expectedWordCnt = expectedByteCnt / 64;
    ap_uint<512> s_data = 0;

    traffic_gen:
    for (ap_uint<64> j = 0; j < expectedWordCnt; ++j)
    {
        #pragma HLS pipeline II=1
        s_data(63, 0) = j;
        s_data_in.write(s_data);
    }
}

extern "C" {
void hls_bil_krnl(
               // Internal Stream
               hls::stream<pkt512>& s_axis_udp_rx, 
               hls::stream<pkt512>& m_axis_udp_tx, 
               hls::stream<pkt256>& s_axis_udp_rx_meta, 
               hls::stream<pkt256>& m_axis_udp_tx_meta, 
               
               hls::stream<pkt16>& m_axis_tcp_listen_port, 
               hls::stream<pkt8>& s_axis_tcp_port_status, 
               hls::stream<pkt64>& m_axis_tcp_open_connection, 
               hls::stream<pkt128>& s_axis_tcp_open_status, 
               hls::stream<pkt16>& m_axis_tcp_close_connection, 
               hls::stream<pkt128>& s_axis_tcp_notification, 
               hls::stream<pkt32>& m_axis_tcp_read_pkg, 
               hls::stream<pkt16>& s_axis_tcp_rx_meta, 
               hls::stream<pkt512>& s_axis_tcp_rx_data, 
               hls::stream<pkt32>& m_axis_tcp_tx_meta, 
               hls::stream<pkt512>& m_axis_tcp_tx_data, 
               hls::stream<pkt64>& s_axis_tcp_tx_status,
               int useConn, 
               int listenPort, 
            //    int expectedRxPkgCnt,
            //    int expectedTxPkgCnt,
               int row_num,
               int max_vocab_size,
               ap_uint<64> expectedRxByteCnt,
               ap_uint<64> expectedTxByteCnt,
               int destIpAddress,
               int destPort,
               int pkgWordCount
                      ) {


#pragma HLS INTERFACE axis port = s_axis_udp_rx
#pragma HLS INTERFACE axis port = m_axis_udp_tx
#pragma HLS INTERFACE axis port = s_axis_udp_rx_meta
#pragma HLS INTERFACE axis port = m_axis_udp_tx_meta
#pragma HLS INTERFACE axis port = m_axis_tcp_listen_port
#pragma HLS INTERFACE axis port = s_axis_tcp_port_status
#pragma HLS INTERFACE axis port = m_axis_tcp_open_connection
#pragma HLS INTERFACE axis port = s_axis_tcp_open_status
#pragma HLS INTERFACE axis port = m_axis_tcp_close_connection
#pragma HLS INTERFACE axis port = s_axis_tcp_notification
#pragma HLS INTERFACE axis port = m_axis_tcp_read_pkg
#pragma HLS INTERFACE axis port = s_axis_tcp_rx_meta
#pragma HLS INTERFACE axis port = s_axis_tcp_rx_data
#pragma HLS INTERFACE axis port = m_axis_tcp_tx_meta
#pragma HLS INTERFACE axis port = m_axis_tcp_tx_data
#pragma HLS INTERFACE axis port = s_axis_tcp_tx_status
#pragma HLS INTERFACE s_axilite port=useConn bundle = control
#pragma HLS INTERFACE s_axilite port=listenPort bundle = control
#pragma HLS INTERFACE s_axilite port=row_num bundle = control
#pragma HLS INTERFACE s_axilite port=max_vocab_size bundle = control
#pragma HLS INTERFACE s_axilite port=expectedRxByteCnt bundle = control
#pragma HLS INTERFACE s_axilite port=expectedTxByteCnt bundle = control
#pragma HLS INTERFACE s_axilite port=destIpAddress bundle = control
#pragma HLS INTERFACE s_axilite port=destPort bundle = control
#pragma HLS INTERFACE s_axilite port=pkgWordCount bundle = control

#pragma HLS INTERFACE s_axilite port = return bundle = control

static hls::stream<ap_uint<512> >    s_data_in;
#pragma HLS STREAM variable=s_data_in depth=512

static hls::stream<ap_uint<512> >    s_data_out;
#pragma HLS STREAM variable=s_data_out depth=512

    hls::stream<ap_uint<512> > input_stream_0;
#pragma HLS stream variable=input_stream_0 depth=256
    hls::stream<ap_uint<512> > input_stream_1;
#pragma HLS stream variable=input_stream_1 depth=256
    hls::stream<ap_uint<512> > input_stream_2;
#pragma HLS stream variable=input_stream_2 depth=256

    hls::stream<ap_uint<512> > numerical_stream_0;
#pragma HLS stream variable=numerical_stream_0 depth=256

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


        //   int pkgWordCount = 64;
        //   ap_uint<64> pkgBytesCount = pkgWordCount * 64;
        //   ap_uint<64> expectedTxPkgCnt_64 = expectedTxPkgCnt;
        //   ap_uint<64> expectedRxPkgCnt_64 = expectedRxPkgCnt;
        //   ap_uint<64> expectedTxByteCnt = expectedTxPkgCnt_64 * pkgBytesCount;
        //   ap_uint<64> expectedRxByteCnt = expectedRxPkgCnt_64 * pkgBytesCount;

          ap_uint<16> sessionID [8];

          listenPorts (listenPort, useConn, m_axis_tcp_listen_port, s_axis_tcp_port_status);
          
        //   openConnections( useConn, destIpAddress, destPort, m_axis_tcp_open_connection, s_axis_tcp_open_status, sessionID);

#pragma HLS dataflow

          recvData(expectedRxByteCnt, 
               s_data_in,
               s_axis_tcp_notification, 
               m_axis_tcp_read_pkg, 
               s_axis_tcp_rx_meta, 
               s_axis_tcp_rx_data);
        
        // traffic_gen( expectedRxByteCnt, s_data_in);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FOR COMPUTATION
        Network2Stream(
            s_data_in,
            input_stream_0,
            input_stream_1,
            input_stream_2,
            row_num
        );

        // Numerical features
        NegsToZeroLog(
            input_stream_0,
            numerical_stream_0,
            row_num
        );
        // Categorical features
        HexToIntModRange(
            input_stream_1,
            categorical_stream_0,
            row_num,
            max_vocab_size
        );
        HexToIntModRange(
            input_stream_2,
            categorical_stream_1,
            row_num,
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
        row_num
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
        row_num
    );

    HashDict(
        categorical_stream_split_16_0_0,
        categorical_dict_value_16_0_0,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_1,
        categorical_dict_value_16_0_1,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_2,
        categorical_dict_value_16_0_2,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_3,
        categorical_dict_value_16_0_3,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_4,
        categorical_dict_value_16_0_4,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_5,
        categorical_dict_value_16_0_5,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_6,
        categorical_dict_value_16_0_6,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_7,
        categorical_dict_value_16_0_7,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_8,
        categorical_dict_value_16_0_8,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_9,
        categorical_dict_value_16_0_9,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_10,
        categorical_dict_value_16_0_10,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_11,
        categorical_dict_value_16_0_11,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_12,
        categorical_dict_value_16_0_12,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_13,
        categorical_dict_value_16_0_13,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_14,
        categorical_dict_value_16_0_14,
        row_num
    );
    HashDict(
        categorical_stream_split_16_0_15,
        categorical_dict_value_16_0_15,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_0,
        categorical_dict_value_16_1_0,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_1,
        categorical_dict_value_16_1_1,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_2,
        categorical_dict_value_16_1_2,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_3,
        categorical_dict_value_16_1_3,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_4,
        categorical_dict_value_16_1_4,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_5,
        categorical_dict_value_16_1_5,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_6,
        categorical_dict_value_16_1_6,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_7,
        categorical_dict_value_16_1_7,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_8,
        categorical_dict_value_16_1_8,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_9,
        categorical_dict_value_16_1_9,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_10,
        categorical_dict_value_16_1_10,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_11,
        categorical_dict_value_16_1_11,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_12,
        categorical_dict_value_16_1_12,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_13,
        categorical_dict_value_16_1_13,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_14,
        categorical_dict_value_16_1_14,
        row_num
    );
    HashDict(
        categorical_stream_split_16_1_15,
        categorical_dict_value_16_1_15,
        row_num
    );

    CreateDict(
        categorical_dict_value_16_0_0,
        categorical_dict_16_0_0,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_1,
        categorical_dict_16_0_1,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_2,
        categorical_dict_16_0_2,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_3,
        categorical_dict_16_0_3,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_4,
        categorical_dict_16_0_4,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_5,
        categorical_dict_16_0_5,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_6,
        categorical_dict_16_0_6,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_7,
        categorical_dict_16_0_7,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_8,
        categorical_dict_16_0_8,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_9,
        categorical_dict_16_0_9,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_10,
        categorical_dict_16_0_10,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_11,
        categorical_dict_16_0_11,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_12,
        categorical_dict_16_0_12,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_13,
        categorical_dict_16_0_13,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_14,
        categorical_dict_16_0_14,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_0_15,
        categorical_dict_16_0_15,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_0,
        categorical_dict_16_1_0,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_1,
        categorical_dict_16_1_1,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_2,
        categorical_dict_16_1_2,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_3,
        categorical_dict_16_1_3,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_4,
        categorical_dict_16_1_4,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_5,
        categorical_dict_16_1_5,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_6,
        categorical_dict_16_1_6,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_7,
        categorical_dict_16_1_7,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_8,
        categorical_dict_16_1_8,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_9,
        categorical_dict_16_1_9,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_10,
        categorical_dict_16_1_10,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_11,
        categorical_dict_16_1_11,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_12,
        categorical_dict_16_1_12,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_13,
        categorical_dict_16_1_13,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_14,
        categorical_dict_16_1_14,
        row_num
    );
    CreateDict(
        categorical_dict_value_16_1_15,
        categorical_dict_16_1_15,
        row_num
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
        row_num
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
        row_num
    );

        Stream2Network(
            numerical_stream_0,
            categorical_stream_combine_0,
            categorical_stream_combine_1,
            s_data_out,
            row_num
        );

//////////////////////////////////////////////////////////////////////////////////////////////////////////

        

        //   sendData( m_axis_tcp_tx_meta, m_axis_tcp_tx_data, s_axis_tcp_tx_status, s_data_out, sessionID, useConn, expectedTxByteCnt, pkgWordCount);


          tie_off_udp(s_axis_udp_rx, 
               m_axis_udp_tx, 
               s_axis_udp_rx_meta, 
               m_axis_udp_tx_meta);

        tie_off_tcp_open_connection(m_axis_tcp_open_connection, 
               s_axis_tcp_open_status);


        tie_off_tcp_tx(m_axis_tcp_tx_meta, 
                        m_axis_tcp_tx_data, 
                        s_axis_tcp_tx_status);


        //   tie_off_tcp_listen_port( m_axis_tcp_listen_port, 
        //        s_axis_tcp_port_status);

          
        //   tie_off_tcp_rx(s_axis_tcp_notification, 
        //        m_axis_tcp_read_pkg, 
        //        s_axis_tcp_rx_meta, 
        //        s_axis_tcp_rx_data);
    
          tie_off_tcp_close_con(m_axis_tcp_close_connection);

     }
}