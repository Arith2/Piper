#include "host.hpp"
#include <unistd.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "constants.hpp"

#define BANK_NAME(n) n | XCL_MEM_TOPOLOGY
/* for u250 specifically */
const int bank[8] = {
    /* 0 ~ 4 DDR */
    BANK_NAME(0),  BANK_NAME(1),  BANK_NAME(2),  BANK_NAME(3),  
    /* 5 ~ 8 PLRAM */
    BANK_NAME(4),  BANK_NAME(5),  BANK_NAME(6),  BANK_NAME(7)};


void loadData(const std::string& fileName, long startPos, long readBytes, std::vector<int, aligned_allocator<int> >& outputArray) {
    std::ifstream file(fileName, std::ios::binary);
    if (!file) {
        std::cerr << "Unable to open file" << std::endl;
        return;
    }
    // Move to the starting position for this portion
    file.seekg(startPos);

    file.read(reinterpret_cast<char*>(outputArray.data()), readBytes);
    size_t bytesRead = file.gcount();  
    std::cout << "StartPos: " << startPos << ", Expected Read Bytes: " << readBytes << ", Real Read Bytes: " << bytesRead << std::endl;

    file.close();
}

int main(int argc, char** argv)
{

    auto start_program = std::chrono::high_resolution_clock::now();  

    auto file_name = "/home/yuzhuyu/u55c/criteo_sharded/train_1/train.txt";
    // auto file_name = "/home/yuzhuyu/u55c/criteo_sharded/train_512/train_512-00000-of-00064";
    // auto file_name = "/home/yuzhuyu/u55c/criteo_sharded/train_8/train_8-00000-of-00008";
    
    std::cout << "File name: " << file_name << std::endl;
    std::ifstream file(file_name);

    // ull DDR_table_bytes = (ull)4 * 1024 * 1024 * 1024;   

    if (!file) {
        std::cerr << "Failed to open file!" << std::endl;
        return 0;
    }

    file.clear(); // clear the error flags

    file.seekg(0, std::ios::end);
    ull filelen = file.tellg();
    std::cout << "File size: " << filelen << std::endl;


    // long bugPos = 0;
    // long bugBytes = 128;
    // file.seekg(bugPos);
    // std::vector<char, aligned_allocator<char >> bug_table(bugBytes, 0);
    // file.read(bug_table, bugBytes);
    // for (long i = 0; i < bugBytes; i++) {
    //     std::cout << std::dec << bug_table[i] << " " << std::dec << std::endl;
    // }
 

    // file.clear(); // clear the error flags    
    file.seekg(0, std::ios::beg); 
    ull csv_line_num = 0;
    std::string line_num;
    while (std::getline(file, line_num)) {
        csv_line_num++;
    }
    std::cout << "csv_line_num: " << csv_line_num << std::endl;     

    file.close();

    auto checkpoint_1 = std::chrono::high_resolution_clock::now();   

    // ** Define some adjustable parameters
    int max_vocab_size = 5000;

    ull input_bytes = filelen;// In Bytes;

    ull filelen_per_4 = filelen / 4;

    ull DDR_input0_int = filelen_per_4 / 3;//SINGLE_BUFFER_LIMIT;
    ull DDR_input1_int = filelen_per_4 / 3;//SINGLE_BUFFER_LIMIT;
    ull DDR_input2_int = (filelen % 4 > 0) ? (filelen / 4 + 1 - DDR_input0_int - DDR_input1_int) : (filelen / 4 - DDR_input0_int - DDR_input1_int);

    ull DDR_input0_bytes = DDR_input0_int * 4;
    ull DDR_input1_bytes = DDR_input1_int * 4;
    ull DDR_input2_bytes = filelen - DDR_input0_bytes - DDR_input1_bytes;

    
    // ull output_bytes = output_total_bytes; // Real number 

    std::cout << "Input total bytes: " << input_bytes << std::endl;
    // std::cout << "Output total bytes: " << output_bytes << std::endl;
    std::cout << "input data size: " << DDR_input0_int <<  ", bytes: " << DDR_input0_bytes << std::endl;
    std::cout << "input data size: " << DDR_input1_int <<  ", bytes: " << DDR_input1_bytes << std::endl;
    std::cout << "input data size: " << DDR_input2_int <<  ", bytes: " << DDR_input2_bytes << std::endl;
    // std::cout << "input data size: " << DDR_input3_bytes << std::endl;

    // // Assert whether one DDR channel is enough to hold all data
    // assert((DDR_table_bytes < 8*1024*1024) && "Dataset exceeds one DDR channel");

    // allocate aligned 2D vectors
//////////////////////////////   TEMPLATE START  //////////////////////////////
    std::vector<int, aligned_allocator<int >> DDR_table0(DDR_input0_int, 0);
    std::vector<int, aligned_allocator<int >> DDR_table1(DDR_input1_int, 0);
    std::vector<int, aligned_allocator<int >> DDR_table2(DDR_input2_int, 0);
    // std::vector<char, aligned_allocator<char >> DDR_table3(DDR_input3_bytes, 0);

    auto checkpoint_buffer = std::chrono::high_resolution_clock::now();    

    // file.clear(); // clear the error flags
    // file.close();

    std::thread t0(loadData, file_name, 0, DDR_input0_bytes, std::ref(DDR_table0));
    std::thread t1(loadData, file_name, DDR_input0_bytes, DDR_input1_bytes, std::ref(DDR_table1));
    std::thread t2(loadData, file_name, (DDR_input0_bytes + DDR_input1_bytes), DDR_input2_bytes, std::ref(DDR_table2));
    // std::thread t3(loadData, file_name, DDR_input2_int, DDR_input3_bytes, std::ref(DDR_table3));

    t0.join();
    t1.join();
    t2.join();
    // t3.join();  

    // for (int j = 0; j < 16; j++) {
    //     uint64_t originalValue = DDR_table0[j];
    //     uint8_t segments[8];
    //     for (int i = 0; i < 8; ++i) {
    //         segments[i] = (originalValue >> (i * 8)) & 0xFF;
    //     }
    //     std::cout << "The 8-bit segments are:" << std::endl;
    //     for (int i = 7; i >= 0; --i) { // Printing in reverse order to match the original 64-bit    layout
    //         std::cout << "Segment " << 7 - i << ": 0x" 
    //                   << std::hex << (int)segments[i] << std::endl;
    //     }
    // }

    auto checkpoint_2 = std::chrono::high_resolution_clock::now();      

    // printf("File len: %ld\n", filelen);
    // std::cout << "File length: " << filelen << std::endl;
    // printf("%s\n",*&buffer);
    // fclose(fileptr); // Close the file
//////////////////////////////   TEMPLATE END  //////////////////////////////
    
// OPENCL HOST CODE AREA START
	
// ------------------------------------------------------------------------------------
// Step 1: Get All PLATFORMS, then search for Target_Platform_Vendor (CL_PLATFORM_VENDOR)
//	   Search for Platform: Xilinx 
// Check if the current platform matches Target_Platform_Vendor
// ------------------------------------------------------------------------------------	
    std::vector<cl::Device> devices = get_devices("Xilinx");
    devices.resize(1);
    cl::Device device = devices[0];

    cl_int err;
    unsigned fileBufSize;

// ------------------------------------------------------------------------------------
// Step 1: Create Context
// ------------------------------------------------------------------------------------
    OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
	
// ------------------------------------------------------------------------------------
// Step 1: Create Command Queue
// ------------------------------------------------------------------------------------
    OCL_CHECK(err, cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err));

