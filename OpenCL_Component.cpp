#include "OpenCL_Component.h"

char* OpenCL_Component::load_kernel(const char* path){
    // Turn kernel file content into a string
    FILE *fp;
    char *source_str;
    size_t program_size;
    
    fp = fopen(path, "rb");
    if (!fp) {
        printf("Failed to load kernel\n");
        return NULL;
    }
    
    fseek(fp, 0, SEEK_END);
    program_size = ftell(fp);
    rewind(fp);
    source_str = (char*)malloc(program_size + 1);
    source_str[program_size] = '\0';
    fread(source_str, sizeof(char), program_size, fp);
    fclose(fp);
    
    return source_str;
    }
    
void OpenCL_Component::cleanup(){
    // Frees up allocated space
    if(kernel_source_code)free(kernel_source_code);
    if(ckKernel)clReleaseKernel(ckKernel);  
    if(cpProgram)clReleaseProgram(cpProgram);
    if(cqCommandQueue)clReleaseCommandQueue(cqCommandQueue);
    if(cxGPUContext)clReleaseContext(cxGPUContext);
    if(dev_gradients)clReleaseMemObject(dev_gradients);
    if(dev_heightmap)clReleaseMemObject(dev_heightmap);
    if(host_gradients)free(host_gradients);
    if(host_heightmap)free(host_heightmap);
    }
    
int OpenCL_Component::verify(int status_code, int index){
    // Assigns error message if an OpenCL error occurs
    
    static const char* error_msg[] = {
        "Unable to obtain PlatformID",
        "Unable to create OpenCL Context",
        "Unable to create Command Queue",
        "Unable to create Buffer on the device",
        "Unable to create Program from kernel",
        "Unable to build kernel program",
        "Unable to create kernel object",
        "Failed to set kernel argument",
        "Failed to enqueue write instruction",
        "Kernel error",
        "Failed to read result"
        };
    
    if (status_code != CL_SUCCESS){
        std::cout << error_msg[index] << std::endl;
        std::cout << "OpenCL Error " << status_code <<std::endl;
        cleanup();
        return status_code;
        }
    return 0;
    }
 
void OpenCL_Component::generate_gradients(cl_float2* lattice, int size){
    // C++ seed
    //std::default_random_engine eng{static_cast<long unsigned int>(time(0))};
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    for (int i = 0; i < size; i++){
        lattice[i].s[0] = dis(eng);
        lattice[i].s[1] = dis(eng);
        }
    }
    
OpenCL_Component::OpenCL_Component(){}

