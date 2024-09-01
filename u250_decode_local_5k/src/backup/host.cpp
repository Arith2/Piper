#include "host.hpp"
#include <unistd.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.hpp"

#define BANK_NAME(n) n | XCL_MEM_TOPOLOGY
/* for u250 specifically */
const int bank[32] = {
    /* 0 ~ 31 DDR */
    BANK_NAME(0),  BANK_NAME(1),  BANK_NAME(2),  BANK_NAME(3)};

int main(int argc, char** argv)
{
    // if (argc != 2) {
    //     std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
	// 	return EXIT_FAILURE;
	// }
    auto start_program = std::chrono::high_resolution_clock::now();

    // std::ifstream file("/home/yuzhuyu/u55c/criteo_sharded/train_512/train_512-00000-of-00064", std::ios::binary); // Open the CSV file in binary mode

    FILE *fileptr;
    ull filelen;

    // fileptr = fopen("/home/yuzhuyu/u55c/criteo_sharded/train_1/train.txt", "rb"); 
    // fileptr = fopen("/home/yuzhuyu/u55c/criteo_sharded/train_8/train_8-00000-of-00008", "rb"); 
    // fileptr = fopen("/home/yuzhuyu/u55c/criteo_sharded/train_32/train_32-00000-of-00004", "rb"); 
    // fileptr = fopen("/home/yuzhuyu/u55c/criteo_sharded/train_256/train_256-00000-of-00032", "rb");  
    fileptr = fopen("/home/yuzhuyu/u55c/criteo_sharded/train_512/train_512-00000-of-00064", "rb");     

    fseek(fileptr, 0, SEEK_END);          
    filelen = ftell(fileptr);            
    rewind(fileptr);   

    auto checkpoint_1 = std::chrono::high_resolution_clock::now();                   

    // ** Define some adjustable parameters
    int max_vocab_size = 5000;

    ull DDR_table0_size = 0;
    ull DDR_table1_size = 0;
    ull DDR_table2_size = 0;

    // change kernel port width to be 64-bit to ensure index in the range
    ull filelen_per_64 = filelen / 64 / 3;
    DDR_table0_size = filelen_per_64 * 64;//SINGLE_BUFFER_LIMIT;
    DDR_table1_size = DDR_table0_size;//SINGLE_BUFFER_LIMIT;
    DDR_table2_size = filelen - 2*DDR_table0_size;
    // }

    // the data is in 8-bit 
    // Memory Input width is 512-bit
    // ull row_num = (ull)1073741824 * 6 - 3;// filelen;// / 64 + 1;
    ull row_num = filelen;

    std::cout << "Row number: " << row_num << std::endl;
    std::cout << "DDR table 0 size: " << DDR_table0_size << std::endl;
    std::cout << "DDR table 1 size: " << DDR_table1_size << std::endl;
    std::cout << "DDR table 2 size: " << DDR_table2_size << std::endl;
    // // Assert whether one DDR channel is enough to hold all data
    // assert((DDR_table0_size < 8*1024*1024) && "Dataset exceeds one DDR channel");

    // allocate aligned 2D vectors
//////////////////////////////   TEMPLATE START  //////////////////////////////
    std::vector<char, aligned_allocator<char > > DDR_table0(DDR_table0_size, 0);
    std::vector<char, aligned_allocator<char > > DDR_table1(DDR_table1_size, 0);
    std::vector<char, aligned_allocator<char > > DDR_table2(DDR_table2_size, 0);

    // char *buffer;
    // buffer = (char *)malloc((filelen+1)*sizeof(char)); 

    // for(ull i = 0; i < filelen; i++) {
    //     if (i < DDR_table0_size) {
    //         DDR_table0[i] = fgetc(fileptr);  
    //     }
    //     else if (i < 2*DDR_table0_size) {
    //         DDR_table1[i-DDR_table0_size] = fgetc(fileptr);  
    //     }
    //     else {
    //         DDR_table2[i-2*DDR_table0_size] = fgetc(fileptr);  
    //     }
    // }

    // Assuming DDR_table0_size, DDR_table1, and DDR_table2 are appropriately sized

    // Read data for DDR_table0
    size_t bytesRead1 = fread(DDR_table0.data(), 1, DDR_table0_size, fileptr);
    size_t bytesRead2 = fread(DDR_table1.data(), 1, DDR_table1_size, fileptr);
    size_t bytesRead3 = fread(DDR_table2.data(), 1, DDR_table2_size, fileptr);
    std::cout << "bytesRead1: " << bytesRead1 << ", bytesRead2: " << bytesRead2 << ", bytesRead3: " << bytesRead3 << std::endl;
    


    auto checkpoint_2 = std::chrono::high_resolution_clock::now();                   


    // printf("File len: %ld\n", filelen);
    std::cout << "File length: " << filelen << std::endl;
    // printf("%s\n",*&buffer);
    fclose(fileptr); // Close the file
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






//////////////////////////////   TEMPLATE START  //////////////////////////////
    OCL_CHECK(err, cl::Buffer buffer_DDR_table0(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, DDR_table0_size *sizeof(char), &DDR_table0Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_DDR_table1(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, DDR_table1_size *sizeof(char), &DDR_table1Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_DDR_table2(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, DDR_table2_size *sizeof(char), &DDR_table2Ext, &err));
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
    OCL_CHECK(err, err = krnl_vector_add.setArg(3, row_num));
    OCL_CHECK(err, err = krnl_vector_add.setArg(4, max_vocab_size));
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

    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({
        buffer_DDR_table0,
        buffer_DDR_table1,
        buffer_DDR_table2
        },CL_MIGRATE_MEM_OBJECT_HOST));

    q.finish();

    auto checkpoint_4 = std::chrono::high_resolution_clock::now();
    FILE* fileptr_out = fopen("output_result.bin", "wb"); // Open in binary write mode
    if (fileptr_out) {
        std::cout << "Write back to disk" << std::endl;
        // Write the contents of DDR_table0 to the file
        fwrite(DDR_table0.data(), 1, DDR_table0_size, fileptr_out);

        // Write the contents of DDR_table1 to the file
        fwrite(DDR_table1.data(), 1, DDR_table1_size, fileptr_out);

        // Write the contents of DDR_table2 to the file
        fwrite(DDR_table2.data(), 1, DDR_table2_size, fileptr_out);

        fclose(fileptr_out);  // Always close the file when done
    } else {
        // Handle file opening error
        std::cerr << "Error opening the output file." << std::endl;
    }

    auto end_program = std::chrono::high_resolution_clock::now();
    

    auto durationUs_1 = (std::chrono::duration_cast<std::chrono::nanoseconds>(checkpoint_1-start_program).count() / 1000.0);

    auto durationUs_2 = (std::chrono::duration_cast<std::chrono::nanoseconds>(checkpoint_2-checkpoint_1).count() / 1000.0);

    auto durationUs_3 = (std::chrono::duration_cast<std::chrono::nanoseconds>(checkpoint_3-checkpoint_2).count() / 1000.0);

    auto durationUs_4 = (std::chrono::duration_cast<std::chrono::nanoseconds>(checkpoint_4-checkpoint_3).count() / 1000.0);

    // auto durationUs_mem = (std::chrono::duration_cast<std::chrono::nanoseconds>(start_write-checkpoint_3).count() / 1000.0);
    // auto durationUs_write = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_mem-start_write).count() / 1000.0);

    auto durationUs_5 = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_program-checkpoint_4).count() / 1000.0);

    auto durationUs_program = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_program-start_program).count() / 1000.0);

    printf("Duration of getting line number: %f us\n", durationUs_1);
    printf("Duration of assigning host buffer: %f us\n", durationUs_2);
    printf("Duration of reading xclbin: %f us\n", durationUs_3);
    printf("Duration of kernel execution: %f us\n", durationUs_4);
    printf("Duration of writing to disk: %f us\n", durationUs_5);
    printf("Duration of the whole program: %f us\n", durationUs_program);
    
// OPENCL HOST CODE AREA END

    // std::cout << "DDR data: " << std::endl;
    // for (int j = 0; j < DDR_table1_size; j++) {
    //     std::cout << DDR_table1[j] << ' '; 
    // }
    // std::cout << std::endl;
    
// ============================================================================
// Step 3: Release Allocated Resources
// ============================================================================
    delete[] fileBuf;

    return 0;
}

