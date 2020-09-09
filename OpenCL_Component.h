#define CL_ENABLE_EXCEPTIONS
#define CL_HAS_NAMED_VECTOR_FIELDS

#pragma once

#include <CL/cl.h>
#include <cassert>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <iostream>
#include <random>
#include <time.h>

class OpenCL_Component{
public:
    OpenCL_Component();
    ~OpenCL_Component();
    int OpenCL_init();
    int execute(int gradWidth, int gradHeight, int hmapWidth, int hmapHeight);
    int executeN(int n, int gradWidth, int gradHeight, int hmapWidth, int hmapHeight);
    void* getResult();
    void* getNResult();
    
private:
    // OpenCL constants
    cl_context cxGPUContext;        // OpenCL context
    cl_command_queue cqCommandQueue;// OpenCL command queue
    cl_platform_id cpPlatform;      // OpenCL platform
    cl_device_id cdDevice;          // OpenCL device
    cl_program cpProgram;           // OpenCL program
    cl_kernel ckKernel;             // OpenCL kernel
    size_t LocalWorkSize;
    size_t GlobalWorkSize;          
    size_t szKernelLength;
    cl_int status;                  // OpenCL Error codes
    char* kernel_source_code;
    const char* kernel_path;
    const char* kernel_name;
    
    // Other data
    int gradientSize;
    int heightmapSize;
    
    // C++ Seed
    std::default_random_engine eng{static_cast<long unsigned int>(time(0))};
    
    // Host Buffers
    cl_float2* host_gradients;
    cl_float*  host_heightmap;
    cl_float** host_frames;
    cl_float*  result;
    cl_float** resultN;
    
    // Device buffers
    cl_mem dev_gradients;
    cl_mem dev_heightmap;
    
    // Methods
    void cleanup();
    char* load_kernel(const char* path);
    int verify(int status_code, int index);
    
    void generate_gradients(cl_float2* lattice, int size);
    };