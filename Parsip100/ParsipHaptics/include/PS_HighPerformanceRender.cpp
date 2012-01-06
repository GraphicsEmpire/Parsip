#include "PS_HighPerformanceRender.h"
#include "PS_ErrorManager.h"
#include "BlobTreeLibraryAll.h"
#include "PS_SimdPoly/include/PS_Polygonizer.h"

#define __NO_STD_VECTOR // Use cl::vector and cl::string and
#define __NO_STD_STRING // not STL versions, more on this later
#include <CL/cl.h>

#define PS_ERROR_PRIM_OVERFLOW -1
#define PS_ERROR_OPERATOR_OVERFLOW -2
#define PS_ERROR_NON_BINARY_OP -3


using namespace PS;
using namespace PS::SIMDPOLY;

#define DATA_SIZE (1024*1240)

const char *KernelSource = "\n"		      \
        "__kernel void square(                    \n" \
        "   __global float* input,                \n" \
        "   __global float* output,               \n" \
        "   const unsigned int count)             \n" \
        "{                                        \n" \
        "   int i = get_global_id(0);             \n" \
        "   if(i < count)                         \n" \
        "       output[i] = input[i] * input[i];  \n" \
        "}                                        \n" \
        "\n";


void SimdPoly::reset()
{
    m_blobPrims.ctPrims = 0;
    m_blobOps.ctOps = 0;
    m_blobPrimMatrices.count = 0;
    m_blobBoxMatrices.count = 0;
    m_polyMPUs.ctMPUs = 0;
}

