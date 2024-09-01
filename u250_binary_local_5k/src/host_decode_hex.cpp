#include "host.hpp"
#include <unistd.h>
#include <fstream>

#define BANK_NAME(n) n | XCL_MEM_TOPOLOGY
/* for u250 specifically */
const int bank[8] = {
    /* 0 ~ 4 DDR */
    BANK_NAME(0),  BANK_NAME(1),  BANK_NAME(2),  BANK_NAME(3),  
    /* 5 ~ 8 PLRAM */
    BANK_NAME(4),  BANK_NAME(5),  BANK_NAME(6),  BANK_NAME(7)};

int main(int argc, char** argv)
{
    // if (argc != 2) {
    //     std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
	// 	return EXIT_FAILURE;
	// }
    auto start_program = std::chrono::high_resolution_clock::now();
    // std::ifstream file("/home/yuzhuyu/u55c/criteo_sharded/train_512/train_512-00000-of-00064", std::ios::binary); // Open the CSV file in binary mode
    // std::ifstream file("/home/yuzhuyu/u55c/criteo_sharded/train_8/train_8-00000-of-00008", std::ios::binary);
    auto file_name = "/home/yuzhuyu/u55c/criteo_sharded/train_1/train.txt";
    // auto file_name = "/home/yuzhuyu/u55c/criteo_sharded/train_512/train_512-00000-of-00064";
    std::cout << "File name: " << file_name << std::endl;
    std::ifstream file(file_name, std::ios::binary);

    // ** Define some adjustable parameters
    int max_vocab_size = 5000;


    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

    // // Get the number of rows in a csv file to determine the size for storage
    file.seekg(0, std::ios::beg); // Move to the beginning of the file
    int csv_line_num = 0;
    std::string line_num;
    while (std::getline(file, line_num)) {
        csv_line_num ++;
    }
    std::cout << "csv_line_num: " << csv_line_num << std::endl;

    auto check_point_1 = std::chrono::high_resolution_clock::now();

    // Use DDR0 to store 1 lable feature and 13 numerical features which are all 32-bit wide
    // label feature & numerical features are 32-bit wide
    int DDR_table0_size =  csv_line_num * 16;
    // Use DDR1~4 to store 26 categorical features in the form of 16, 10
    // categorical features are 32-bit wide
    int DDR_table1_size =  csv_line_num * 16;
    int DDR_table2_size =  csv_line_num * 16;

    int row_num = csv_line_num; // csv_line_num

    std::cout << "DDR table 0 size: " << DDR_table0_size << std::endl;
    std::cout << "DDR table 1 size: " << DDR_table1_size << std::endl;
    std::cout << "DDR table 2 size: " << DDR_table2_size << std::endl;

    // // Assert whether one DDR channel is enough to hold all data
    // assert((DDR_table0_size < 8*1024*1024) && "Dataset exceeds one DDR channel");
          
    // allocate aligned 2D vectors
//////////////////////////////   TEMPLATE START  //////////////////////////////
    std::vector<int, aligned_allocator<int> > DDR_table0(DDR_table0_size, 0);
    std::vector<int, aligned_allocator<int> > DDR_table1(DDR_table1_size, 0);
    std::vector<int, aligned_allocator<int> > DDR_table2(DDR_table2_size, 0);

    auto check_point_buffer = std::chrono::high_resolution_clock::now();
    auto durationUs_buffer = (std::chrono::duration_cast<std::chrono::nanoseconds>(check_point_buffer-check_point_1).count() / 1000.0);
//////////////////////////////   TEMPLATE END  //////////////////////////////
    // auto start_host = std::chrono::high_resolution_clock::now();
    // ** FillMissing()
    // ** Neglect the condition that the value is missing because the vector is initialized as zero
    // ** Guarantee that the index is correct
    file.clear();  // Clear any error flags
    file.seekg(0, std::ios::beg); // Move to the beginning of the file
    std::string line;
    int DDR_index_0_curr = 0, DDR_index_0_pre = 0;
    int DDR_index_1_curr = 0, DDR_index_1_pre = 0;
    int DDR_index_2_curr = 0, DDR_index_2_pre = 0;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string cell;

        int lineIndex = 0;
        DDR_index_0_curr = DDR_index_0_pre;
        DDR_index_1_curr = DDR_index_1_pre;
        DDR_index_2_curr = DDR_index_2_pre;

        int original32BitData = 0;

        while (std::getline(ss, cell, '\t')) {  // Split the line by space
            // for label key and numerical features, there are 32-bit wide

            // if (lineIndex < (NUM_LABEL_KEY+NUM_NUMERICAL_KEY)) {

            //     DDR_table0[DDR_index_0_curr] = cell.empty() ? 0 : std::stoi(cell, nullptr, 10);
            //     DDR_index_0_curr++;

            // }
            // else {
            //     original32BitData = cell.empty() ? 0 : std::stol(cell, nullptr, 16);
            //     // std::cout << "original32BitData: " << std::hex << (original32BitData>>32) << " " << (original32BitData & 0xffffffff) << std::endl;

            //     if (lineIndex < (NUM_LABEL_KEY+NUM_NUMERICAL_KEY+16)) {
            //         DDR_table1[DDR_index_1_curr] = original32BitData;
            //         DDR_index_1_curr++;
            //     }
            //     else {
            //         DDR_table2[DDR_index_2_curr] = original32BitData;
            //         DDR_index_2_curr++;
            //     }
            // }
            // lineIndex++;

            original32BitData = cell.empty() ? 0 : std::stol(cell, nullptr, 16);
            // std::cout << "original32BitData: " << std::hex << (original32BitData>>32) << " " << (original32BitData & 0xffffffff) << std::endl;

            if (lineIndex < (NUM_LABEL_KEY+NUM_NUMERICAL_KEY)) {
                DDR_table0[DDR_index_0_curr] = original32BitData;
                DDR_index_0_curr++;
            }
            else if (lineIndex < (NUM_LABEL_KEY+NUM_NUMERICAL_KEY+16)) {
                DDR_table1[DDR_index_1_curr] = original32BitData;
                DDR_index_1_curr++;
            }
            else {
                DDR_table2[DDR_index_2_curr] = original32BitData;
                DDR_index_2_curr++;
            }

            lineIndex++;
        }

        DDR_index_0_pre += 16; // ** change to the next line
        DDR_index_1_pre += 16;  // ** change to the next line
        DDR_index_2_pre += 16;  // ** change to the next line
    }

    
    file.close();  // Close the file
    // auto end_host = std::chrono::high_resolution_clock::now();
    // auto durationUs_host = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_host-start_host).count() / 1000.0);
    // printf("Duration of host operation: %f us\n", durationUs_host);
    auto check_point_2 = std::chrono::high_resolution_clock::now();
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
    // CL_MEM_EXT_PTR_XILINX
