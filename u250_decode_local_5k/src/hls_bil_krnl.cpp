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
               ap_uint<64> input_file_bytes,
               ap_uint<64> expectedRxByteCnt,
               ap_uint<64> expectedTxByteCnt,
               int destIpAddress,
               int destPort,
               int pkgWordCount,
               ap_uint<64> expectedRxPacket
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
#pragma HLS INTERFACE s_axilite port=input_file_bytes bundle = control
#pragma HLS INTERFACE s_axilite port=expectedRxByteCnt bundle = control
#pragma HLS INTERFACE s_axilite port=expectedTxByteCnt bundle = control
#pragma HLS INTERFACE s_axilite port=destIpAddress bundle = control
#pragma HLS INTERFACE s_axilite port=destPort bundle = control
#pragma HLS INTERFACE s_axilite port=pkgWordCount bundle = control
#pragma HLS INTERFACE s_axilite port=expectedRxPacket bundle = control

#pragma HLS INTERFACE s_axilite port = return bundle = control

static hls::stream<ap_uint<512> >    s_data_in;
#pragma HLS STREAM variable=s_data_in depth=512

static hls::stream<ap_uint<512> >    s_data_out;
#pragma HLS STREAM variable=s_data_out depth=512

    hls::stream<ap_uint<32> > ddr_stream_32[1];
#pragma HLS stream variable=ddr_stream_32 depth=256
    hls::stream<ap_uint<16> > ddr_stream_16[1];
#pragma HLS stream variable=ddr_stream_16 depth=256
    hls::stream<ap_uint<16> > ddr_stream[1];
#pragma HLS stream variable=ddr_stream depth=256
    hls::stream<ap_uint<16> > ddr_stream_filter[1];
#pragma HLS stream variable=ddr_stream_filter depth=256

    hls::stream<ap_uint<16> > ascii_stream[1];
#pragma HLS stream variable=ascii_stream depth=256

    hls::stream<ap_uint<64> > decode_stream[1];
#pragma HLS stream variable=decode_stream depth=256
    hls::stream<ap_uint<64> > numerical_stream[1];
#pragma HLS stream variable=numerical_stream depth=256

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
#pragma HLS stream variable=categorical_dict_output depth=256


        //   int pkgWordCount = 64;
        //   ap_uint<64> pkgBytesCount = pkgWordCount * 64;
        //   ap_uint<64> expectedTxPkgCnt_64 = expectedTxPkgCnt;
        //   ap_uint<64> expectedRxPkgCnt_64 = expectedRxPkgCnt;
        //   ap_uint<64> expectedTxByteCnt = expectedTxPkgCnt_64 * pkgBytesCount;

        //   ap_uint<64> num_packet = input_file_bytes / 64;
        //   ap_uint<64> packet_remain = input_file_bytes - input_file_bytes * 64;
        //   ap_uint<64> expectedRxPacket = (packet_remain > 0) ? (num_packet + 1) : num_packet;
        //   ap_uint<64> expectedRxByteCnt = expectedRxPacket * 64;

          ap_uint<16> sessionID [8];

        //   listenPorts (listenPort, useConn, m_axis_tcp_listen_port, s_axis_tcp_port_status);
          
          openConnections( useConn, destIpAddress, destPort, m_axis_tcp_open_connection, s_axis_tcp_open_status, sessionID);

#pragma HLS dataflow

        //   recvData(expectedRxByteCnt, 
        //        s_data_in,
        //        s_axis_tcp_notification, 
        //        m_axis_tcp_read_pkg, 
        //        s_axis_tcp_rx_meta, 
        //        s_axis_tcp_rx_data);
        
        traffic_gen( expectedRxByteCnt, s_data_in);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
        // FOR COMPUTATION

        // transform from 512-bit to 32-bit
        Network2Stream(
            s_data_in,
            ddr_stream[0], 
            input_file_bytes,
            pkgWordCount,
            expectedRxPacket
        );
        // filter unnecessary 32-bit due to patching
        Network2StreamFilter(
            ddr_stream[0], 
            ddr_stream_filter[0], 
            input_file_bytes,
            pkgWordCount,
            expectedRxPacket
        );

        DecodeASCII(
            ddr_stream_filter[0], 
            ascii_stream[0],
            input_file_bytes
        );

        DecodeUTF8(
            ascii_stream[0],
            decode_stream[0],
            // categorical_stream[0],
            input_file_bytes
        );

        DecodeFilter(
            decode_stream[0],
            numerical_stream[0],
            // categorical_stream[0],
            input_file_bytes
        );
        // Numerical features
        // Log operation
        Neg2ZeroLog(
            numerical_stream[0],
            numerical_stream_log[0],
            input_file_bytes
        );

        // Categorical features
        // Hex to Integer Modulo
        Hex2IntMod(
            // categorical_stream[0],
            numerical_stream_log[0],
            categorical_stream_int[0],
            input_file_bytes,
            max_vocab_size
        );
        MapDict(
            categorical_stream_int[0],
            categorical_dict_value[0], 
            input_file_bytes
        );
        CreateDict(
            categorical_dict_value[0],
            categorical_dict_output[0], 
            input_file_bytes
        );


        Stream2Network(
            categorical_dict_output[0], 
            s_data_out,
            row_num
        );

//////////////////////////////////////////////////////////////////////////////////////////////////////////

        

          sendData( m_axis_tcp_tx_meta, m_axis_tcp_tx_data, s_axis_tcp_tx_status, s_data_out, sessionID, useConn, expectedTxByteCnt, pkgWordCount);


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