int SimdPoly::linearizeBlobTree(CBlobNode* root, int parentID, int& outIsOperator)
{
    int curID = -1;
    if(parentID == -1)
    {
        vec3f lo = root->getOctree().lower;
        vec3f hi = root->getOctree().upper;
        m_blobPrims.bboxLo = svec3f(lo.x, lo.y, lo.z);
        m_blobPrims.bboxHi = svec3f(hi.x, hi.y, hi.z);

        //Set identity matrix
        float identity[] = {1.0f, 0.0f, 0.0f, 0.0f,
                            0.0f, 1.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f};

        m_blobPrimMatrices.count = 1;
        m_blobBoxMatrices.count = 1;
        for(U32 i=0; i<PRIM_MATRIX_STRIDE; i++)
            m_blobPrimMatrices.matrix[i] = identity[i];
        for(U32 i=0; i<BOX_MATRIX_STRIDE; i++)
            m_blobBoxMatrices.matrix[i] = identity[i];
    }

    //Operator
    outIsOperator = root->isOperator();
    if(outIsOperator)
    {
        if(m_blobOps.ctOps >= MAX_TREE_NODES)
        {
            ReportError("Exceeded maximum number of allowed Operators");
            FlushAllErrors();
            return PS_ERROR_OPERATOR_OVERFLOW;
        }

        curID = m_blobOps.ctOps;
        m_blobOps.ctOps++;
        m_blobOps.opType[curID] = root->getNodeType();

        if(root->countChildren() != 2)
        {
            ReportError("Not a binary Operator!");
            FlushAllErrors();
            return PS_ERROR_NON_BINARY_OP;
        }

        int isOp[2];
        int kidID[2];
        kidID[0] = this->linearizeBlobTree(root->getChild(0), curID, isOp[0]);
        kidID[1] = this->linearizeBlobTree(root->getChild(1), curID, isOp[1]);
        m_blobOps.opLeftChild[curID]  = kidID[0];
        m_blobOps.opRightChild[curID] = kidID[1];
        m_blobOps.opChildKind[curID]  = isOp[0]*2 + isOp[1];

        vec3f lo = root->getOctree().lower;
        vec3f hi = root->getOctree().upper;
        //ReadBox from BlobTree
        m_blobOps.vBoxLoX[curID] = lo.x;
        m_blobOps.vBoxLoY[curID] = lo.y;
        m_blobOps.vBoxLoZ[curID] = lo.z;

        m_blobOps.vBoxHiX[curID] = hi.x;
        m_blobOps.vBoxHiY[curID] = hi.y;
        m_blobOps.vBoxHiZ[curID] = hi.z;


        switch(root->getNodeType())
        {
        case(bntOpPCM):
        {
            CPcm* lpPCM = reinterpret_cast<CPcm*>(root);
            m_blobOps.resX[curID] = lpPCM->getPropagateLeft();
            m_blobOps.resY[curID] = lpPCM->getPropagateRight();
            m_blobOps.resZ[curID] = lpPCM->getAlphaLeft();
            m_blobOps.resW[curID] = lpPCM->getAlphaRight();
        }
            break;
        case(bntOpRicciBlend):
        {
            CRicciBlend* ricci = reinterpret_cast<CRicciBlend*>(root);
            float n = ricci->getN();
            m_blobOps.resX[curID] = n;
            if(n != 0.0f)
                m_blobOps.resY[curID] = 1.0f / n;
        }
            break;
        case(bntOpWarpTwist):
        {
            CWarpTwist* twist = reinterpret_cast<CWarpTwist*>(root);
            m_blobOps.resX[curID] = twist->getWarpFactor();
            m_blobOps.resY[curID] = static_cast<float>(twist->getMajorAxis());
        }
            break;
        case(bntOpWarpTaper):
        {
            CWarpTaper* taper = reinterpret_cast<CWarpTaper*>(root);
            m_blobOps.resX[curID] = taper->getWarpFactor();
            m_blobOps.resY[curID] = static_cast<float>(taper->getAxisAlong());
            m_blobOps.resZ[curID] = static_cast<float>(taper->getAxisTaper());
        }
            break;
        case(bntOpWarpBend):
        {
            CWarpBend* bend = reinterpret_cast<CWarpBend*>(root);
            m_blobOps.resX[curID] = bend->getBendRate();
            m_blobOps.resY[curID] = bend->getBendCenter();
            m_blobOps.resZ[curID] = bend->getBendRegion().left;
            m_blobOps.resW[curID] = bend->getBendRegion().right;
        }
            break;
        case(bntOpWarpShear):
        {
            CWarpShear* shear = reinterpret_cast<CWarpShear*>(root);
            m_blobOps.resX[curID] = shear->getWarpFactor();
            m_blobOps.resY[curID] = static_cast<float>(shear->getAxisAlong());
            m_blobOps.resZ[curID] = static_cast<float>(shear->getAxisDependent());
        }
            break;
        }

        return curID;
    }
    else
    {
        if(m_blobPrims.ctPrims >= MAX_TREE_NODES)
        {
            ReportError("Exceeded maximum number of allowed primitives");
            FlushAllErrors();
            return PS_ERROR_PRIM_OVERFLOW;
        }

        curID = m_blobPrims.ctPrims;
        m_blobPrims.ctPrims++;

        //
        vec4f d = root->getMaterial().diffused;
        m_blobPrims.colorX[curID] = d.x;
        m_blobPrims.colorY[curID] = d.y;
        m_blobPrims.colorZ[curID] = d.z;

        vec3f lo = root->getOctree().lower;
        vec3f hi = root->getOctree().upper;

        //ReadBox from BlobTree
        m_blobPrims.vPrimBoxLoX[curID] = lo.x;
        m_blobPrims.vPrimBoxLoY[curID] = lo.y;
        m_blobPrims.vPrimBoxLoZ[curID] = lo.z;

        m_blobPrims.vPrimBoxHiX[curID] = hi.x;
        m_blobPrims.vPrimBoxHiY[curID] = hi.y;
        m_blobPrims.vPrimBoxHiZ[curID] = hi.z;

        CMatrix mtxBackward = root->getTransform().getBackwardMatrix();
        if(mtxBackward.isIdentity())
        {
            m_blobPrims.idxMatrix[curID] = 0;
        }
        else
        {
            int idxMat = m_blobPrimMatrices.count;
            m_blobPrims.idxMatrix[curID] = idxMat;
            idxMat *= PRIM_MATRIX_STRIDE;

            float row[16];
            mtxBackward.getRow(&row[0], 0);
            mtxBackward.getRow(&row[4], 1);
            mtxBackward.getRow(&row[8], 2);
            mtxBackward.getRow(&row[12], 3);

            for(int i=0; i<PRIM_MATRIX_STRIDE; i++)
                m_blobPrimMatrices.matrix[idxMat + i] = row[i];
            m_blobPrimMatrices.count++;
        }

        m_blobPrims.skeletType[curID] = root->getNodeType();

        switch(root->getNodeType())
        {
        case(bntPrimPoint):
        {
            CSkeletonPrimitive *sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonPoint* skeletPoint = reinterpret_cast<CSkeletonPoint*>(sprim->getSkeleton());
            vec3f pos = skeletPoint->getPosition();

            m_blobPrims.posX[curID] = pos.x;
            m_blobPrims.posY[curID] = pos.y;
            m_blobPrims.posZ[curID] = pos.z;
        }
            break;
        case(bntPrimLine):
        {
            CSkeletonPrimitive *sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonLine* skeletLine = reinterpret_cast<CSkeletonLine*>(sprim->getSkeleton());
            //cfg->writeVec3f(strNodeName, "start", skeletLine->getStartPosition());
            //cfg->writeVec3f(strNodeName, "end", skeletLine->getEndPosition());
            vec3f s = skeletLine->getStartPosition();
            vec3f e = skeletLine->getEndPosition();

            m_blobPrims.posX[curID] = s.x;
            m_blobPrims.posY[curID] = s.y;
            m_blobPrims.posZ[curID] = s.z;
            m_blobPrims.dirX[curID] = e.x;
            m_blobPrims.dirY[curID] = e.y;
            m_blobPrims.dirZ[curID] = e.z;
        }
            break;
        case(bntPrimRing):
        {
            CSkeletonPrimitive *sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonRing* skeletRing = reinterpret_cast<CSkeletonRing*>(sprim->getSkeleton());
            //cfg->writeVec3f(strNodeName, "position", skeletRing->getPosition());
            //cfg->writeVec3f(strNodeName, "direction", skeletRing->getDirection());
            //cfg->writeFloat(strNodeName, "radius", skeletRing->getRadius());
            vec3f p = skeletRing->getPosition();
            vec3f d = skeletRing->getDirection();
            float r = skeletRing->getRadius();

            m_blobPrims.posX[curID] = p.x;
            m_blobPrims.posY[curID] = p.y;
            m_blobPrims.posZ[curID] = p.z;
            m_blobPrims.dirX[curID] = d.x;
            m_blobPrims.dirY[curID] = d.y;
            m_blobPrims.dirZ[curID] = d.z;
            m_blobPrims.resX[curID] = r;
            m_blobPrims.resY[curID] = r*r;

        }
            break;
        case(bntPrimDisc):
        {
            CSkeletonPrimitive *sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonDisc* skeletDisc = reinterpret_cast<CSkeletonDisc*>(sprim->getSkeleton());
            //cfg->writeVec3f(strNodeName, "position", skeletDisc->getPosition());
            //cfg->writeVec3f(strNodeName, "direction", skeletDisc->getDirection());
            //cfg->writeFloat(strNodeName, "radius", skeletDisc->getRadius());
            vec3f p = skeletDisc->getPosition();
            vec3f d = skeletDisc->getDirection();
            float r = skeletDisc->getRadius();
            m_blobPrims.posX[curID] = p.x;
            m_blobPrims.posY[curID] = p.y;
            m_blobPrims.posZ[curID] = p.z;
            m_blobPrims.dirX[curID] = d.x;
            m_blobPrims.dirY[curID] = d.y;
            m_blobPrims.dirZ[curID] = d.z;
            m_blobPrims.resX[curID] = r;
            m_blobPrims.resY[curID] = r*r;
        }
            break;
        case(bntPrimCylinder):
        {
            CSkeletonPrimitive *sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonCylinder* skeletCyl = reinterpret_cast<CSkeletonCylinder*>(sprim->getSkeleton());
            //cfg->writeVec3f(strNodeName, "position", skeletCyl->getPosition());
            //cfg->writeVec3f(strNodeName, "direction", skeletCyl->getDirection());
            //cfg->writeFloat(strNodeName, "radius", skeletCyl->getRadius());
            //cfg->writeFloat(strNodeName, "height", skeletCyl->getHeight());
            vec3f p = skeletCyl->getPosition();
            vec3f d = skeletCyl->getDirection();
            m_blobPrims.posX[curID] = p.x;
            m_blobPrims.posY[curID] = p.y;
            m_blobPrims.posZ[curID] = p.z;
            m_blobPrims.dirX[curID] = d.x;
            m_blobPrims.dirY[curID] = d.y;
            m_blobPrims.dirZ[curID] = d.z;
            m_blobPrims.resX[curID] = skeletCyl->getRadius();
            m_blobPrims.resY[curID] = skeletCyl->getHeight();
        }
            break;

        case(bntPrimCube):
        {
            CSkeletonPrimitive *sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonCube* skeletCube = reinterpret_cast<CSkeletonCube*>(sprim->getSkeleton());
            //cfg->writeVec3f(strNodeName, "position", skeletCube->getPosition());
            //cfg->writeFloat(strNodeName, "side", skeletCube->getSide());
            vec3f p = skeletCube->getPosition();
            float side = skeletCube->getSide();
            m_blobPrims.posX[curID] = p.x;
            m_blobPrims.posY[curID] = p.y;
            m_blobPrims.posZ[curID] = p.z;
            m_blobPrims.resX[curID] = side;

        }
            break;
        case(bntPrimTriangle):
        {
            CSkeletonPrimitive *sprim = reinterpret_cast<CSkeletonPrimitive*>(root);
            CSkeletonTriangle* skeletTriangle = reinterpret_cast<CSkeletonTriangle*>(sprim->getSkeleton());
            //cfg->writeVec3f(strNodeName, "corner0", skeletTriangle->getTriangleCorner(0));
            //cfg->writeVec3f(strNodeName, "corner1", skeletTriangle->getTriangleCorner(1));
            //cfg->writeVec3f(strNodeName, "corner2", skeletTriangle->getTriangleCorner(2));
            vec3f p0 = skeletTriangle->getTriangleCorner(0);
            vec3f p1 = skeletTriangle->getTriangleCorner(1);
            vec3f p2 = skeletTriangle->getTriangleCorner(2);
            m_blobPrims.posX[curID] = p0.x;
            m_blobPrims.posY[curID] = p0.y;
            m_blobPrims.posZ[curID] = p0.z;
            m_blobPrims.dirX[curID] = p1.x;
            m_blobPrims.dirY[curID] = p1.y;
            m_blobPrims.dirZ[curID] = p1.z;
            m_blobPrims.resX[curID] = p2.x;
            m_blobPrims.resY[curID] = p2.y;
            m_blobPrims.resX[curID] = p2.z;
        }
            break;
        case(bntPrimNull):
        {
            m_blobPrims.posX[curID] = 0.0f;
            m_blobPrims.posY[curID] = 0.0f;
            m_blobPrims.posZ[curID] = 0.0f;
        }
        break;
        default:
        {
            DAnsiStr strMsg = printToAStr("Primitive %s has not been implemented in compact mode yet!", root->getName().c_str());
            ReportError(strMsg.ptr());
            FlushAllErrors();
        }
        }

        return curID;
    }
}