int OpenCL_Component::OpenCL_init(){
    
    // Determine OpenCL platform
    status = clGetPlatformIDs(1, &cpPlatform, NULL);
    if (verify(status, 0)) return status;
    
    // Get OpenCL Devices
    status = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL); 
    
    // Create OpenCL Context
    cxGPUContext = clCreateContext(0, 1, &cdDevice, NULL, NULL, &status);
    if (verify(status, 1)) return status;
    
    // Create Command Queue
    cqCommandQueue = clCreateCommandQueue(cxGPUContext, cdDevice, 0, &status);
    if (verify(status, 2)) return status;
    
    // TODO: Check if the below code has to be used multiple times, if so move it to another method (loadKernel())
    // Load Kernel from source_str
    
    kernel_path = "./perlin_kernel.cl";
    kernel_name = "PerlinNoise";
    
    char* kernel_source_code = load_kernel(kernel_path);
    
    szKernelLength = strlen(kernel_source_code);
    cpProgram = clCreateProgramWithSource(cxGPUContext, 1, (const char **)&kernel_source_code, &szKernelLength, &status);
    
    if (verify(status, 4)) return status;
    
    // Create OpenCL program
    status = clBuildProgram(cpProgram, 0, NULL, NULL, NULL, NULL);
    
    if (status == CL_BUILD_PROGRAM_FAILURE){
        size_t log_size;
        clGetProgramBuildInfo(cpProgram, cdDevice, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        
        char *log = (char *) malloc(log_size);
        clGetProgramBuildInfo(cpProgram, cdDevice, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("%s\n", log);
        }
    if (verify(status, 5)) return status;
    
    // Create the kernel
    ckKernel = clCreateKernel(cpProgram, kernel_name, &status);
    if (verify(status, 6)) return status;
    
    return 0;
    }
  
int OpenCL_Component::execute(int gradWidth, int gradHeight, int hmapWidth, int hmapHeight){
    
    // GlobalWorkSize = threads_per_group * group_num;
    LocalWorkSize = 256;
    GlobalWorkSize = LocalWorkSize*4;   // Testing with 4 octaves, should be parameter
    
    cl_int gradSize = gradHeight * gradWidth;
    cl_int hmapSize = hmapHeight * hmapWidth;
   
    // Array initialization
    host_gradients = (cl_float2*)calloc(gradSize, sizeof(cl_float2));
    host_heightmap = (cl_float*)calloc(hmapSize, sizeof(cl_float));
    result = (cl_float*)malloc(sizeof(cl_float) * hmapSize);
    
    // Populate gradients with random 2D vectors
    generate_gradients(host_gradients, gradSize);
    
    // Allocate GPU buffers with clCreateBuffer 
    dev_gradients = clCreateBuffer(cxGPUContext, CL_MEM_READ_ONLY, sizeof(cl_float2) * gradSize, NULL, &status);
    if (verify(status, 3)) return status;
    
    dev_heightmap = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, sizeof(cl_float) * hmapSize, NULL, &status);
    if (verify(status, 3)) return status;
    
    // Set arguments for kernel 
    status = clSetKernelArg(ckKernel, 0, sizeof(cl_mem), (void*)&dev_gradients);
    if (verify(status, 7)) return status;
    status = clSetKernelArg(ckKernel, 1, sizeof(cl_mem), (void*)&dev_heightmap);
    if (verify(status, 7)) return status;
    
    // TODO: Might be better to pass height and width as float2
    status = clSetKernelArg(ckKernel, 2, sizeof(cl_int), &gradWidth);
    if (verify(status, 7)) return status;
    status = clSetKernelArg(ckKernel, 3, sizeof(cl_int), &gradHeight);
    if (verify(status, 7)) return status;
    status = clSetKernelArg(ckKernel, 4, sizeof(cl_int), &hmapWidth);
    if (verify(status, 7)) return status;
    status = clSetKernelArg(ckKernel, 5, sizeof(cl_int), &hmapHeight);
    if (verify(status, 7)) return status;
    
    // Write data to GPU Buffers
    status = clEnqueueWriteBuffer(cqCommandQueue, dev_gradients, CL_FALSE, 0, sizeof(cl_float2) * gradSize, host_gradients, 0, NULL, NULL);
    if (verify(status, 8)) return status;
    
    status = clEnqueueWriteBuffer(cqCommandQueue, dev_heightmap, CL_FALSE, 0, sizeof(cl_float) * hmapSize, host_heightmap, 0, NULL, NULL);
    if (verify(status, 8)) return status;
    
    // Execute the kernel for each octave, use the same buffer to add each octave result
    for (int octaveID = 0; octaveID < 4; octaveID++){
        // Set the octave value
        status = clSetKernelArg(ckKernel, 6, sizeof(cl_int), &octaveID);
        if (verify(status, 7)) return status;
        
        // Compile Octave
        status = clEnqueueNDRangeKernel(cqCommandQueue, ckKernel, 1, NULL, &GlobalWorkSize, &LocalWorkSize, 0, NULL, NULL);
        if (verify(status, 9)) return status;
        }
    // Read result
    status = clEnqueueReadBuffer(cqCommandQueue, dev_heightmap, CL_TRUE, 0, sizeof(cl_float) * hmapSize, result, 0, NULL, NULL);
        
    return 0;
    }