// ------------------------------------------------------------------
// Step 1: Load Binary File from disk
// ------------------------------------------------------------------	
    std::string binaryFile = argv[1];
    char* fileBuf = read_binary_file(binaryFile, fileBufSize);
    cl::Program::Binaries bins{{fileBuf, fileBufSize}};
	
// -------------------------------------------------------------
// Step 1: Create the program object from the binary and program the FPGA device with it
// -------------------------------------------------------------	
    OCL_CHECK(err, cl::Program program(context, devices, bins, NULL, &err));

// -------------------------------------------------------------
// Step 1: Create Kernels
// -------------------------------------------------------------
    OCL_CHECK(err, cl::Kernel krnl_vector_add(program,"vadd", &err));

// ================================================================
// Step 2: Setup Buffers and run Kernels
// ================================================================
//   o) Allocate Memory to store the results 
//   o) Create Buffers in Global Memory to store data
// ================================================================

// ------------------------------------------------------------------
// Step 2: Create Buffers in Global Memory to store data
//             o) buffer_in1 - stores source_in1
//             o) buffer_in2 - stores source_in2
//             o) buffer_ouput - stores Results
// ------------------------------------------------------------------	

// .......................................................
// Allocate Global Memory for source_in1
// .......................................................	
//////////////////////////////   TEMPLATE START  //////////////////////////////
    cl_mem_ext_ptr_t    DDR_table0Ext,
                        DDR_table1Ext,
                        DDR_table2Ext;
                        // DDR_table3Ext;
