/**********
Copyright (c) 2019, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/
#include "xcl2.hpp"
#include <vector>
#include <chrono>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DATA_SIZE 62500000

//Set IP address of FPGA
#define IP_ADDR 0x0A01D498
#define BOARD_NUMBER 0
#define ARP 0x0A01D498

#define BANK_NAME(n) n | XCL_MEM_TOPOLOGY
// for U55c
const int bank[32] = {
    /* 0 ~ 31 HBM */
    BANK_NAME(0),  BANK_NAME(1),  BANK_NAME(2),  BANK_NAME(3),  BANK_NAME(4),
    BANK_NAME(5),  BANK_NAME(6),  BANK_NAME(7),  BANK_NAME(8),  BANK_NAME(9),
    BANK_NAME(10), BANK_NAME(11), BANK_NAME(12), BANK_NAME(13), BANK_NAME(14),
    BANK_NAME(15), BANK_NAME(16), BANK_NAME(17), BANK_NAME(18), BANK_NAME(19),
    BANK_NAME(20), BANK_NAME(21), BANK_NAME(22), BANK_NAME(23), BANK_NAME(24),
    BANK_NAME(25), BANK_NAME(26), BANK_NAME(27), BANK_NAME(28), BANK_NAME(29),
    BANK_NAME(30), BANK_NAME(31)};