int OpenCL_Component::executeN(int n, int gradWidth, int gradHeight, int hmapWidth, int hmapHeight){
    
    // GlobalWorkSize = threads_per_group * group_num;
    LocalWorkSize = 256;
    GlobalWorkSize = LocalWorkSize*4;   // Testing with 4 octaves, should be parameter
    
    cl_int gradSize = gradHeight * gradWidth;
    cl_int hmapSize = hmapHeight * hmapWidth;
   
    // Allocate GPU buffers with clCreateBuffer 
    dev_gradients = clCreateBuffer(cxGPUContext, CL_MEM_READ_ONLY, sizeof(cl_float2) * gradSize, NULL, &status);
    if (verify(status, 3)) return status;
    
    dev_heightmap = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, sizeof(cl_float) * hmapSize, NULL, &status);
    if (verify(status, 3)) return status;
    
    // Set arguments for kernel 
    status = clSetKernelArg(ckKernel, 0, sizeof(cl_mem), (void*)&dev_gradients);
    if (verify(status, 7)) return status;
    status = clSetKernelArg(ckKernel, 1, sizeof(cl_mem), (void*)&dev_heightmap);
    if (verify(status, 7)) return status;
    
    // TODO: Might be better to pass height and width as float2
    status = clSetKernelArg(ckKernel, 2, sizeof(cl_int), &gradWidth);
    if (verify(status, 7)) return status;
    status = clSetKernelArg(ckKernel, 3, sizeof(cl_int), &gradHeight);
    if (verify(status, 7)) return status;
    status = clSetKernelArg(ckKernel, 4, sizeof(cl_int), &hmapWidth);
    if (verify(status, 7)) return status;
    status = clSetKernelArg(ckKernel, 5, sizeof(cl_int), &hmapHeight);
    if (verify(status, 7)) return status;
    
    // Allocate host buffers
    host_gradients = (cl_float2*)calloc(gradSize, sizeof(cl_float2));
     
    resultN = (cl_float**)malloc(sizeof(cl_float*) * n);
    
    for (int frame = 0; frame < n; frame++){
        
        // Populate gradient buffer with random 2D vectors
        generate_gradients(host_gradients, gradSize);
        
        // We allocate a new heightmap containing the current frame (starts everything at 0)
        cl_float* host_frame = (cl_float*)calloc(hmapSize, sizeof(cl_float));
    
        // Write data to GPU Buffers
        status = clEnqueueWriteBuffer(cqCommandQueue, dev_gradients, CL_FALSE, 0, sizeof(cl_float2) * gradSize, host_gradients, 0, NULL, NULL);
        if (verify(status, 8)) return status;
        
        status = clEnqueueWriteBuffer(cqCommandQueue, dev_heightmap, CL_FALSE, 0, sizeof(cl_float) * hmapSize, host_frame, 0, NULL, NULL);
        if (verify(status, 8)) return status;
    
        // Execute the kernel for each octave, use the same buffer to add each octave result
        for (int octaveID = 0; octaveID < 4; octaveID++){
            // Set the octave value
            status = clSetKernelArg(ckKernel, 6, sizeof(cl_int), &octaveID);
            if (verify(status, 7)) return status;
            
            // Compile Octave
            status = clEnqueueNDRangeKernel(cqCommandQueue, ckKernel, 1, NULL, &GlobalWorkSize, &LocalWorkSize, 0, NULL, NULL);
            if (verify(status, 9)) return status;
            }
        
        // Read result into the frame buffer
        status = clEnqueueReadBuffer(cqCommandQueue, dev_heightmap, CL_TRUE, 0, sizeof(cl_float) * hmapSize, host_frame, 0, NULL, NULL);
        if (verify(status, 10)) return status;
        
        // Save pointer to new array
        resultN[frame] = host_frame;
        }
    return 0;
    }


void* OpenCL_Component::getResult(){
    return result;
    }
   
void* OpenCL_Component::getNResult(){   //Refactor this spaget
    return (void*)resultN;
    }    
    
OpenCL_Component::~OpenCL_Component(){
    cleanup();
    }