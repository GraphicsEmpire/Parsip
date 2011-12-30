#include "PS_ComputeDevice.h"
#include "PS_ErrorManager.h"
#include <iostream>
#include <fstream>

using namespace PS;

namespace PS{
namespace HPC{

ComputeKernel* ComputeProgram::addKernel(const char *chrKernelTitle)
{
    cl_int err;
    // Create the compute kernel in the program
    cl_kernel k = clCreateKernel(m_clProgram, chrKernelTitle, &err);
    if (!k || err != CL_SUCCESS) {
        ReportError("Error: Failed to create compute kernel!");
        return NULL;
    }

    ComputeKernel* kernel = new ComputeKernel(k, chrKernelTitle);
    m_lstKernels.push_back(kernel);
    return kernel;
}

ComputeProgram::~ComputeProgram()
{
    for(size_t i=0; i<m_lstKernels.size(); i++)
    {
        SAFE_DELETE(m_lstKernels[i]);
    }
    m_lstKernels.resize(0);
    clReleaseProgram(this->m_clProgram);
}

ComputeDevice::ComputeDevice(DEVICETYPE dev)
{
    m_bReady = false;

    m_deviceType = dev;

    // Connect to a compute device
    cl_int err = clGetPlatformIDs(1, &m_clPlatform, NULL);
    if (err != CL_SUCCESS)
    {
        ReportError("Error: Failed to find a platform!");
        FlushAllErrors();
        return;
    }

    // Get a device of the appropriate type
    err = clGetDeviceIDs(m_clPlatform, dev, 1, &m_clDeviceID, NULL);
    if (err != CL_SUCCESS) {
        ReportError("Error: Failed to create a device group!");
        FlushAllErrors();
        return;
    }

    // Create a compute context
    m_clContext = clCreateContext(0, 1, &m_clDeviceID, NULL, NULL, &err);
    if (!m_clContext) {
        ReportError("Error: Failed to create a compute context!");
        FlushAllErrors();
        return;
    }

    // Create a command commands
    m_clCommandQueue = clCreateCommandQueue(m_clContext, m_clDeviceID, 0, &err);
    if (!m_clCommandQueue) {
        ReportError("Error: Failed to create a command commands!");
        FlushAllErrors();
        return;
    }

    m_bReady = true;
}

//Read Program from file and then compile it
ComputeProgram* ComputeDevice::addProgramFromFile(const char *chrFilePath)
{
    if(!m_bReady) return NULL;
    std::ifstream fp;
    fp.open(chrFilePath, std::ios::binary);
    if(!fp.is_open())
        return false;

    size_t size;
    fp.seekg(0, std::ios::end);
    size = fp.tellg();
    fp.seekg(0, std::ios::beg);

    char * buf = new char[size+1];
    //Read file content
    fp.read(buf, size);
    buf[size] = '\0';

    string strCode = string(buf);
    SAFE_DELETE(buf);
    fp.close();

    return addProgram(strCode.c_str());
}

ComputeProgram* ComputeDevice::addProgram(const char* chrComputeCode)
{
    if(!m_bReady) return NULL;

    cl_int err = 0;

    // Create the compute program from the source buffer
    cl_program program = clCreateProgramWithSource(m_clContext, 1,
                                                  (const char **) &chrComputeCode,
                                                  NULL, &err);
    if (!program) {
        ReportError("Error: Failed to create compute program!");
        FlushAllErrors();
        return NULL;
    }

    // Build the program executable
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(program, m_clDeviceID, CL_PROGRAM_BUILD_LOG,
                              sizeof(buffer), buffer, &len);
        cerr << buffer << endl;

        ReportError("Error: Failed to build program executable!");
        ReportError(buffer);
        FlushAllErrors();
        return NULL;
    }

    ComputeProgram* compute = new ComputeProgram(program);
    m_lstPrograms.push_back(compute);
    return compute;
}


}
}
