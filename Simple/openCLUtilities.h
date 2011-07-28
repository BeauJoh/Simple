//
//  openCLUtilities.h
//  Simple
//
//  Created by Beau Johnston on 25/07/11.
//  Copyright 2011 University Of New England. All rights reserved.
//

#ifndef Simple_openCLUtilities_h
#define Simple_openCLUtilities_h

#if defined (__APPLE__)  && defined (__MACH__)
//Apple openCL build
#include <OpenCL/opencl.h>
#else
//Nix openCL build
#include <CL/opencl.h>
#endif

#include <sys/stat.h>


#define FATAL(msg)\
do {\
fprintf(stderr,"FATAL [%s:%d]:%s:%s\n", __FILE__, __LINE__, msg, strerror(errno)); \
assert(0); \
} while(0)

#define SRC 1
#define DST 2


char *print_cl_errstring(cl_int err);
cl_bool there_was_an_error(cl_int err);
void getGPUUnitSupportedImageFormats(cl_context context);
cl_bool doesGPUSupportImageObjects(cl_device_id device_id);
char *load_program_source(const char *filename);
cl_bool cleanupAndKill();


void checkErr(cl_int err, const char * name);

void DisplayPlatformInfo(cl_platform_id id,
                         cl_platform_info name,
                         std::string str);

template<typename T>
void appendBitfield(
                    T info, T value, std::string name, std::string & str)
{
    if (info & value)
    {
        if (str.length() > 0)
        {
            str.append(" | ");
        }
        str.append(name);
    }
}