void wait_for_enter(const std::string &msg) {
    std::cout << msg << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> [<#Row_num> <local BoardNum> <Dest BoardNum> <Dest IP> <Dest Port> <Packet Word>]" << std::endl;
        return EXIT_FAILURE;
    }

    std::string binaryFile = argv[1];

    cl_int err;
    cl::CommandQueue q;
    cl::Context context;

    cl::Kernel user_kernel;
    cl::Kernel network_kernel;

    auto size = DATA_SIZE;
    
    //Allocate Memory in Host Memory
    auto vector_size_bytes = sizeof(int) * size;
    std::vector<int, aligned_allocator<int>> network_ptr0(size);
    std::vector<int, aligned_allocator<int>> network_ptr1(size);


    //OPENCL HOST CODE AREA START
    //Create Program and Kernel
    auto devices = xcl::get_xil_devices();

    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    int valid_device = 0;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context({device}, NULL, NULL, NULL, &err));
        OCL_CHECK(err,
                  q = cl::CommandQueue(
                      context, {device}, CL_QUEUE_PROFILING_ENABLE, &err));

        std::cout << "Trying to program device[" << i
                  << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
                  cl::Program program(context, {device}, bins, NULL, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i
                      << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            OCL_CHECK(err,
                      network_kernel = cl::Kernel(program, "network_krnl", &err));
            OCL_CHECK(err,
                      user_kernel = cl::Kernel(program, "hls_bil_krnl", &err));
            valid_device++;
            break; // we break because we found a valid device
        }
    }
    if (valid_device == 0) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }
    
    uint32_t local_IP = 0x0A01D498;
    uint32_t local_boardNum = 1;
    uint32_t local_Port = 5001; 

    uint32_t connection = 1;
    // uint32_t rxPkt = 100;
    // uint32_t txPkt = 100;
    uint32_t row_num = 100;
    uint32_t max_vocab_size = 1000000;

    uint32_t dest_IP = 0x0A01D46E; //alveo0
    uint32_t dest_Port = 5002;
    uint32_t dest_boardNum = 1;

    uint32_t pkgWordCount = 64;

    int HBM_table_size = 1024; // 

    std::vector<int, aligned_allocator<int> > HBM_table0(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table1(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table2(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table3(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table4(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table5(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table6(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table7(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table8(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table9(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table10(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table11(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table12(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table13(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table14(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table15(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table16(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table17(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table18(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table19(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table20(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table21(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table22(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table23(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table24(HBM_table_size, 0);
    std::vector<int, aligned_allocator<int> > HBM_table25(HBM_table_size, 0);

    if(argc >=3)
    {
        row_num = strtol(argv[2], NULL, 10);
    }

    int ip_local_array [4];
    ip_local_array[0] = 10;
    ip_local_array[1] = 253;
    ip_local_array[2] = 74;
    ip_local_array[3] = 68;
    
    if (argc >= 4)
    {
        local_boardNum = strtol(argv[3], NULL, 10);

        ip_local_array[3] = 68 + (local_boardNum - 1) * 4;
        local_IP = ip_local_array[3] | (ip_local_array[2] << 8) | (ip_local_array[1] << 16) | (ip_local_array[0] << 24);
        local_Port = 5000 + local_boardNum;
     }

    int ip_dest_array [4];

    if (argc >= 5)
    {
        dest_boardNum = strtol(argv[4], NULL, 10);

//////////////////////////////////////////////////////////////////////
// // This part is useless but when removing it, the execution would fail
//         std::string s = argv[4];
//         std::string delimiter = ".";
//         size_t pos = 0;
//         std::string token;
//         int i = 0;
        
//         while ((pos = s.find(delimiter)) != std::string::npos) {
//             token = s.substr(0, pos);
//             ip_dest_array [i] = stoi(token);
//             s.erase(0, pos + delimiter.length());
//             i++;
//         }
////////////////////////////////////////////////////////////////////////
        // ip_dest_array[i] = stoi(s); 
        // 
        ip_dest_array [0] = 10;
        ip_dest_array [1] = 253;
        ip_dest_array [2] = 74;
        ip_dest_array [3] = 68 + (dest_boardNum - 1) * 4;
        dest_IP = ip_dest_array[3] | (ip_dest_array[2] << 8) | (ip_dest_array[1] << 16) | (ip_dest_array[0] << 24);
        dest_Port = 5000 + dest_boardNum;
    }

    if (argc >= 6)
    {
        std::string s = argv[5];
        std::string delimiter = ".";
        // int ip [4];
        size_t pos = 0;
        std::string token;
        int i = 0;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            ip_dest_array [i] = stoi(token);
            s.erase(0, pos + delimiter.length());
            i++;
        }
        ip_dest_array[i] = stoi(s); 
        dest_IP = ip_dest_array[3] | (ip_dest_array[2] << 8) | (ip_dest_array[1] << 16) | (ip_dest_array[0] << 24);
    }

    if (argc >= 7)
    {
        dest_Port = strtol(argv[6], NULL, 10);
    }

    if (argc >= 8)
    {
        pkgWordCount = strtol(argv[7], NULL, 10);
    }

    uint64_t row_num_64 = row_num;
    uint64_t expectedRxByteCnt = 2 * row_num_64 * 3 * 64;
    uint64_t expectedTxByteCnt = 2 * row_num_64 * 3 * 64;

    printf("\nlocal_IP: %d.%d.%d.%d, %x\n", ip_local_array[0], ip_local_array[1], ip_local_array[2], ip_local_array[3], local_IP);
    printf("local_boardNum: %d, local_port: %d\n", local_boardNum, local_Port);
    printf("dest_IP: %d.%d.%d.%d, %x\n", ip_dest_array[0], ip_dest_array[1], ip_dest_array[2], ip_dest_array[3], dest_IP);
    printf("local_boardNum: %d, dest_port: %d\n", dest_boardNum, dest_Port);
    // printf("rxPkt: %d, packetWord: %d \n", rxPkt, numPacketWord);
    printf("row_num: %d, packetWord: %d \n", row_num, pkgWordCount);
    printf("expectedRxByteCnt: %lld\n", expectedRxByteCnt);
    printf("expectedTxByteCnt: %lld\n", expectedTxByteCnt);
    // printf("rxPkt: %d, packetWord: %d \n", rxPkt, pkgWordCount);

    wait_for_enter("\nPress ENTER to continue after setting up ILA trigger...");

    // printf("local_IP:%x, local_boardNum:%d\n", local_IP, local_boardNum);

    // Set network kernel arguments
    OCL_CHECK(err, err = network_kernel.setArg(0, local_IP)); // Default IP address
    OCL_CHECK(err, err = network_kernel.setArg(1, local_boardNum)); // Board number
    OCL_CHECK(err, err = network_kernel.setArg(2, local_IP)); // ARP lookup

    OCL_CHECK(err,
              cl::Buffer buffer_r1(context,
                                   CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                   vector_size_bytes,
                                   network_ptr0.data(),
                                   &err));
    OCL_CHECK(err,
            cl::Buffer buffer_r2(context,
                                CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                vector_size_bytes,
                                network_ptr1.data(),
                                &err));

    OCL_CHECK(err, err = network_kernel.setArg(3, buffer_r1));
    OCL_CHECK(err, err = network_kernel.setArg(4, buffer_r2));

    printf("enqueue network kernel...\n");
    OCL_CHECK(err, err = q.enqueueTask(network_kernel));
    OCL_CHECK(err, err = q.finish());

    cl_mem_ext_ptr_t    HBM_table0Ext, HBM_table1Ext, HBM_table2Ext, HBM_table3Ext, 
                        HBM_table4Ext, HBM_table5Ext, HBM_table6Ext, HBM_table7Ext, 
                        HBM_table8Ext, HBM_table9Ext, HBM_table10Ext, HBM_table11Ext, 
                        HBM_table12Ext, HBM_table13Ext, HBM_table14Ext, HBM_table15Ext, 
                        HBM_table16Ext, HBM_table17Ext, HBM_table18Ext, HBM_table19Ext, 
                        HBM_table20Ext, HBM_table21Ext, HBM_table22Ext, HBM_table23Ext, 
                        HBM_table24Ext, HBM_table25Ext;

    HBM_table0Ext.obj = HBM_table0.data();
    HBM_table0Ext.param = 0;
    HBM_table0Ext.flags = bank[0];
    HBM_table1Ext.obj = HBM_table1.data();
    HBM_table1Ext.param = 0;
    HBM_table1Ext.flags = bank[1];
    HBM_table2Ext.obj = HBM_table2.data();
    HBM_table2Ext.param = 0;
    HBM_table2Ext.flags = bank[2];
    HBM_table3Ext.obj = HBM_table3.data();
    HBM_table3Ext.param = 0;
    HBM_table3Ext.flags = bank[3];
    HBM_table4Ext.obj = HBM_table4.data();
    HBM_table4Ext.param = 0;
    HBM_table4Ext.flags = bank[4];
    HBM_table5Ext.obj = HBM_table5.data();
    HBM_table5Ext.param = 0;
    HBM_table5Ext.flags = bank[5];
    HBM_table6Ext.obj = HBM_table6.data();
    HBM_table6Ext.param = 0;
    HBM_table6Ext.flags = bank[6];
    HBM_table7Ext.obj = HBM_table7.data();
    HBM_table7Ext.param = 0;
    HBM_table7Ext.flags = bank[7];
    HBM_table8Ext.obj = HBM_table8.data();
    HBM_table8Ext.param = 0;
    HBM_table8Ext.flags = bank[8];
    HBM_table9Ext.obj = HBM_table9.data();
    HBM_table9Ext.param = 0;
    HBM_table9Ext.flags = bank[9];
    HBM_table10Ext.obj = HBM_table10.data();
    HBM_table10Ext.param = 0;
    HBM_table10Ext.flags = bank[10];
    HBM_table11Ext.obj = HBM_table11.data();
    HBM_table11Ext.param = 0;
    HBM_table11Ext.flags = bank[11];
    HBM_table12Ext.obj = HBM_table12.data();
    HBM_table12Ext.param = 0;
    HBM_table12Ext.flags = bank[12];
    HBM_table13Ext.obj = HBM_table13.data();
    HBM_table13Ext.param = 0;
    HBM_table13Ext.flags = bank[13];
    HBM_table14Ext.obj = HBM_table14.data();
    HBM_table14Ext.param = 0;
    HBM_table14Ext.flags = bank[14];
    HBM_table15Ext.obj = HBM_table15.data();
    HBM_table15Ext.param = 0;
    HBM_table15Ext.flags = bank[15];
    HBM_table16Ext.obj = HBM_table16.data();
    HBM_table16Ext.param = 0;
    HBM_table16Ext.flags = bank[16];
    HBM_table17Ext.obj = HBM_table17.data();
    HBM_table17Ext.param = 0;
    HBM_table17Ext.flags = bank[17];
    HBM_table18Ext.obj = HBM_table18.data();
    HBM_table18Ext.param = 0;
    HBM_table18Ext.flags = bank[18];
    HBM_table19Ext.obj = HBM_table19.data();
    HBM_table19Ext.param = 0;
    HBM_table19Ext.flags = bank[19];
    HBM_table20Ext.obj = HBM_table20.data();
    HBM_table20Ext.param = 0;
    HBM_table20Ext.flags = bank[20];
    HBM_table21Ext.obj = HBM_table21.data();
    HBM_table21Ext.param = 0;
    HBM_table21Ext.flags = bank[21];
    HBM_table22Ext.obj = HBM_table22.data();
    HBM_table22Ext.param = 0;
    HBM_table22Ext.flags = bank[22];
    HBM_table23Ext.obj = HBM_table23.data();
    HBM_table23Ext.param = 0;
    HBM_table23Ext.flags = bank[23];
    HBM_table24Ext.obj = HBM_table24.data();
    HBM_table24Ext.param = 0;
    HBM_table24Ext.flags = bank[24];
    HBM_table25Ext.obj = HBM_table25.data();
    HBM_table25Ext.param = 0;
    HBM_table25Ext.flags = bank[25];
    // CL_MEM_EXT_PTR_XILINX
//////////////////////////////   TEMPLATE START  //////////////////////////////
    OCL_CHECK(err, cl::Buffer buffer_HBM_table0(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table0Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table1(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table1Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table2(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table2Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table3(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table3Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table4(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table4Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table5(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table5Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table6(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table6Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table7(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table7Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table8(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table8Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table9(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table9Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table10(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table10Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table11(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table11Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table12(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table12Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table13(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table13Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table14(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table14Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table15(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table15Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table16(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table16Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table17(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table17Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table18(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table18Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table19(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table19Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table20(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table20Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table21(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table21Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table22(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table22Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table23(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table23Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table24(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table24Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_HBM_table25(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            HBM_table_size *sizeof(int), &HBM_table25Ext, &err));

    double durationUs = 0.0;

    //Set user Kernel Arguments
    //Set user Kernel Arguments
    OCL_CHECK(err, err = user_kernel.setArg(16, connection));
    OCL_CHECK(err, err = user_kernel.setArg(17, local_Port));
    OCL_CHECK(err, err = user_kernel.setArg(18, row_num));
    OCL_CHECK(err, err = user_kernel.setArg(19, max_vocab_size));
    OCL_CHECK(err, err = user_kernel.setArg(20, expectedRxByteCnt));
    OCL_CHECK(err, err = user_kernel.setArg(21, expectedTxByteCnt));
    OCL_CHECK(err, err = user_kernel.setArg(22, dest_IP));
    OCL_CHECK(err, err = user_kernel.setArg(23, dest_Port));
    OCL_CHECK(err, err = user_kernel.setArg(24, pkgWordCount));

    OCL_CHECK(err, err = user_kernel.setArg(25, buffer_HBM_table0));
    OCL_CHECK(err, err = user_kernel.setArg(26, buffer_HBM_table1));
    OCL_CHECK(err, err = user_kernel.setArg(27, buffer_HBM_table2));
    OCL_CHECK(err, err = user_kernel.setArg(28, buffer_HBM_table3));
    OCL_CHECK(err, err = user_kernel.setArg(29, buffer_HBM_table4));
    OCL_CHECK(err, err = user_kernel.setArg(30, buffer_HBM_table5));
    OCL_CHECK(err, err = user_kernel.setArg(31, buffer_HBM_table6));
    OCL_CHECK(err, err = user_kernel.setArg(32, buffer_HBM_table7));
    OCL_CHECK(err, err = user_kernel.setArg(33, buffer_HBM_table8));
    OCL_CHECK(err, err = user_kernel.setArg(34, buffer_HBM_table9));
    OCL_CHECK(err, err = user_kernel.setArg(35, buffer_HBM_table10));
    OCL_CHECK(err, err = user_kernel.setArg(36, buffer_HBM_table11));
    OCL_CHECK(err, err = user_kernel.setArg(37, buffer_HBM_table12));
    OCL_CHECK(err, err = user_kernel.setArg(38, buffer_HBM_table13));
    OCL_CHECK(err, err = user_kernel.setArg(39, buffer_HBM_table14));
    OCL_CHECK(err, err = user_kernel.setArg(40, buffer_HBM_table15));
    OCL_CHECK(err, err = user_kernel.setArg(41, buffer_HBM_table16));
    OCL_CHECK(err, err = user_kernel.setArg(42, buffer_HBM_table17));
    OCL_CHECK(err, err = user_kernel.setArg(43, buffer_HBM_table18));
    OCL_CHECK(err, err = user_kernel.setArg(44, buffer_HBM_table19));
    OCL_CHECK(err, err = user_kernel.setArg(45, buffer_HBM_table20));
    OCL_CHECK(err, err = user_kernel.setArg(46, buffer_HBM_table21));
    OCL_CHECK(err, err = user_kernel.setArg(47, buffer_HBM_table22));
    OCL_CHECK(err, err = user_kernel.setArg(48, buffer_HBM_table23));
    OCL_CHECK(err, err = user_kernel.setArg(49, buffer_HBM_table24));
    OCL_CHECK(err, err = user_kernel.setArg(50, buffer_HBM_table25));

    //Launch the Kernel
    auto start = std::chrono::high_resolution_clock::now();
    printf("enqueue user kernel...\n");
    OCL_CHECK(err, err = q.enqueueTask(user_kernel));

    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({
            buffer_HBM_table0, buffer_HBM_table1, buffer_HBM_table2, buffer_HBM_table3, 
            buffer_HBM_table4, buffer_HBM_table5, buffer_HBM_table6, buffer_HBM_table7, 
            buffer_HBM_table8, buffer_HBM_table9, buffer_HBM_table10, buffer_HBM_table11, 
            buffer_HBM_table12, buffer_HBM_table13, buffer_HBM_table14, buffer_HBM_table15, 
            buffer_HBM_table16, buffer_HBM_table17, buffer_HBM_table18, buffer_HBM_table19, 
            buffer_HBM_table20, buffer_HBM_table21, buffer_HBM_table22, buffer_HBM_table23, 
            buffer_HBM_table24, buffer_HBM_table25
        },CL_MIGRATE_MEM_OBJECT_HOST));

    OCL_CHECK(err, err = q.finish());
    auto end = std::chrono::high_resolution_clock::now();
    durationUs = (std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() / 1000.0);
    printf("durationUs:%f\n",durationUs);
    //OPENCL HOST CODE AREA END    

    std::cout << "EXIT recorded" << std::endl;
}