//////////////////////////////   TEMPLATE END  //////////////////////////////

//////////////////////////////   TEMPLATE START  //////////////////////////////
    // sleep(100);
    DDR_table0Ext.obj = DDR_table0.data();
    DDR_table0Ext.param = 0;
    DDR_table0Ext.flags = bank[0];
    DDR_table1Ext.obj = DDR_table1.data();
    DDR_table1Ext.param = 0;
    DDR_table1Ext.flags = bank[1];
    DDR_table2Ext.obj = DDR_table2.data();
    DDR_table2Ext.param = 0;
    DDR_table2Ext.flags = bank[2];
    // DDR_table3Ext.obj = DDR_table3.data();
    // DDR_table3Ext.param = 0;
    // DDR_table3Ext.flags = bank[3];
    // DDR_table3Ext.obj = DDR_table3.data();
    // DDR_table3Ext.param = 0;
    // DDR_table3Ext.flags = bank[3];






//////////////////////////////   TEMPLATE START  //////////////////////////////
    OCL_CHECK(err, cl::Buffer buffer_DDR_table0(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, DDR_input0_int *sizeof(int), &DDR_table0Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_DDR_table1(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, DDR_input1_int *sizeof(int), &DDR_table1Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_DDR_table2(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, DDR_input2_int *sizeof(int), &DDR_table2Ext, &err));
    // OCL_CHECK(err, cl::Buffer buffer_DDR_table3(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, DDR_input3_bytes *sizeof(char), &DDR_table3Ext, &err));
//============================================================================
// Step 2: Set Kernel Arguments and Run the Application
//         o) Set Kernel Arguments
//         o) Copy Input Data from Host to Global Memory on the device
//         o) Submit Kernels for Execution
//         o) Copy Results from Global Memory, device to Host
// ============================================================================	
    
//////////////////////////////   TEMPLATE START  //////////////////////////////
    OCL_CHECK(err, err = krnl_vector_add.setArg(0, buffer_DDR_table0));
    OCL_CHECK(err, err = krnl_vector_add.setArg(1, buffer_DDR_table1));
    OCL_CHECK(err, err = krnl_vector_add.setArg(2, buffer_DDR_table2));
    // OCL_CHECK(err, err = krnl_vector_add.setArg(3, buffer_DDR_table3));
    OCL_CHECK(err, err = krnl_vector_add.setArg(3, input_bytes));
    OCL_CHECK(err, err = krnl_vector_add.setArg(4, csv_line_num));
    // OCL_CHECK(err, err = krnl_vector_add.setArg(5, DDR_table_bytes));
    OCL_CHECK(err, err = krnl_vector_add.setArg(5, max_vocab_size));
//////////////////////////////   TEMPLATE END  //////////////////////////////
// ------------------------------------------------------
// Step 2: Copy Input data from Host to Global Memory on the device
// ------------------------------------------------------
//////////////////////////////   TEMPLATE START  //////////////////////////////
    auto checkpoint_3 = std::chrono::high_resolution_clock::now();

    OCL_CHECK(
        err, err = q.enqueueMigrateMemObjects({
        buffer_DDR_table0,
        buffer_DDR_table1,
        buffer_DDR_table2
        // buffer_DDR_table3
        }, 0/* 0 means from host*/));	
    // q.finish();
    // auto end_write_mem = std::chrono::high_resolution_clock::now();
    // auto durationUs_write_mem = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_write_mem-checkpoint_3).count() / 1000.0);
    // printf("Duration of write mem :%f us\n", durationUs_write_mem);
//////////////////////////////   TEMPLATE END  //////////////////////////////
// ----------------------------------------
// Step 2: Submit Kernels for Execution
// ----------------------------------------
    // auto start_task = std::chrono::high_resolution_clock::now();

    OCL_CHECK(err, err = q.enqueueTask(krnl_vector_add));
    // q.finish();


    // auto end_task = std::chrono::high_resolution_clock::now();
    // auto durationUs_ask = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_task-end_write_mem).count() / 1000.0);
    // printf("Duration of task :%f us\n", durationUs_ask);
// --------------------------------------------------
// Step 2: Copy Results from Device Global Memory to Host
// --------------------------------------------------
    // auto start_mem = std::chrono::high_resolution_clock::now();

    // OCL_CHECK(err, err = q.enqueueMigrateMemObjects({
    //     buffer_DDR_table0,
    //     buffer_DDR_table1,
    //     buffer_DDR_table2
    //     },CL_MIGRATE_MEM_OBJECT_HOST));

    q.finish();

    auto checkpoint_4 = std::chrono::high_resolution_clock::now();
    // FILE* fileptr_out = fopen("output_result.bin", "wb"); // Open in binary write mode
    // if (fileptr_out) {
    //     std::cout << "Write back to disk" << std::endl;
    //     // Write the contents of DDR_table0 to the file
    //     fwrite(DDR_table0.data(), 1, DDR_output0_bytes, fileptr_out);

    //     // Write the contents of DDR_table1 to the file
    //     fwrite(DDR_table1.data(), 1, DDR_output1_bytes, fileptr_out);

    //     // Write the contents of DDR_table2 to the file
    //     fwrite(DDR_table2.data(), 1, DDR_output2_bytes, fileptr_out);

    //     fclose(fileptr_out);  // Always close the file when done
    // } else {
    //     // Handle file opening error
    //     std::cerr << "Error opening the output file." << std::endl;
    // }

    auto end_program = std::chrono::high_resolution_clock::now();
    

    auto durationUs_buffer = (std::chrono::duration_cast<std::chrono::nanoseconds>(checkpoint_1-start_program).count() / 1000.0);

    auto durationUs_1 = (std::chrono::duration_cast<std::chrono::nanoseconds>(checkpoint_buffer-checkpoint_1).count() / 1000.0);

    auto durationUs_2 = (std::chrono::duration_cast<std::chrono::nanoseconds>(checkpoint_2-checkpoint_buffer).count() / 1000.0);

    auto durationUs_3 = (std::chrono::duration_cast<std::chrono::nanoseconds>(checkpoint_3-checkpoint_2).count() / 1000.0);

    auto durationUs_4 = (std::chrono::duration_cast<std::chrono::nanoseconds>(checkpoint_4-checkpoint_3).count() / 1000.0);

    // auto durationUs_mem = (std::chrono::duration_cast<std::chrono::nanoseconds>(start_write-checkpoint_3).count() / 1000.0);
    // auto durationUs_write = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_mem-start_write).count() / 1000.0);

    auto durationUs_5 = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_program-checkpoint_4).count() / 1000.0);

    auto durationUs_program = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_program-start_program).count() / 1000.0);

    printf("Duration of getting line number: %f us\n", durationUs_buffer);
    printf("Duration of initializing buffer: %f us\n", durationUs_1);
    printf("Duration of assigning host buffer: %f us\n", durationUs_2);
    printf("Duration of reading xclbin: %f us\n", durationUs_3);
    printf("Duration of kernel execution: %f us\n", durationUs_4);
    printf("Duration of writing to disk: %f us\n", durationUs_5);
    printf("Duration of the whole program: %f us\n", durationUs_program);
    
// OPENCL HOST CODE AREA END

    // std::cout << "DDR data: " << std::endl;
    // for (int j = 0; j < DDR_table_bytes; j++) {
    //     std::cout << DDR_table1[j] << ' '; 
    // }
    // std::cout << std::endl;
    
// ============================================================================
// Step 3: Release Allocated Resources
// ============================================================================
    delete[] fileBuf;

    return 0;
}