template <typename T>
class InfoDevice
{
public:
    static void display(
                        cl_device_id id, cl_device_info name, std::string str)
    {
        cl_int errNum;
        std::size_t paramValueSize;
        errNum = clGetDeviceInfo(id, name, 0, NULL, &paramValueSize);
        if (errNum != CL_SUCCESS)
        {
            std::cerr << "Failed to find OpenCL device info "
            << str << "." << std::endl;
            return;
        }
        T * info = (T *)alloca(sizeof(T) * paramValueSize);
        errNum = clGetDeviceInfo(id,name,paramValueSize,info,NULL);
        if (errNum != CL_SUCCESS)
        {
            std::cerr << "Failed to find OpenCL device info "
            << str << "." << std::endl;
            return; }
        switch (name)
        {
            case CL_DEVICE_TYPE:
            {
                std::string deviceType;
                appendBitfield<cl_device_type>(
                                               *(reinterpret_cast<cl_device_type*>(info)),
                                               CL_DEVICE_TYPE_CPU, "CL_DEVICE_TYPE_CPU", deviceType);
                appendBitfield<cl_device_type>(
                                               *(reinterpret_cast<cl_device_type*>(info)),
                                               CL_DEVICE_TYPE_GPU, "CL_DEVICE_TYPE_GPU", deviceType);
                appendBitfield<cl_device_type>(
                                               *(reinterpret_cast<cl_device_type*>(info)),
                                               CL_DEVICE_TYPE_ACCELERATOR,
                                               "CL_DEVICE_TYPE_ACCELERATOR",
                                               
                                               deviceType);
                appendBitfield<cl_device_type>(
                                               *(reinterpret_cast<cl_device_type*>(info)),
                                               CL_DEVICE_TYPE_DEFAULT,
                                               "CL_DEVICE_TYPE_DEFAULT",
                                               deviceType);
                std::cout << "\t\t" << str << ":\t"
                << deviceType << std::endl;
            }
                break;
            case CL_DEVICE_SINGLE_FP_CONFIG:
            {
                std::string fpType;
                appendBitfield<cl_device_fp_config>(
                                                    *(reinterpret_cast<cl_device_fp_config*>(info)),
                                                    CL_FP_DENORM, "CL_FP_DENORM", fpType);
                appendBitfield<cl_device_fp_config>(
                                                    *(reinterpret_cast<cl_device_fp_config*>(info)),
                                                    CL_FP_INF_NAN, "CL_FP_INF_NAN", fpType);
                appendBitfield<cl_device_fp_config>(
                                                    *(reinterpret_cast<cl_device_fp_config*>(info)),
                                                    CL_FP_ROUND_TO_NEAREST, "CL_FP_ROUND_TO_NEAREST", fpType);
                appendBitfield<cl_device_fp_config>(
                                                    *(reinterpret_cast<cl_device_fp_config*>(info)),
                                                    CL_FP_ROUND_TO_ZERO, "CL_FP_ROUND_TO_ZERO", fpType);
                appendBitfield<cl_device_fp_config>(
                                                    *(reinterpret_cast<cl_device_fp_config*>(info)),
                                                    CL_FP_ROUND_TO_INF, "CL_FP_ROUND_TO_INF", fpType);
                appendBitfield<cl_device_fp_config>(
                                                    *(reinterpret_cast<cl_device_fp_config*>(info)),
                                                    CL_FP_FMA, "CL_FP_FMA", fpType);
                appendBitfield<cl_device_fp_config>(
                                                    *(reinterpret_cast<cl_device_fp_config*>(info)),
                                                    CL_FP_SOFT_FLOAT, "CL_FP_SOFT_FLOAT", fpType);
                std::cout << "\t\t" << str << ":\t" << fpType << std::endl;
            }
                break;
            case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
            {
                std::string memType;
                appendBitfield<cl_device_mem_cache_type>(
                                                         *(reinterpret_cast<cl_device_mem_cache_type*>(info)),
                                                         CL_NONE, "CL_NONE", memType);
                appendBitfield<cl_device_mem_cache_type>(
                                                         *(reinterpret_cast<cl_device_mem_cache_type*>(info)),
                                                         CL_READ_ONLY_CACHE, "CL_READ_ONLY_CACHE", memType);
                appendBitfield<cl_device_mem_cache_type>(
                                                         *(reinterpret_cast<cl_device_mem_cache_type*>(info)),
                                                         CL_READ_WRITE_CACHE, "CL_READ_WRITE_CACHE", memType);
                std::cout << "\t\t" << str << ":\t" << memType << std::endl;
            }
                break;
            case CL_DEVICE_LOCAL_MEM_TYPE:
            {
                std::string memType;
                appendBitfield<cl_device_local_mem_type>(
                                                         *(reinterpret_cast<cl_device_local_mem_type*>(info)),
                                                         CL_GLOBAL, "CL_LOCAL", memType);
                appendBitfield<cl_device_local_mem_type>(
                                                         *(reinterpret_cast<cl_device_local_mem_type*>(info)),
                                                         CL_GLOBAL, "CL_GLOBAL", memType);
                std::cout << "\t\t" << str << ":\t" << memType << std::endl;
            }
                break;
            case CL_DEVICE_EXECUTION_CAPABILITIES:
            {
                std::string memType;
                appendBitfield<cl_device_exec_capabilities>(
                                                            *(reinterpret_cast<cl_device_exec_capabilities*>(info)),
                                                            CL_EXEC_KERNEL, "CL_EXEC_KERNEL", memType);
                appendBitfield<cl_device_exec_capabilities>(
                                                            *(reinterpret_cast<cl_device_exec_capabilities*>(info)),
                                                            CL_EXEC_NATIVE_KERNEL, "CL_EXEC_NATIVE_KERNEL", memType);
                std::cout << "\t\t" << str << ":\t" << memType << std::endl;
            }
                break;
            case CL_DEVICE_QUEUE_PROPERTIES:
            {
                std::string memType;
                appendBitfield<cl_device_exec_capabilities>(
                                                            *(reinterpret_cast<cl_device_exec_capabilities*>(info)),
                                                            CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
                                                            "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE", memType);
                appendBitfield<cl_device_exec_capabilities>(
                                                            *(reinterpret_cast<cl_device_exec_capabilities*>(info)),
                                                            CL_QUEUE_PROFILING_ENABLE, "CL_QUEUE_PROFILING_ENABLE",
                                                            memType);
                std::cout << "\t\t" << str << ":\t" << memType << std::endl;
            }
                break;
            default:
                std::cout << "\t\t" << str << ":\t" << *info << std::endl;
            break; }
    } 
};
#endif
