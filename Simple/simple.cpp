//
//  main.cpp
//  Simple
//
//  Created by Beau Johnston on 25/07/11.
//  Copyright 2011 University Of New England. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "openCLUtilities.h"


// If more than one platform installed then set this to pick which
// one to use
#define PLATFORM_INDEX 0
#define NUM_BUFFER_ELEMENTS 10

// main() for simple buffer and sub-buffer example
//
int main(int argc, char** argv)
{
    cl_int errNum;
    cl_uint numPlatforms;
    cl_uint numDevices;
    cl_platform_id * platformIDs;
    cl_device_id * deviceIDs;
    cl_context context;
    cl_program program;
    std::vector<cl_kernel> kernels;
    std::vector<cl_command_queue> queues;
    std::vector<cl_mem> buffers;
    int * inputOutput;
    std::cout << "Simple buffer and sub-buffer Example" << std::endl;
    // First, select an OpenCL platform to run on.
    errNum = clGetPlatformIDs(0, NULL, &numPlatforms);
    checkErr(
             (errNum != CL_SUCCESS) ?
             errNum : (numPlatforms <= 0 ? -1 : CL_SUCCESS),
             "clGetPlatformIDs");
    platformIDs = (cl_platform_id *)alloca(sizeof(cl_platform_id) * numPlatforms);
    std::cout << "Number of platforms: \t" << numPlatforms << std::endl;
    errNum = clGetPlatformIDs(numPlatforms, platformIDs, NULL);
    checkErr(
             (errNum != CL_SUCCESS) ?
             errNum : (numPlatforms <= 0 ? -1 : CL_SUCCESS),
             "clGetPlatformIDs");
    std::ifstream srcFile("simple.cl");
    
    checkErr(srcFile.is_open() ? CL_SUCCESS : -1, "reading simple.cl");
    
    std::string srcProg(
                        std::istreambuf_iterator<char>(srcFile),
                        (std::istreambuf_iterator<char>()));
    const char * src = srcProg.c_str();
    size_t length = srcProg.length();
    deviceIDs = NULL;
    DisplayPlatformInfo(
                        platformIDs[PLATFORM_INDEX],
                        CL_PLATFORM_VENDOR,
                        "CL_PLATFORM_VENDOR");
    errNum = clGetDeviceIDs(
                            platformIDs[PLATFORM_INDEX],
                            CL_DEVICE_TYPE_ALL,
                            0,
                            NULL,
                            &numDevices);
    if (errNum != CL_SUCCESS && errNum != CL_DEVICE_NOT_FOUND){
        checkErr(errNum, "clGetDeviceIDs");
    }
    
    deviceIDs = (cl_device_id *)alloca(
                                       sizeof(cl_device_id) * numDevices);
    errNum = clGetDeviceIDs(
                            platformIDs[PLATFORM_INDEX],
                            CL_DEVICE_TYPE_ALL,
                            numDevices,
                            &deviceIDs[0],
                            NULL);
    checkErr(errNum, "clGetDeviceIDs");
    
    cl_context_properties contextProperties[] =
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platformIDs[PLATFORM_INDEX],
        0
    };
    
    context = clCreateContext(
                              contextProperties,
                              numDevices,
                              deviceIDs,
                              NULL,
                              NULL,
                              &errNum);
    
    checkErr(errNum, "clCreateContext");
    // Create program from source
    program = clCreateProgramWithSource(
                                        context,
                                        1,
                                        &src,
                                        &length,
                                        &errNum);
    checkErr(errNum, "clCreateProgramWithSource");
    
    // Build program
    errNum = clBuildProgram(
                            program,
                            numDevices,
                            deviceIDs,
                            "-I.",
                            NULL,
                            NULL);

    if (errNum != CL_SUCCESS){
        // Determine the reason for the error
        char buildLog[16384];
        clGetProgramBuildInfo(
                              program,
                              deviceIDs[0],
                              CL_PROGRAM_BUILD_LOG,
                              sizeof(buildLog),
                              buildLog,
                              NULL);
        std::cerr << "Error in OpenCL C source: " << std::endl;
        std::cerr << buildLog;
        checkErr(errNum, "clBuildProgram");
    }
        // create buffers and sub-buffers
        inputOutput = new int[NUM_BUFFER_ELEMENTS * numDevices];
        for (unsigned int i = 0; i < NUM_BUFFER_ELEMENTS * numDevices; i++)
        {
            inputOutput[i] = i;
        }
        
        // create a single buffer to cover all the input data
        cl_mem buffer = clCreateBuffer(
                                       context,
                                       CL_MEM_READ_WRITE,
                                       sizeof(int) * NUM_BUFFER_ELEMENTS * numDevices,
                                       NULL,
                                       &errNum);
        checkErr(errNum, "clCreateBuffer");
        buffers.push_back(buffer);
        // now for all devices other than the first create a sub-buffer
        for (unsigned int i = 1; i < numDevices; i++)
        {
            cl_buffer_region region =
            {
                NUM_BUFFER_ELEMENTS * i * sizeof(int),
                NUM_BUFFER_ELEMENTS * sizeof(int)
            };
            buffer = clCreateSubBuffer(
                                       buffers[0],
                                       CL_MEM_READ_WRITE,
                                       CL_BUFFER_CREATE_TYPE_REGION,
                                       &region,
                                       &errNum);
            checkErr(errNum, "clCreateSubBuffer");
            buffers.push_back(buffer);
        }
        // Create command queues
        for (int i = 0; i < numDevices; i++)
        {
            InfoDevice<cl_device_type>::display(deviceIDs[i], CL_DEVICE_TYPE, "CL_DEVICE_TYPE");
            cl_command_queue queue =
            clCreateCommandQueue(
                                 context,
                                 deviceIDs[i],
                                 0,
                                 &errNum);
            checkErr(errNum, "clCreateCommandQueue");
            queues.push_back(queue);
            cl_kernel kernel = clCreateKernel(
                                              program,
                                              "square",
                                              &errNum);
            checkErr(errNum, "clCreateKernel(square)");
            errNum = clSetKernelArg(
                                    kernel,
                                    0,
                                    sizeof(cl_mem), (void *)&buffers[i]);
            checkErr(errNum, "clSetKernelArg(square)");
            kernels.push_back(kernel);
            // Write input data
            clEnqueueWriteBuffer(
                                 queues[0],
                                 buffers[0],
                                 CL_TRUE,
                                 0,
                                 sizeof(int) * NUM_BUFFER_ELEMENTS * numDevices,
                                 (void*)inputOutput,
                                 0,
                                 NULL,
                                 NULL);
            std::vector<cl_event> events;
            // call kernel for each device
            for (int i = 0; i < queues.size(); i++)
            {
                cl_event event;
                size_t gWI = NUM_BUFFER_ELEMENTS;
                errNum = clEnqueueNDRangeKernel(
                                                queues[i],
                                                kernels[i],
                                                1,
                                                NULL,
                                                (const size_t*)&gWI,
                                                (const size_t*)NULL,
                                                0,
                                                0,
                                                &event);
                events.push_back(event);
            }
            // Technically don't need this as we are doing a blocking read
            // with in-order queue.
            clWaitForEvents(events.size(), events.data());
            // Read back computed data
            clEnqueueReadBuffer(
                                queues[0],
                                buffers[0],
                                CL_TRUE,
                                0,
                                sizeof(int) * NUM_BUFFER_ELEMENTS * numDevices,
                                (void*)inputOutput,
                                0,
                                NULL,
                                NULL);
            // Display output in rows
            for (unsigned i = 0; i < numDevices; i++)
            {
                for (unsigned elems = i * NUM_BUFFER_ELEMENTS;
                     elems < ((i+1) * NUM_BUFFER_ELEMENTS);
                     elems++)
                {
                    std::cout << " " << inputOutput[elems];
                }
                std::cout << std::endl;
            }
            std::cout << "Program completed successfully" << std::endl;
            return 0; 
        }
}
            