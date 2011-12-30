#ifndef PS_COMPUTEDEVICE_H
#define PS_COMPUTEDEVICE_H
#include <vector>
#include <string.h>

#include <CL/cl.h>

using namespace std;

namespace PS{
namespace HPC{

/*!
  * A Kernel is a simple function to run on device
  */
class ComputeKernel
{
public:
    ComputeKernel(cl_kernel kernel, const std::string& strTitle)
    {
        m_clKernel = kernel;
        m_strTitle = strTitle;
    }

    ~ComputeKernel() {
        clReleaseKernel(m_clKernel);
    }

    cl_kernel getKernel() const {return m_clKernel;}
    std::string getTitle() const {return m_strTitle;}
private:
   cl_kernel m_clKernel;
   std::string m_strTitle;
};

/*!
  * A program may contain multiple kernels.
  */
class ComputeProgram
{
public:
    ComputeProgram(cl_program program)
    {
        m_clProgram = program;
    }
    ~ComputeProgram();

    ComputeKernel* addKernel(const char* chrKernelTitle);
private:
    cl_program m_clProgram;
    std::vector<ComputeKernel*> m_lstKernels;
};

/*!
  * A compute device holds multiple programs
  */
class ComputeDevice{
public:
    enum DEVICETYPE{dtCPU = CL_DEVICE_TYPE_CPU, dtGPU = CL_DEVICE_TYPE_GPU};

    ComputeDevice() { m_bReady = false;}
    ComputeDevice(DEVICETYPE dev);
    ~ComputeDevice();

    bool isReady() const {return m_bReady;}

    ComputeProgram* addProgram(const char* chrComputeCode);
    ComputeProgram* addProgramFromFile(const char* chrFilePath);

    //Access
    cl_device_id getDevice() const {return m_clDeviceID;}
    cl_context getContext() const {return m_clContext;}
    cl_command_queue getCommandQ() const {return m_clCommandQueue;}
private:
    DEVICETYPE      m_deviceType;
    cl_platform_id  m_clPlatform;
    cl_device_id    m_clDeviceID;

    cl_context      m_clContext;
    cl_command_queue m_clCommandQueue;

    std::vector<ComputeProgram*> m_lstPrograms;

    bool m_bReady;
};


}
}

#endif // PS_COMPUTEDEVICE_H