int SimdPoly::linearizeBlobTree(CBlobNode* root)
{
    int isOperator;
    this->reset();
    return this->linearizeBlobTree(root, -1, isOperator);
}

int SimdPoly::run(float cellsize)
{
    return PS::SIMDPOLY::Polygonize(cellsize, m_blobPrims, m_blobPrimMatrices, m_blobOps, m_polyMPUs);
}

void SimdPoly::draw(bool bDrawNormals)
{
    if(m_polyMPUs.ctMPUs == 0) return;

    for (U32 i=0; i<m_polyMPUs.ctMPUs; i++)
    {
        if(m_polyMPUs.vMPUs[i].ctTriangles > 0)
        {
            //Color is RGB
            glColorPointer(3, GL_FLOAT, 0, m_polyMPUs.vMPUs[i].vColor);
            glEnableClientState(GL_COLOR_ARRAY);

            //Normals
            glNormalPointer(GL_FLOAT, 0, m_polyMPUs.vMPUs[i].vNorm);
            glEnableClientState(GL_NORMAL_ARRAY);

            //Vertex Unit is 3
            glVertexPointer(3, GL_FLOAT, 0, m_polyMPUs.vMPUs[i].vPos);
            glEnableClientState(GL_VERTEX_ARRAY);

            glDrawElements(GL_TRIANGLES, (GLsizei)m_polyMPUs.vMPUs[i].ctTriangles * 3, GL_UNSIGNED_SHORT, m_polyMPUs.vMPUs[i].triangles);
            //glDrawElements(GL_LINE_LOOP, (GLsizei)m_polyMPUs.vMPUs[i].ctTriangles * 3, GL_UNSIGNED_SHORT, m_polyMPUs.vMPUs[i].triangles);

            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);


            if(bDrawNormals)
            {
                glColor3f(1.0f, 0.0f, 0.0f);

                for(U32 j=0; j<m_polyMPUs.vMPUs[i].ctVertices; j++)
                {
                    svec3f v = svec3f(m_polyMPUs.vMPUs[i].vPos[j*3], m_polyMPUs.vMPUs[i].vPos[j*3 +1], m_polyMPUs.vMPUs[i].vPos[j*3 + 2]);
                    svec3f n = svec3f(m_polyMPUs.vMPUs[i].vNorm[j*3], m_polyMPUs.vMPUs[i].vNorm[j*3 +1], m_polyMPUs.vMPUs[i].vNorm[j*3 + 2]);

                    svec3f e = vadd3f(v, vscale3f(0.5f, n));

                    glBegin(GL_LINES);
                    glVertex3f(v.x, v.y, v.z);
                    glVertex3f(e.x, e.y, e.z);
                    glEnd();
                }
            }
        }
    }

}