//////////////////////////////   TEMPLATE START  //////////////////////////////
    OCL_CHECK(err, cl::Buffer buffer_DDR_table0(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            DDR_table0_size *sizeof(int), &DDR_table0Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_DDR_table1(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            DDR_table1_size *sizeof(int), &DDR_table1Ext, &err));
    OCL_CHECK(err, cl::Buffer buffer_DDR_table2(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX, 
            DDR_table2_size *sizeof(int), &DDR_table2Ext, &err));

// // .......................................................
// // Allocate Global Memory for sourcce_hw_results
// // .......................................................
//     OCL_CHECK(err, cl::Buffer buffer_output(
//         context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY | CL_MEM_EXT_PTR_XILINX, 
//         size_results_out *sizeof(int), &sourcce_hw_resultsExt, &err));

// ============================================================================
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
    // auto start_write_mem = std::chrono::high_resolution_clock::now();
    auto check_point_3 = std::chrono::high_resolution_clock::now();

    OCL_CHECK(
        err, err = q.enqueueMigrateMemObjects({
        buffer_DDR_table0,
        buffer_DDR_table1,
        buffer_DDR_table2
        }, 0/* 0 means from host*/));	
    
    // auto end_write_mem = std::chrono::high_resolution_clock::now();
    // auto durationUs_write_mem = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_write_mem-start_write_mem).count() / 1000.0);
    // printf("Duration of write mem :%f us\n", durationUs_write_mem);
//////////////////////////////   TEMPLATE END  //////////////////////////////
// ----------------------------------------
// Step 2: Submit Kernels for Execution
// ----------------------------------------
    // auto start_task = std::chrono::high_resolution_clock::now();

    OCL_CHECK(err, err = q.enqueueTask(krnl_vector_add));
    // q.finish();

    // auto end_task = std::chrono::high_resolution_clock::now();
    // auto durationUs_task = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_task-start_task).count() / 1000.0);
    // printf("Duration of task :%f us\n", durationUs_task);
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

    // auto end_mem = std::chrono::high_resolution_clock::now();
    // auto durationUs_mem = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_mem-start_mem).count() / 1000.0);
    // printf("Duration of memory to host: %f us\n", durationUs_mem);
    // auto start_write = std::chrono::high_resolution_clock::now();
    auto check_point_4 = std::chrono::high_resolution_clock::now();

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

    auto end_program = std::chrono::high_resolution_clock::now();

    auto durationUs_1 = (std::chrono::duration_cast<std::chrono::nanoseconds>(check_point_1-start_program).count() / 1000.0);
    auto durationUs_2 = (std::chrono::duration_cast<std::chrono::nanoseconds>(check_point_2-check_point_buffer).count() / 1000.0);
    auto durationUs_3 = (std::chrono::duration_cast<std::chrono::nanoseconds>(check_point_3-check_point_2).count() / 1000.0);
    auto durationUs_4 = (std::chrono::duration_cast<std::chrono::nanoseconds>(check_point_4-check_point_3).count() / 1000.0);
    auto durationUs_5 = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_program-check_point_4).count() / 1000.0);
    auto durationUs_program = (std::chrono::duration_cast<std::chrono::nanoseconds>(end_program-start_program).count() / 1000.0);
    printf("Duration of getting line number: %f us\n", durationUs_1);
    printf("Duration of initialize buffer: %f us\n", durationUs_buffer);
    printf("Duration of assigning host buffer: %f us\n", durationUs_2);
    printf("Duration of reading xclbin: %f us\n", durationUs_3);
    printf("Duration of kernel execution: %f us\n", durationUs_4);
    printf("Duration of writing to disk: %f us\n", durationUs_5);
    printf("Duration of the whole program: %f us\n", durationUs_program);

    return 0;
}