//OpenCL polygonizer
int RunOclPolygonizer()
{
    int devType=CL_DEVICE_TYPE_GPU;

    cl_int err;     // error code returned from api calls

    size_t global;  // global domain size for our calculation
    size_t local;   // local domain size for our calculation

    cl_platform_id cpPlatform; // OpenCL platform
    cl_device_id device_id;    // compute device id
    cl_context context;        // compute context
    cl_command_queue commands; // compute command queue
    cl_program program;        // compute program
    cl_kernel kernel;          // compute kernel

    // Connect to a compute device
    err = clGetPlatformIDs(1, &cpPlatform, NULL);
    if (err != CL_SUCCESS) {
        cerr << "Error: Failed to find a platform!" << endl;
        return EXIT_FAILURE;
    }

    // Get a device of the appropriate type
    err = clGetDeviceIDs(cpPlatform, devType, 1, &device_id, NULL);
    if (err != CL_SUCCESS) {
        cerr << "Error: Failed to create a device group!" << endl;
        return EXIT_FAILURE;
    }

    // Create a compute context
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if (!context) {
        cerr << "Error: Failed to create a compute context!" << endl;
        return EXIT_FAILURE;
    }

    // Create a command commands
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands) {
        cerr << "Error: Failed to create a command commands!" << endl;
        return EXIT_FAILURE;
    }

    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1,
                                        (const char **) &KernelSource,
                                        NULL, &err);
    if (!program) {
        cerr << "Error: Failed to create compute program!" << endl;
        return EXIT_FAILURE;
    }

    // Build the program executable
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t len;
        char buffer[2048];

        cerr << "Error: Failed to build program executable!" << endl;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
                              sizeof(buffer), buffer, &len);
        cerr << buffer << endl;
        exit(1);
    }

    // Create the compute kernel in the program
    kernel = clCreateKernel(program, "square", &err);
    if (!kernel || err != CL_SUCCESS) {
        cerr << "Error: Failed to create compute kernel!" << endl;
        exit(1);
    }

    // create data for the run
    float* data = new float[DATA_SIZE];    // original data set given to device
    float* results = new float[DATA_SIZE]; // results returned from device
    unsigned int correct;               // number of correct results returned
    cl_mem input;                       // device memory used for the input array
    cl_mem output;                      // device memory used for the output array

    // Fill the vector with random float values
    unsigned int count = DATA_SIZE;
    for(int i = 0; i < count; i++)
        data[i] = rand() / (float)RAND_MAX;

    // Create the device memory vectors
    //
    input = clCreateBuffer(context,  CL_MEM_READ_ONLY,
                           sizeof(float) * count, NULL, NULL);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                            sizeof(float) * count, NULL, NULL);
    if (!input || !output) {
        cerr << "Error: Failed to allocate device memory!" << endl;
        exit(1);
    }

    // Transfer the input vector into device memory
    err = clEnqueueWriteBuffer(commands, input,
                               CL_TRUE, 0, sizeof(float) * count,
                               data, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        cerr << "Error: Failed to write to source array!" << endl;
        exit(1);
    }

    // Set the arguments to the compute kernel
    err = 0;
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
    err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);
    if (err != CL_SUCCESS) {
        cerr << "Error: Failed to set kernel arguments! " << err << endl;
        exit(1);
    }

    // Get the maximum work group size for executing the kernel on the device
    err = clGetKernelWorkGroupInfo(kernel, device_id,
                                   CL_KERNEL_WORK_GROUP_SIZE,
                                   sizeof(local), &local, NULL);
    if (err != CL_SUCCESS) {
        cerr << "Error: Failed to retrieve kernel work group info! "
             <<  err << endl;
        exit(1);
    }

    // Execute the kernel over the vector using the
    // maximum number of work group items for this device
    global = count;
    err = clEnqueueNDRangeKernel(commands, kernel,
                                 1, NULL, &global, &local,
                                 0, NULL, NULL);
    if (err) {
        cerr << "Error: Failed to execute kernel!" << endl;
        return EXIT_FAILURE;
    }

    // Wait for all commands to complete
    clFinish(commands);

    // Read back the results from the device to verify the output
    //
    err = clEnqueueReadBuffer( commands, output,
                               CL_TRUE, 0, sizeof(float) * count,
                               results, 0, NULL, NULL );
    if (err != CL_SUCCESS) {
        cerr << "Error: Failed to read output array! " <<  err << endl;
        exit(1);
    }

    // Validate our results
    //
    correct = 0;
    for(int i = 0; i < count; i++) {
        if(results[i] == data[i] * data[i])
            correct++;
    }

    // Print a brief summary detailing the results
    cout << "Computed " << correct << "/" << count << " correct values" << endl;
    cout << "Computed " << 100.f * (float)correct/(float)count
         << "% correct values" << endl;

    // Shutdown and cleanup
    delete [] data; delete [] results;

    clReleaseMemObject(input);
    clReleaseMemObject(output);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    return 0;
}


