#include <assert.h>
#include "PS_Polygonizer.h"
#include "_CellConfigTable.h"

#include "PS_SIMDVecN.h"
#include "PS_MATRIX4.h"

using namespace PS::MATHFUNCTIONAL;
using namespace PS::MATHSIMD;

namespace PS{
namespace SIMDPOLY{


typedef tbb::enumerable_thread_specific< std::pair<int, int> > CounterTotalCrossedMPU;
typedef tbb::enumerable_thread_specific< FieldComputer > ThreadFieldComputer;

CounterTotalCrossedMPU g_ctTotalCrossed(std::make_pair(0, 0));
ThreadFieldComputer g_threadFieldComputer;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
ThreadStartSetup::ThreadStartSetup(const SOABlobPrims* lpPrims,
                                   const SOABlobPrimMatrices* lpMatrices,
                                   const SOABlobOps* lpOps)
{
    m_lpBlobPrims = const_cast<SOABlobPrims*>(lpPrims);
    m_lpBlobMatrices = const_cast<SOABlobPrimMatrices*>(lpMatrices);
    m_lpBlobOps = const_cast<SOABlobOps*>(lpOps);
    observe(true);
}


void ThreadStartSetup::on_scheduler_entry(bool is_worker)
{
    ThreadFieldComputer::reference m_localFieldComputer = g_threadFieldComputer.local();


    //Create this copy for the thread
    memcpy(&m_localFieldComputer.m_blobPrims, m_lpBlobPrims, sizeof(SOABlobPrims));
    memcpy(&m_localFieldComputer.m_blobPrimMatrices, m_lpBlobMatrices, sizeof(SOABlobPrimMatrices));
    memcpy(&m_localFieldComputer.m_blobOps, m_lpBlobOps, sizeof(SOABlobOps));
    //	m_localFieldComputer.setup(m_lpBlobPrims, m_lpBlobMatrices, m_lpBlobOps);

    //__builtin_prefetch(&m_localFieldComputer.m_blobPrims);
    //__builtin_prefetch(&m_localFieldComputer.m_blobPrimMatrices);
    //__builtin_prefetch(&m_localFieldComputer.m_blobOps);
    PS_PREFETCH(&m_localFieldComputer.m_blobPrims);
    PS_PREFETCH(&m_localFieldComputer.m_blobPrimMatrices);
    PS_PREFETCH(&m_localFieldComputer.m_blobOps);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//KDTree and Blob Primitive Bounding Volumes are not created every time
//Just whenever there is a change in the Blobtree
int PrepareBBoxes(float cellsize, SOABlobPrims& blobPrims, SOABlobBoxMatrices& boxMatrices, SOABlobOps& blobOps)
{
    if(blobPrims.ctPrims == 0)
        return RET_PARAM_ERROR;

    //U8 cost[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
    //Compute all primitive BBoxes
    //Boxes are provided with input structure in future
    {
        float isoDist = ComputeWyvillIsoDistance(ISO_VALUE);
        isoDist += 5.0f * MIN_CELL_SIZE;

        svec3f bboxLo = svec3f(FLT_MAX, FLT_MAX, FLT_MAX);
        svec3f bboxHi = svec3f(FLT_MIN, FLT_MIN, FLT_MIN);

        //Computing bounding box per each primitive using its skeletal info
        for(size_t i=0; i<blobPrims.ctPrims; i++)
        {
            switch(blobPrims.skeletType[i])
            {
            case(sktPoint):
            {
                blobPrims.vPrimBoxLoX[i] = blobPrims.posX[i] - isoDist;
                blobPrims.vPrimBoxLoY[i] = blobPrims.posY[i] - isoDist;
                blobPrims.vPrimBoxLoZ[i] = blobPrims.posZ[i] - isoDist;

                blobPrims.vPrimBoxHiX[i] = blobPrims.posX[i] + isoDist;
                blobPrims.vPrimBoxHiY[i] = blobPrims.posY[i] + isoDist;
                blobPrims.vPrimBoxHiZ[i] = blobPrims.posZ[i] + isoDist;
            }
                break;
            case(sktLine):
            {
                svec3f s0 = svec3f(blobPrims.posX[i], blobPrims.posY[i], blobPrims.posZ[i]);
                svec3f s1 = svec3f(blobPrims.dirX[i], blobPrims.dirY[i], blobPrims.dirZ[i]);
                svec3f expand = vadd3f(vscale3f(isoDist, svec3f(1.0f, 1.0f, 1.0f)), vscale3f(3.0f*isoDist, vsub3f(s1, s0)));

                blobPrims.vPrimBoxLoX[i] = s0.x - expand.x;
                blobPrims.vPrimBoxLoY[i] = s0.y - expand.y;
                blobPrims.vPrimBoxLoZ[i] = s0.z - expand.z;

                blobPrims.vPrimBoxHiX[i] = s1.x + expand.x;
                blobPrims.vPrimBoxHiY[i] = s1.y + expand.y;
                blobPrims.vPrimBoxHiZ[i] = s1.z + expand.z;
            }
                break;
            case(sktRing):
            {
                svec3f pos = svec3f(blobPrims.posX[i], blobPrims.posY[i], blobPrims.posZ[i]);
                svec3f dir = svec3f(blobPrims.dirX[i], blobPrims.dirY[i], blobPrims.dirZ[i]);
                float radius = blobPrims.resX[i];

                svec3f dirComp = vsub3f(svec3f(1.0f, 1.0f, 1.0f), dir);
                radius += isoDist;

                svec3f expand = vadd3f(vscale3f(radius + isoDist, dirComp), vscale3f(isoDist, dir));

                blobPrims.vPrimBoxLoX[i] = pos.x - expand.x;
                blobPrims.vPrimBoxLoY[i] = pos.y - expand.y;
                blobPrims.vPrimBoxLoZ[i] = pos.z - expand.z;

                blobPrims.vPrimBoxHiX[i] = pos.x + expand.x;
                blobPrims.vPrimBoxHiY[i] = pos.y + expand.y;
                blobPrims.vPrimBoxHiZ[i] = pos.z + expand.z;
            }
                break;
            case(sktDisc):
            {
                svec3f pos = svec3f(blobPrims.posX[i], blobPrims.posY[i], blobPrims.posZ[i]);
                svec3f dir = svec3f(blobPrims.dirX[i], blobPrims.dirY[i], blobPrims.dirZ[i]);
                float radius = blobPrims.resX[i];

                svec3f dirComp = vsub3f(svec3f(1.0f, 1.0f, 1.0f), dir);
                radius += isoDist;

                svec3f expand = vadd3f(vscale3f(radius + isoDist, dirComp), vscale3f(isoDist, dir));

                blobPrims.vPrimBoxLoX[i] = pos.x - expand.x;
                blobPrims.vPrimBoxLoY[i] = pos.y - expand.y;
                blobPrims.vPrimBoxLoZ[i] = pos.z - expand.z;

                blobPrims.vPrimBoxHiX[i] = pos.x + expand.x;
                blobPrims.vPrimBoxHiY[i] = pos.y + expand.y;
                blobPrims.vPrimBoxHiZ[i] = pos.z + expand.z;
            }
                break;
            case(sktCylinder):
            {
                svec3f s0 = svec3f(blobPrims.posX[i], blobPrims.posY[i], blobPrims.posZ[i]);
                svec3f dir = svec3f(blobPrims.dirX[i], blobPrims.dirY[i], blobPrims.dirZ[i]);
                float radius = blobPrims.resX[i];
                float height = blobPrims.resY[i];

                svec3f s1 = vadd3f(s0, vscale3f(height, dir));
                svec3f expand = vadd3f(vscale3f(isoDist + radius, svec3f(1.0f, 1.0f, 1.0f)), vscale3f(0.5f*isoDist, dir));

                blobPrims.vPrimBoxLoX[i] = s0.x - expand.x;
                blobPrims.vPrimBoxLoY[i] = s0.y - expand.y;
                blobPrims.vPrimBoxLoZ[i] = s0.z - expand.z;

                blobPrims.vPrimBoxHiX[i] = s1.x + expand.x;
                blobPrims.vPrimBoxHiY[i] = s1.y + expand.y;
                blobPrims.vPrimBoxHiZ[i] = s1.z + expand.z;
            }
                break;
            case(sktCube):
            {
                svec3f s0 = svec3f(blobPrims.posX[i], blobPrims.posY[i], blobPrims.posZ[i]);
                float side = blobPrims.resX[i] + isoDist;

                svec3f slo = vsub3f(s0, svec3f(side, side, side));
                svec3f shi = vadd3f(s0, svec3f(side, side, side));

                blobPrims.vPrimBoxLoX[i] = slo.x;
                blobPrims.vPrimBoxLoY[i] = slo.y;
                blobPrims.vPrimBoxLoZ[i] = slo.z;

                blobPrims.vPrimBoxHiX[i] = shi.x;
                blobPrims.vPrimBoxHiY[i] = shi.y;
                blobPrims.vPrimBoxHiZ[i] = shi.z;
            }
                break;
            case(sktTriangle):
            {
                svec3f s0 = svec3f(blobPrims.posX[i], blobPrims.posY[i], blobPrims.posZ[i]);
                svec3f s1 = svec3f(blobPrims.dirX[i], blobPrims.dirY[i], blobPrims.dirZ[i]);
                svec3f s2 = svec3f(blobPrims.resX[i], blobPrims.resY[i], blobPrims.resZ[i]);

                svec3f slo = vmin3f(vmin3f(s0, s1), s2);
                svec3f shi = vmax3f(vmax3f(s0, s1), s2);

                blobPrims.vPrimBoxLoX[i] = slo.x - isoDist;
                blobPrims.vPrimBoxLoY[i] = slo.y - isoDist;
                blobPrims.vPrimBoxLoZ[i] = slo.z - isoDist;

                blobPrims.vPrimBoxHiX[i] = shi.x + isoDist;
                blobPrims.vPrimBoxHiY[i] = shi.y + isoDist;
                blobPrims.vPrimBoxHiZ[i] = shi.z + isoDist;
            }
                break;
            }

            svec3f curLo = svec3f(blobPrims.vPrimBoxLoX[i], blobPrims.vPrimBoxLoY[i], blobPrims.vPrimBoxLoZ[i]);
            svec3f curHi = svec3f(blobPrims.vPrimBoxHiX[i], blobPrims.vPrimBoxHiY[i], blobPrims.vPrimBoxHiZ[i]);

            //Apply Transformation Matrix to BBox
            int idxMatrix = blobPrims.idxMatrix[i];
            if(idxMatrix != 0)
            {
                assert(idxMatrix < (int)boxMatrices.count);
                idxMatrix *= BOX_MATRIX_STRIDE;
                MAT44 mat(&boxMatrices.matrix[idxMatrix]);

                curLo = mat4Transform(mat, curLo);
                curHi = mat4Transform(mat, curHi);

                blobPrims.vPrimBoxLoX[i] = curLo.x;
                blobPrims.vPrimBoxLoY[i] = curLo.y;
                blobPrims.vPrimBoxLoZ[i] = curLo.z;

                blobPrims.vPrimBoxHiX[i] = curHi.x;
                blobPrims.vPrimBoxHiY[i] = curHi.y;
                blobPrims.vPrimBoxHiZ[i] = curHi.z;
            }


            if(i == 0)
            {
                bboxLo = curLo;
                bboxHi = curHi;
            }
            else
            {
                bboxLo = vmin3f(bboxLo, curLo);
                bboxHi = vmax3f(bboxHi, curHi);
            }
        }

        blobPrims.bboxLo = bboxLo;
        blobPrims.bboxHi = bboxHi;
    }

    //Prepare Boxes for Ops
    if(blobOps.ctOps > 0)
    {
        SIMPLESTACK<MAX_TREE_NODES> stkOps;
        U8 opBoxComputed[MAX_TREE_NODES];
        memset(opBoxComputed, 0, MAX_TREE_NODES);

        U32 idxOp, idxLC, idxRC;
        U8 lChildIsOp, rChildIsOp, bReady;

        stkOps.push(0);
        while(!stkOps.empty())
        {
            idxOp = stkOps.id[stkOps.idxTop];
            idxLC = blobOps.opLeftChild[idxOp];
            idxRC = blobOps.opRightChild[idxOp];
            lChildIsOp = (blobOps.opChildKind[idxOp] & 2) >> 1;
            rChildIsOp = (blobOps.opChildKind[idxOp] & 1);

            bReady = !((lChildIsOp && (opBoxComputed[idxLC] == 0))||
                       (rChildIsOp && (opBoxComputed[idxRC] == 0)));
            if(bReady)
            {
                stkOps.pop();
                svec3f lcLo, lcHi, rcLo, rcHi;
                if(lChildIsOp)
                {
                    lcLo = svec3f(blobOps.vBoxLoX[idxLC], blobOps.vBoxLoY[idxLC], blobOps.vBoxLoZ[idxLC]);
                    lcHi = svec3f(blobOps.vBoxHiX[idxLC], blobOps.vBoxHiY[idxLC], blobOps.vBoxHiZ[idxLC]);
                }
                else
                {
                    lcLo = svec3f(blobPrims.vPrimBoxLoX[idxLC], blobPrims.vPrimBoxLoY[idxLC], blobPrims.vPrimBoxLoZ[idxLC]);
                    lcHi = svec3f(blobPrims.vPrimBoxHiX[idxLC], blobPrims.vPrimBoxHiY[idxLC], blobPrims.vPrimBoxHiZ[idxLC]);
                }

                if(rChildIsOp)
                {
                    rcLo = svec3f(blobOps.vBoxLoX[idxRC], blobOps.vBoxLoY[idxRC], blobOps.vBoxLoZ[idxRC]);
                    rcHi = svec3f(blobOps.vBoxHiX[idxRC], blobOps.vBoxHiY[idxRC], blobOps.vBoxHiZ[idxRC]);
                }
                else
                {
                    rcLo = svec3f(blobPrims.vPrimBoxLoX[idxRC], blobPrims.vPrimBoxLoY[idxRC], blobPrims.vPrimBoxLoZ[idxRC]);
                    rcHi = svec3f(blobPrims.vPrimBoxHiX[idxRC], blobPrims.vPrimBoxHiY[idxRC], blobPrims.vPrimBoxHiZ[idxRC]);
                }

                lcLo = vmin3f(lcLo, rcLo);
                lcHi = vmax3f(lcHi, rcHi);

                opBoxComputed[idxOp] = 1;
                blobOps.vBoxLoX[idxOp] = lcLo.x;
                blobOps.vBoxLoY[idxOp] = lcLo.y;
                blobOps.vBoxLoZ[idxOp] = lcLo.z;

                blobOps.vBoxHiX[idxOp] = lcHi.x;
                blobOps.vBoxHiY[idxOp] = lcHi.y;
                blobOps.vBoxHiZ[idxOp] = lcHi.z;
            }
            else
            {
                //If lchild is op and not processed
                if(lChildIsOp && (opBoxComputed[idxLC] == 0))
                    stkOps.push(idxLC);

                //If rchild is op and not processed
                if(rChildIsOp && (opBoxComputed[idxRC] == 0))
                    stkOps.push(idxRC);
            }
        }
    }
    return RET_SUCCESS;
}

/*!
 * Main polygonizer function. Creates MPUs and setup the runnable threads using
 * TBB
 */
int Polygonize(float cellsize,
               const SOABlobPrims& blobPrims,
               const SOABlobPrimMatrices& blobPrimMatrices,
               const SOABlobOps& blobOps,
               PolyMPUs& polyMPUs,
               MPUSTATS* lpProcessStats)
{
    if(blobPrims.ctPrims == 0)
        return RET_PARAM_ERROR;

    {
        polyMPUs.ctMPUs = 0;

        const int cellsPerMPU = GRID_DIM - 1;
        const float side = cellsize * (float)cellsPerMPU;


        {
            svec3f lo = blobPrims.bboxLo;
            svec3f hi = blobPrims.bboxHi;
            svec3f allSides = vsub3f(hi, lo);
            svec3i ctCellsNeeded;
            svec3i ctMPUNeeded;

            ctCellsNeeded.x = static_cast<int>(ceil(allSides.x / cellsize));
            ctCellsNeeded.y = static_cast<int>(ceil(allSides.y / cellsize));
            ctCellsNeeded.z = static_cast<int>(ceil(allSides.z / cellsize));


            ctMPUNeeded.x = ctCellsNeeded.x / cellsPerMPU;
            ctMPUNeeded.y = ctCellsNeeded.y / cellsPerMPU;
            ctMPUNeeded.z = ctCellsNeeded.z / cellsPerMPU;
            if (ctCellsNeeded.x % cellsPerMPU != 0)
                ctMPUNeeded.x++;
            if (ctCellsNeeded.y % cellsPerMPU != 0)
                ctMPUNeeded.y++;
            if (ctCellsNeeded.z % cellsPerMPU != 0)
                ctMPUNeeded.z++;

            U32 ctTotalMPU = ctMPUNeeded.x * ctMPUNeeded.y * ctMPUNeeded.z;
            if(ctTotalMPU >= MAX_MPU_COUNT)
                printf("WARNING! MAX MPU Count Reached! Total = %d MAX = %d\n", ctTotalMPU, MAX_MPU_COUNT);

            //Create all MPUs
            //Create all intersecting MPUs
            for (int i = 0; i < ctMPUNeeded.x; i++) {
                for (int j = 0; j < ctMPUNeeded.y; j++) {
                    for (int k = 0; k < ctMPUNeeded.z; k++) {
                        if (polyMPUs.ctMPUs < MAX_MPU_COUNT) {
                            size_t idxMPU = polyMPUs.ctMPUs;

                            polyMPUs.vMPUs[idxMPU].bboxLo = vadd3f(lo, svec3f((float) i * side, (float) j * side, (float) k * side));
                            polyMPUs.ctMPUs++;
                        }
                    }
                }
            }

        }
    }

    //Now Run Threads
    ThreadStartSetup startSetup(&blobPrims, &blobPrimMatrices, &blobOps);

    CMPUProcessor body(cellsize, polyMPUs.ctMPUs, polyMPUs.vMPUs, lpProcessStats);
    tbb::parallel_for(blocked_range<size_t>(0, polyMPUs.ctMPUs), body,
                      tbb::auto_partitioner());
    g_threadFieldComputer.clear();

    return RET_SUCCESS;
}

//Returns the count of needed MPUs
U32 CountMPUNeeded(float cellsize, const svec3f& lo, const svec3f& hi)
{
    const int cellsPerMPU = GRID_DIM - 1;
    //const float side = cellsize * (float)cellsPerMPU;

    svec3f allSides = vsub3f(hi, lo);
    svec3i ctCellsNeeded;
    svec3i ctMPUNeeded;

    ctCellsNeeded.x = static_cast<int>(ceil(allSides.x / cellsize));
    ctCellsNeeded.y = static_cast<int>(ceil(allSides.y / cellsize));
    ctCellsNeeded.z = static_cast<int>(ceil(allSides.z / cellsize));

    ctMPUNeeded.x = ctCellsNeeded.x / cellsPerMPU;
    ctMPUNeeded.y = ctCellsNeeded.y / cellsPerMPU;
    ctMPUNeeded.z = ctCellsNeeded.z / cellsPerMPU;
    if (ctCellsNeeded.x % cellsPerMPU != 0)
        ctMPUNeeded.x++;
    if (ctCellsNeeded.y % cellsPerMPU != 0)
        ctMPUNeeded.y++;
    if (ctCellsNeeded.z % cellsPerMPU != 0)
        ctMPUNeeded.z++;

    return ctMPUNeeded.x * ctMPUNeeded.y * ctMPUNeeded.z;
}

void PrintThreadResults(int ctAttempts, U32* lpThreadProcessed, U32* lpThreadCrossed)
{
    //Print Results
    int idxThread = 0;
    for(CounterTotalCrossedMPU::const_iterator i = g_ctTotalCrossed.begin(); i != g_ctTotalCrossed.end(); i++)
    {
        if(lpThreadProcessed)
            lpThreadProcessed[idxThread] = i->first / ctAttempts;
        if(lpThreadCrossed)
            lpThreadCrossed[idxThread] = i->second / ctAttempts;
        idxThread++;
        printf("Thread#  %d, Processed MPUs %d, Crossed MPUs %d \n", idxThread, i->first / ctAttempts, i->second / ctAttempts);
    }
    g_ctTotalCrossed.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////
CMPUProcessor::CMPUProcessor(float cellsize,
                             size_t ctMPUs, MPU* lpMPU, MPUSTATS* lpStats)

{
    m_cellsize = cellsize;
    m_ctMPUs = ctMPUs;
    m_lpMPU = lpMPU;
    m_lpStats = lpStats;
}

void CMPUProcessor::operator()(const blocked_range<size_t>& range) const
{
    CounterTotalCrossedMPU::reference m_localCounter = g_ctTotalCrossed.local();
    ThreadFieldComputer::reference m_localFieldComputer = g_threadFieldComputer.local();
    tbb_thread::id threadID = this_tbb_thread::get_id();

    for(size_t i=range.begin(); i!= range.end(); i++)
    {
        if(m_lpStats)
        {
            m_lpStats[i].threadID = threadID;
            m_lpStats[i].tickStart = tick_count::now();
        }

        //Call MPU processor
        process_cells_simd(m_localFieldComputer, m_lpMPU[i]);
        //process_cells_fieldsimd_cellserial(m_localFieldComputer, m_lpMPU[i]);
        //process_cells_scalar(m_localFieldComputer, m_lpMPU[i]);

        if(m_lpStats)
            m_lpStats[i].tickEnd = tick_count::now();

        //Increment Total MPU Counter
        ++m_localCounter.first;

        //Increment intersected MPUs Counter
        if(m_lpMPU[i].ctTriangles > 0)
            ++m_localCounter.second;
    }
}

/*!
 * Processes cells in SIMD
 */
void CMPUProcessor::process_cells_simd(const FieldComputer& fc, MPU& mpu) const
{	
    //Create Field-Value Cache and init it
    const U32 m = GRID_DIM * GRID_DIM * GRID_DIM;
    float PS_SIMD_ALIGN(fvCache[m]);

    //Check Zhang method Problem with some

    {
        float PS_SIMD_ALIGN(arrX[8]);
        float PS_SIMD_ALIGN(arrY[8]);
        float PS_SIMD_ALIGN(arrZ[8]);

        for(int i=0; i<8; i++)
        {
            arrX[i] = (float)((0xAA >> i) & 1);
            arrY[i] = (float)((0xCC >> i) & 1);
            arrZ[i] = (float)((0xF0 >> i) & 1);
        }

#ifdef SIMD_USE_M128
        //My SIMD VARS
        Float_ X(&arrX[0]);
        Float_ Y(&arrY[0]);
        Float_ Z1(&arrZ[0]);
        Float_ Z2(&arrZ[4]);
        Float_ zero(0.0f);
        Float_ one(1.0f);
        Float_ side((GRID_DIM - 1) * m_cellsize);
        X = (X * side) + Float_(mpu.bboxLo.x);
        Y = (Y * side) + Float_(mpu.bboxLo.y);
        Z1 = (Z1 * side) + Float_(mpu.bboxLo.z);
        Z2 = (Z2 * side) + Float_(mpu.bboxLo.z);

        Float_ F1;
        Float_ F2;
        fc.fieldValue(X, Y, Z1, F1);
        fc.fieldValue(X, Y, Z2, F2);

        F1 = SimdAnd(SimdGT(F1, zero), one);
        F2 = SimdAnd(SimdGT(F2, zero), one);
        VecNMask mask1(F1.v);
        VecNMask mask2(F2.v);
        if((mask1 == PS_SIMD_ALLZERO)&&(mask2 == PS_SIMD_ALLZERO))
            return;

#elif defined(SIMD_USE_M256)
        Float_ X(&arrX);
        Float_ Y(&arrY);
        Float_ Z(&arrZ);
        Float_ zero(0.0f);
        Float_ one(1.0f);
        Float_ side((GRID_DIM - 1) * m_cellsize);
        X = (X * side) + Float_(mpu.bboxLo.x);
        Y = (Y * side) + Float_(mpu.bboxLo.y);
        Z = (Z * side) + Float_(mpu.bboxLo.z);

        Float_ F;
        fc.fieldValue(X, Y, Z, F);
        F = SimdAnd(SimdGT(F, zero), one);
        VecNMask mask1(F.v);
        if(mask1 == PS_SIMD_ALLZERO)
            return;

#endif
    }


    //Init
    EDGETABLE edgeTable;
    memset(&edgeTable, 0, sizeof(EDGETABLE));
    mpu.ctFieldEvals = 0;
    mpu.ctVertices = 0;
    mpu.ctTriangles = 0;

    //Compute steps
    const int kMax = PS_SIMD_BLOCKS(GRID_DIM);
    const int szOneNeedle = PS_SIMD_FLEN * kMax;
    const int szOneSlab = szOneNeedle * GRID_DIM;

    //Temporary goodies
    Float_ arrIsoVal(ISO_VALUE);
    Float_ one(1.0f);
    float PS_SIMD_ALIGN(scaleZ[PS_SIMD_FLEN]);
    for(int iSimd=0; iSimd<PS_SIMD_FLEN; iSimd++)
        scaleZ[iSimd] = (float)iSimd;

    //Compute field at grid positions
    {
        //Temp variables
        Float_ arrInside(0.0f);
        Float_ arrOutside(0.0f);

        Float_ arrCellSize(m_cellsize);
        Float_ arrScaleZ(scaleZ);

        //Cache all field-values with in this MPU
        for(int i=0; i<GRID_DIM; i++)
        {
            for(int j=0; j<GRID_DIM; j++)
            {
                for(int k=0; k<kMax; k++)
                {
                    Float_ cellCornerPOSX_(mpu.bboxLo.x + i*m_cellsize);
                    Float_ cellCornerPOSY_(mpu.bboxLo.y + j*m_cellsize);
                    Float_ cellCornerPOSZ_(mpu.bboxLo.z);
                    Float_ k_((float)k * PS_SIMD_FLEN);

                    cellCornerPOSZ_ = cellCornerPOSZ_ + (k_ + arrScaleZ) * arrCellSize;

                    Float_ arrField;

                    //FieldValue computed using SIMD
                    fc.fieldValue(cellCornerPOSX_, cellCornerPOSY_, cellCornerPOSZ_, arrField);

                    //Store Fields Row By Row
                    arrField.store(&fvCache[ i * szOneNeedle + j * szOneSlab + k * PS_SIMD_FLEN]);

                    //Keep stats
                    mpu.ctFieldEvals++;
                    cellCornerPOSX_ = SimdAnd(SimdGTE(arrField, arrIsoVal), one);
                    arrInside = arrInside + cellCornerPOSX_;
                    arrOutside = arrOutside + one - cellCornerPOSX_;
                }
            }
        }

        //Early Discard Shaved 2 seconds off
        VecNMask mask1(arrInside.v);
        if(mask1 == PS_SIMD_ALLZERO)
            return;

        mask1 = VecNMask(arrOutside.v);
        if(mask1 == PS_SIMD_ALLZERO)
            return;
    }
    /////////////////////////////////////////////////////////
    //Process Multiple cells- Column Swap Method
    //For the 8 corners of each cell
    float PS_SIMD_ALIGN(arrPowerMask[8]);
    float PS_SIMD_ALIGN(arrFields[8]);

    arrPowerMask[0] = 1.0f;
    for(int iSimd=1; iSimd<8; iSimd++)
        arrPowerMask[iSimd] = 2.0f * arrPowerMask[iSimd - 1];

    int arrConfig[16];
    int idxCellConfig = 0;
#ifdef SIMD_USE_M128
    Float_ slabLeft;
    Float_ slabRight;
    Float_ maskPowerLeft = Float_(&arrPowerMask[0]);
    Float_ maskPowerRight = Float_(&arrPowerMask[4]);
    Float_ rootResolution;
    {
        Float_ scale(scaleZ);
        //Float_ oneThirdScaled(m_cellsize / 3.0f);
        Float_ oneThird(1.0f / 3.0f);
        rootResolution = scale * oneThird;
    }

#elif defined(SIMD_USE_M256)
    Float_ cell;
    Float_ maskPower = Float_(&arrPowerMask[0]);
    Float_ rootResolution;
    {
        Float_ scale(scaleZ);
        Float_ oneSeventh(1.0f / 7.0f);
        rootResolution = scale * oneSeventh;
    }
#endif

    for(int i=0; i<GRID_DIM-1; i++)
    {
        //Compute once use multiple of i
        U32 ip0 = i * szOneNeedle;
        U32 ip1 = ip0 + szOneNeedle;

        for(int j=0; j<GRID_DIM-1; j++)
        {
            //Compute once use multiple of j
            U32 jp0 = j * szOneSlab;
            U32 jp1 = jp0 + szOneSlab;

            for(int k=0; k<GRID_DIM-1; k++)
            {
                //GATHER FIELDS
                arrFields[0] = fvCache[ ip0 + jp0 + k];
                arrFields[1] = fvCache[ ip0 + jp0 + k + 1];
                arrFields[2] = fvCache[ ip0 + jp1 + k];
                arrFields[3] = fvCache[ ip0 + jp1 + k + 1];
                arrFields[4] = fvCache[ ip1 + jp0 + k];
                arrFields[5] = fvCache[ ip1 + jp0 + k + 1];
                arrFields[6] = fvCache[ ip1 + jp1 + k];
                arrFields[7] = fvCache[ ip1 + jp1 + k + 1];
#ifdef SIMD_USE_M128
                slabLeft = Float_(&arrFields[0]);
                slabRight = Float_(&arrFields[4]);

                //Compute configuration
                Float_ Left = SimdAnd(SimdGTE(slabLeft, arrIsoVal), one) * maskPowerLeft;
                Float_ Right = SimdAnd(SimdGTE(slabRight, arrIsoVal), one) * maskPowerRight;

                //3 Levels of HADDS Needed
                Left = SimdhAdd(Left, Right);
                Left = SimdhAdd(Left, Left);
                Left = SimdhAdd(Left, Left);
                idxCellConfig = static_cast<int>(Left[0]);
#elif defined(SIMD_USE_M256)
                cell = Float_(&arrFields[0]);
                Float_ Left = SimdAnd(SimdGTE(cell, arrIsoVal), one) * maskPower;
                Left = SimdhAdd(Left, Left);
                Left = SimdhAdd(Left, Left);
                idxCellConfig = static_cast<int>(Left[0] + Left[4]);
#endif
                if(idxCellConfig == 0 || idxCellConfig == 255)
                    continue;

                //Fetch cell configuration
                memcpy(arrConfig, &g_triTableCache[idxCellConfig][0], 16 * sizeof(int));
                //Increment number of processed cells
                //Compute surface vertex, normal and material on each crossing edge
                int idxMeshVertex[16];
                Left = SimdhAdd(Left, Left);
                //Read case
                int ctTotalPolygons = 0;
                int ctEdges = 0;

                for(int icase=0; icase<16; icase++)
                {
                    int candidate = arrConfig[icase];
                    if(candidate != -1)
                    {
                        int idxEdgeStart = corner1[candidate];
                        int idxEdgeAxis  = edgeaxis[candidate];

                        //Compute indices
                        int sx = i + ((idxEdgeStart & 4) >> 2);
                        int sy = j + ((idxEdgeStart & 2) >> 1);
                        int sz = k + (idxEdgeStart & 1);

                        idxMeshVertex[icase] = getEdge(edgeTable, sx, sy, sz, idxEdgeAxis);

                        //See if the vertex exist in edge table. If it doesn't exist compute and add it to edge table
                        if(idxMeshVertex[icase] == -1)
                        {
                            //Reduced operations for edge processing
                            svec3f e1 = vadd3f(mpu.bboxLo, svec3f(m_cellsize * sx, m_cellsize * sy, m_cellsize * sz));
                            svec3f e2 = e1;
                            vsetElement3f(e2, idxEdgeAxis, velement3f(e2, idxEdgeAxis) + m_cellsize);


                            //int idxEdgeStart = corner2[candidate];
                            Float_ rootFields;
                            Float_ posX(e1.x);
                            Float_ posY(e1.y);
                            Float_ posZ(e1.z);
                            {
                                Float_ displaceX(e2.x - e1.x);
                                Float_ displaceY(e2.y - e1.y);
                                Float_ displaceZ(e2.z - e1.z);

                                posX = posX + displaceX * rootResolution;
                                posY = posY + displaceY * rootResolution;
                                posZ = posZ + displaceZ * rootResolution;

                                fc.fieldValue(posX, posY, posZ, rootFields);
                            }

                            Float_ inside = SimdAnd(SimdGTE(rootFields, arrIsoVal), one);
                            VecNMask mask(inside.v);

                            //Find the interval that contains the root
                            U32 state = mask.u.m[0];
                            int interval = 0;
                            for(int i=1; i<PS_SIMD_FLEN; i++)
                            {
                                interval = i;
                                if(mask.u.m[i] != state)
                                    break;
                            }

                            //Update e1 and e2 for the high resolution field
                            e1 = svec3f(posX[interval - 1], posY[interval - 1], posZ[interval - 1]);
                            e2 = svec3f(posX[interval], posY[interval], posZ[interval]);

                            float scale = (ISO_VALUE - rootFields[interval - 1])/(rootFields[interval] - rootFields[interval-1]);
                            svec3f p = vadd3f(e1, vscale3f(scale, vsub3f(e2, e1)));

                            Float_ cellCornerPosX_(p.x);
                            Float_ cellCornerPosY_(p.y);
                            Float_ cellCornerPosZ_(p.z);
                            Float_ vtxField;
                            Float_ outColorX;
                            Float_ outColorY;
                            Float_ outColorZ;

                            Float_ outNormalX;
                            Float_ outNormalY;
                            Float_ outNormalZ;

                            //Compute Field and Color
                            fc.fieldValueAndColor(cellCornerPosX_, cellCornerPosY_, cellCornerPosZ_,
                                                  vtxField, outColorX, outColorY, outColorZ);

                            //Use computed field to get the normal
                            fc.normal(cellCornerPosX_, cellCornerPosY_, cellCornerPosZ_, vtxField,
                                      outNormalX, outNormalY, outNormalZ, NORMAL_DELTA);


                            U16 idxVertex = mpu.ctVertices;
                            U16 idxVertex_X = idxVertex * 3;
                            U16 idxVertex_Y = idxVertex * 3 + 1;
                            U16 idxVertex_Z = idxVertex * 3 + 2;

                            mpu.vPos[idxVertex_X]   = p.x;
                            mpu.vPos[idxVertex_Y]   = p.y;
                            mpu.vPos[idxVertex_Z]   = p.z;

                            mpu.vNorm[idxVertex_X]  = outNormalX[0];
                            mpu.vNorm[idxVertex_Y]  = outNormalY[0];
                            mpu.vNorm[idxVertex_Z]  = outNormalZ[0];

                            mpu.vColor[idxVertex_X] = outColorX[0];
                            mpu.vColor[idxVertex_Y] = outColorY[0];
                            mpu.vColor[idxVertex_Z] = outColorZ[0];

                            mpu.ctVertices++;


                            //Get vertex v index from list. It is the last one
                            idxMeshVertex[icase] = idxVertex;
                            setEdge(edgeTable, sx, sy, sz, idxEdgeAxis, idxVertex);
                        }

                        ctEdges++;
                    }
                    else
                        break;
                }//End icase

                //Number of polygons
                ctTotalPolygons = ctEdges / 3;
                for(int icase = 0; icase < ctTotalPolygons; icase++)
                {
                    int idxTriangle = mpu.ctTriangles;
                    mpu.triangles[idxTriangle * 3 + 0] = idxMeshVertex[icase*3 + 0];
                    mpu.triangles[idxTriangle * 3 + 1] = idxMeshVertex[icase*3 + 1];
                    mpu.triangles[idxTriangle * 3 + 2] = idxMeshVertex[icase*3 + 2];
                    mpu.ctTriangles++;
                }
            }//Processed One Cell
        }
    }
}


int CMPUProcessor::getEdge(const EDGETABLE& edgeTable, int i, int j, int k, int edgeAxis) const
{
    //Hash Value of the low corner
    int hashval = CELLID_FROM_IDX(i, j, k);

    //Check how many edges are registered at that vertex
    if(edgeTable.ctEdges[hashval] > 0)
    {
        //The following line is simply x = 0, y = 1 or z = 2 since the difference is one at one direction only.
        if(edgeTable.hasEdgeWithHiNeighbor[hashval*3 + edgeAxis])
            return edgeTable.idxVertices[hashval*3 + edgeAxis];
    }
    return -1;
}

int CMPUProcessor::getEdge(const EDGETABLE& edgeTable , svec3i& start, svec3i& end) const
{
    int i1 = start.x;
    int j1 = start.y;
    int k1 = start.z;
    int i2 = end.x;
    int j2 = end.y;
    int k2 = end.z;

    if (i1>i2 || (i1==i2 && (j1>j2 || (j1==j2 && k1>k2))))
    {
        int t;
        t=i1; i1=i2; i2=t;
        t=j1; j1=j2; j2=t;
        t=k1; k1=k2; k2=t;
        start = svec3i(i1, j1, k1);
        end = svec3i(i2, j2, k2);
    }

    //Hash Value of the low corner
    int hashval = CELLID_FROM_IDX(i1, j1, k1);

    //Check how many edges are registered at that vertex
    if(edgeTable.ctEdges[hashval] > 0)
    {
        //The following line is simply x = 0, y = 1 or z = 2 since the difference is one at one direction only.
        int idxEdge = (((i2 - i1) + (j2 - j1)*2 + (k2 - k1)*4) >> 1);
        if(edgeTable.hasEdgeWithHiNeighbor[hashval*3 + idxEdge])
            return edgeTable.idxVertices[hashval*3 + idxEdge];
    }
    return -1;
}


void CMPUProcessor::setEdge(EDGETABLE& edgeTable, int i, int j, int k, int edgeAxis, int vid) const
{
    int hashval = CELLID_FROM_IDX(i, j, k);

    edgeTable.ctEdges[hashval]++;
    edgeTable.hasEdgeWithHiNeighbor[hashval*3 + edgeAxis] = 1;
    edgeTable.idxVertices[hashval*3 + edgeAxis] = vid;
}

void CMPUProcessor::setEdge(EDGETABLE& edgeTable, svec3i& start, svec3i& end, int vid) const
{
    int i1 = start.x;
    int j1 = start.y;
    int k1 = start.z;
    int i2 = end.x;
    int j2 = end.y;
    int k2 = end.z;

    if (i1>i2 || (i1==i2 && (j1>j2 || (j1==j2 && k1>k2))))
    {
        int t;
        t=i1; i1=i2; i2=t;
        t=j1; j1=j2; j2=t;
        t=k1; k1=k2; k2=t;
        start = svec3i(i1, j1, k1);
        end = svec3i(i2, j2, k2);
    }

    int hashval = CELLID_FROM_IDX(i1, j1, k1);

    edgeTable.ctEdges[hashval]++;

    int idxEdge = (((i2 - i1) + (j2 - j1)*2 + (k2 - k1)*4) >> 1);
    edgeTable.hasEdgeWithHiNeighbor[hashval*3 + idxEdge] = 1;
    edgeTable.idxVertices[hashval*3 + idxEdge] = vid;
}

////////////////////////////////////////////////////////////////////////////////////////////////
FieldComputer::FieldComputer(SOABlobPrims* lpPrims, SOABlobBoxMatrices* lpPrimMatrices, SOABlobOps* lpOps)
{
    setup(lpPrims, lpPrimMatrices, lpOps);
}

void FieldComputer::setup(SOABlobPrims* lpPrims,
                          SOABlobBoxMatrices* lpPrimMatrices,
                          SOABlobOps* lpOps)
{
    memcpy(&m_blobPrims, lpPrims, sizeof(SOABlobPrims));
    memcpy(&m_blobPrimMatrices, lpPrimMatrices, sizeof(SOABlobPrimMatrices));
    memcpy(&m_blobOps, lpOps, sizeof(SOABlobOps));
}


void FieldComputer::computePrimitiveField(const Float_& pX, const Float_& pY, const Float_& pZ,
                                          Float_& primField, U32 idxPrimitive) const
{
    Float_ dist2;
    Float_ ptX, ptY, ptZ;
    dist2.setZero();

    U32 idxMatrix = m_blobPrims.idxMatrix[idxPrimitive];
    if(idxMatrix == 0)
    {
        ptX = pX;
        ptY = pY;
        ptZ = pZ;
    }
    else
    {
        assert(idxMatrix < m_blobPrimMatrices.count);
        idxMatrix *= PRIM_MATRIX_STRIDE;

        Float_ matc1 = m_blobPrimMatrices.matrix[idxMatrix];
        Float_ matc2 = m_blobPrimMatrices.matrix[idxMatrix + 1];
        Float_ matc3 = m_blobPrimMatrices.matrix[idxMatrix + 2];
        Float_ matc4 = m_blobPrimMatrices.matrix[idxMatrix + 3];
        ptX = matc1 * pX + matc2 * pY + matc3 * pZ + matc4;

        matc1 = m_blobPrimMatrices.matrix[idxMatrix + 4];
        matc2 = m_blobPrimMatrices.matrix[idxMatrix + 5];
        matc3 = m_blobPrimMatrices.matrix[idxMatrix + 6];
        matc4 = m_blobPrimMatrices.matrix[idxMatrix + 7];
        ptY = matc1 * pX + matc2 * pY + matc3 * pZ + matc4;

        matc1 = m_blobPrimMatrices.matrix[idxMatrix + 8];
        matc2 = m_blobPrimMatrices.matrix[idxMatrix + 9];
        matc3 = m_blobPrimMatrices.matrix[idxMatrix + 10];
        matc4 = m_blobPrimMatrices.matrix[idxMatrix + 11];
        ptZ = matc1 * pX + matc2 * pY + matc3 * pZ + matc4;
    }


    switch(m_blobPrims.skeletType[idxPrimitive])
    {
    case(sktPoint):
    {
        //this is just for a sphere
        Float_ distX = Float_(m_blobPrims.posX[idxPrimitive]) - ptX;
        Float_ distY = Float_(m_blobPrims.posY[idxPrimitive]) - ptY;
        Float_ distZ = Float_(m_blobPrims.posZ[idxPrimitive]) - ptZ;
        dist2 = (distX * distX) + (distY * distY) + (distZ * distZ);
    }
        break;
    case(sktLine):
    {
        Float_ line0X = Float_(m_blobPrims.posX[idxPrimitive]);
        Float_ line0Y = Float_(m_blobPrims.posY[idxPrimitive]);
        Float_ line0Z = Float_(m_blobPrims.posZ[idxPrimitive]);

        Float_ lineDeltaX = Float_(m_blobPrims.dirX[idxPrimitive]) - line0X;
        Float_ lineDeltaY = Float_(m_blobPrims.dirY[idxPrimitive]) - line0Y;
        Float_ lineDeltaZ = Float_(m_blobPrims.dirZ[idxPrimitive]) - line0Z;
        Float_ lineDeltaDot = lineDeltaX * lineDeltaX + lineDeltaY * lineDeltaY + lineDeltaZ * lineDeltaZ;

        Float_ distX = ptX - line0X;
        Float_ distY = ptY - line0Y;
        Float_ distZ = ptZ - line0Z;


        Float_ delta = distX * lineDeltaX + distY * lineDeltaY + distZ * lineDeltaZ;
        delta = delta / lineDeltaDot;

        //Nearest Point
        distX = ptX - (line0X + delta * lineDeltaX);
        distY = ptY - (line0Y + delta * lineDeltaY);
        distZ = ptZ - (line0Z + delta * lineDeltaZ);

        //Distance to nearest Point
        dist2 = (distX * distX) + (distY * distY) + (distZ * distZ);
    }
        break;
    case(sktCylinder):
    {
        Float_ posX = ptX - Float_(m_blobPrims.posX[idxPrimitive]);
        Float_ posY = ptY - Float_(m_blobPrims.posY[idxPrimitive]);
        Float_ posZ = ptZ - Float_(m_blobPrims.posZ[idxPrimitive]);


        Float_ dirX = Float_(m_blobPrims.dirX[idxPrimitive]);
        Float_ dirY = Float_(m_blobPrims.dirY[idxPrimitive]);
        Float_ dirZ = Float_(m_blobPrims.dirZ[idxPrimitive]);

        Float_ radius = Float_(m_blobPrims.resX[idxPrimitive]);
        Float_ height = Float_(m_blobPrims.resY[idxPrimitive]);

        Float_ zero(0.0f);
        Float_ y = posX * dirX + posY * dirY + posZ * dirZ;
        Float_ x = SimdMax(zero, SimdSqrt(posX * posX + posY * posY + posZ * posZ - y*y) - radius);

        //
        Float_ one(1.0f);
        Float_ mask = SimdGT(y, zero);
        mask = SimdAnd(mask, one);

        y = mask * SimdMax(zero, y - height) + (one - mask) * y;

        dist2 = x*x + y*y;
    }
        break;
    case(sktTriangle):
    {
        Float_ tri0X = Float_(m_blobPrims.posX[idxPrimitive]);
        Float_ tri0Y = Float_(m_blobPrims.posY[idxPrimitive]);
        Float_ tri0Z = Float_(m_blobPrims.posZ[idxPrimitive]);

        Float_ tri1X = Float_(m_blobPrims.dirX[idxPrimitive]);
        Float_ tri1Y = Float_(m_blobPrims.dirY[idxPrimitive]);
        Float_ tri1Z = Float_(m_blobPrims.dirZ[idxPrimitive]);

        Float_ tri2X = Float_(m_blobPrims.resX[idxPrimitive]);
        Float_ tri2Y = Float_(m_blobPrims.resY[idxPrimitive]);
        Float_ tri2Z = Float_(m_blobPrims.resZ[idxPrimitive]);

        //Compute Triangle distance
        dist2 = FLT_MAX;
    }
        break;

    case(sktCube):
    {
        Float_ posX = Float_(m_blobPrims.posX[idxPrimitive]);
        Float_ posY = Float_(m_blobPrims.posY[idxPrimitive]);
        Float_ posZ = Float_(m_blobPrims.posZ[idxPrimitive]);
        Float_ side = Float_(m_blobPrims.resX[idxPrimitive]);
        Float_ minusSide = Float_(-1.0f * m_blobPrims.resX[idxPrimitive]);

        Float_ difX = ptX - posX;
        Float_ difY = ptY - posY;
        Float_ difZ = ptZ - posZ;

        Float_ one(1.0f);

        //ALONG X
        Float_ maskMinus = SimdGT(minusSide, difX);
        Float_ maskPositive = SimdGT(difX, side);
        maskMinus = SimdAnd(maskMinus, one);
        maskPositive = SimdAnd(maskPositive, one);
        Float_ delta = (difX + side) * maskMinus + (difX - side) * maskPositive;
        dist2 = delta * delta;

        //ALONG Y
        maskMinus = SimdGT(minusSide, difY);
        maskPositive = SimdGT(difY, side);
        maskMinus = SimdAnd(maskMinus, one);
        maskPositive = SimdAnd(maskPositive, one);
        delta = (difY + side) * maskMinus + (difY - side) * maskPositive;
        dist2 += delta * delta;

        //ALONG Z
        maskMinus = SimdGT(minusSide, difZ);
        maskPositive = SimdGT(difZ, side);
        maskMinus = SimdAnd(maskMinus, one);
        maskPositive = SimdAnd(maskPositive, one);
        delta = (difZ + side) * maskMinus + (difZ - side) * maskPositive;
        dist2 += delta * delta;
    }
        break;

    case(sktDisc):
    {
        Float_ dX = ptX - Float_(m_blobPrims.posX[idxPrimitive]);
        Float_ dY = ptY - Float_(m_blobPrims.posY[idxPrimitive]);
        Float_ dZ = ptZ - Float_(m_blobPrims.posZ[idxPrimitive]);

        Float_ nX = Float_(m_blobPrims.dirX[idxPrimitive]);
        Float_ nY = Float_(m_blobPrims.dirY[idxPrimitive]);
        Float_ nZ = Float_(m_blobPrims.dirZ[idxPrimitive]);
        Float_ radius = Float_(m_blobPrims.resX[idxPrimitive]);

        Float_ dot = nX * dX + nY * dY + nZ * dZ;
        Float_ dirX = dX - nX * dot;
        Float_ dirY = dY - nY * dot;
        Float_ dirZ = dZ - nZ * dot;
        dot = dirX * dirX + dirY * dirY + dirZ * dirZ;

        Float_ rsdot = SimdRSqrt(dot);
        dirX = dirX * rsdot;
        dirY = dirY * rsdot;
        dirZ = dirZ * rsdot;

        nX = radius * dirX - dX;
        nY = radius * dirY - dY;
        nZ = radius * dirZ - dZ;

        Float_ mask = SimdGTE(radius * radius, dot);
        Float_ one(1.0f);
        mask = SimdAnd(mask, one);

        dist2 =  mask * (dX * dX + dY * dY + dZ * dZ - dot) +
                (one - mask) * (nX * nX + nY * nY + nZ * nZ);
    }
        break;

    case(sktRing):
    {
        Float_ dX = ptX - Float_(m_blobPrims.posX[idxPrimitive]);
        Float_ dY = ptY - Float_(m_blobPrims.posY[idxPrimitive]);
        Float_ dZ = ptZ - Float_(m_blobPrims.posZ[idxPrimitive]);

        Float_ nX = Float_(m_blobPrims.dirX[idxPrimitive]);
        Float_ nY = Float_(m_blobPrims.dirY[idxPrimitive]);
        Float_ nZ = Float_(m_blobPrims.dirZ[idxPrimitive]);
        Float_ radius = Float_(m_blobPrims.resX[idxPrimitive]);

        Float_ dot = nX * dX + nY * dY + nZ * dZ;
        Float_ dirX = dX - nX * dot;
        Float_ dirY = dY - nY * dot;
        Float_ dirZ = dZ - nZ * dot;
        dot = dirX * dirX + dirY * dirY + dirZ * dirZ;

        Float_ zero(0.0f);
        Float_ one(1.0f);

        Float_ mask = SimdEQ(dot, zero);
        mask = SimdAnd(mask, one);

        //Normalize Dir for (one - mask) case
        dot = SimdRSqrt(dot);
        dirX = dirX * dot;
        dirY = dirY * dot;
        dirZ = dirZ * dot;


        nX = radius * dirX - dX;
        nY = radius * dirY - dY;
        nZ = radius * dirZ - dZ;

        //dist2 = mask * (radius * radius + dX * dX + dY * dY + dZ * dZ);

        dist2 = mask * (radius * radius + dX * dX + dY * dY + dZ * dZ) +
                (one - mask) * (nX * nX + nY * nY + nZ * nZ);
    }
        break;

    }

    //Compute Output Field for primitive
    ComputeWyvillFieldValueSquare_(dist2, primField);
}


//For primitives check if the query point is inside BBOX
//Pass an array to get all fields due to all primitives for field and color function
int FieldComputer::fieldValue(const Float_& pX, const Float_& pY, const Float_& pZ,
                              Float_& outField, float* lpOutFieldPrims, float* lpOutFieldOps) const
{
    outField.setZero();

    const U32 szPrimFieldArrSize = PS_SIMD_PADSIZE(m_blobPrims.ctPrims * PS_SIMD_FLEN);
    const U32 szOpFieldArrSize   = PS_SIMD_PADSIZE(m_blobOps.ctOps * PS_SIMD_FLEN);

    //float PS_SIMD_ALIGN( arrPrimFields[szPrimFieldArrSize] );
    //float PS_SIMD_ALIGN( arrOpFields[szOpFieldArrSize] );
    float PS_SIMD_ALIGN(arrPrimFields[PS_SIMD_PADSIZE(MAX_TREE_NODES*PS_SIMD_FLEN)]);
    float PS_SIMD_ALIGN(arrOpFields[PS_SIMD_PADSIZE(MAX_TREE_NODES*PS_SIMD_FLEN)]);

    //Evaluate Operators
    if(m_blobOps.ctOps > 0)
    {
        outField.setZero();

        //NO VLA
        //U8 arrOpsFieldComputed[m_blobOps.ctOps];
        //memset(arrOpsFieldComputed, 0, m_blobOps.ctOps);
        U8 arrOpsFieldComputed[MAX_TREE_NODES];
        memset(arrOpsFieldComputed, 0, MAX_TREE_NODES);

        KDTREE_TRAVERSE_STACK<MAX_TREE_NODES> stkOps;
        stkOps.push(0, 0);

        U16 idxOp, idxLC, idxRC, depth;
        U8 childKind, opType;
        U8 lChildIsOp, rChildIsOp, bReady;


        while(!stkOps.empty())
        {
            idxOp = stkOps.id[stkOps.idxTop];
            depth = stkOps.depth[stkOps.idxTop];
            idxLC = m_blobOps.opLeftChild[idxOp];
            idxRC = m_blobOps.opRightChild[idxOp];
            childKind = m_blobOps.opChildKind[idxOp];
            lChildIsOp = (childKind & 2) >> 1;
            rChildIsOp = (childKind & 1);


            //Here process if we need to go down the tree
            if(depth > 3)
            {
                Float_ one(1.0f);
                Float_ opBoxLoX = Float_(m_blobOps.vBoxLoX[idxOp]);
                Float_ opBoxLoY = Float_(m_blobOps.vBoxLoY[idxOp]);
                Float_ opBoxLoZ = Float_(m_blobOps.vBoxLoZ[idxOp]);

                Float_ opBoxHiX = Float_(m_blobOps.vBoxHiX[idxOp]);
                Float_ opBoxHiY = Float_(m_blobOps.vBoxHiY[idxOp]);
                Float_ opBoxHiZ = Float_(m_blobOps.vBoxHiZ[idxOp]);

                Float_ inside = SimdAnd(one, SimdAnd(SimdGTE(pX, opBoxLoX), SimdGTE(opBoxHiX, pX)));
                inside = SimdOr(inside, SimdAnd(one, SimdAnd(SimdGTE(pY, opBoxLoY), SimdGTE(opBoxHiY, pY))));
                inside = SimdOr(inside, SimdAnd(one, SimdAnd(SimdGTE(pZ, opBoxLoZ), SimdGTE(opBoxHiZ, pZ))));

                if(VecNMask(inside.v) == PS_SIMD_ALLZERO)
                {
                    stkOps.pop();

                    outField.setZero();
                    outField.store(&arrOpFields[idxOp * PS_SIMD_FLEN]);
                    arrOpsFieldComputed[idxOp] = 1;
                    continue;
                }
            }




            bReady = !((lChildIsOp && (arrOpsFieldComputed[idxLC] == 0))||
                       (rChildIsOp && (arrOpsFieldComputed[idxRC] == 0)));
            if(bReady)
            {
                stkOps.pop();
                Float_ leftChildField;
                Float_ rightChildField;

                if(lChildIsOp)
                    leftChildField = Float_(&arrOpFields[idxLC * PS_SIMD_FLEN]);
                else
                {
                    computePrimitiveField(pX, pY, pZ, leftChildField, idxLC);
                    leftChildField.store(&arrPrimFields[idxLC * PS_SIMD_FLEN]);
                }

                if(rChildIsOp)
                    rightChildField = Float_(&arrOpFields[idxRC * PS_SIMD_FLEN]);
                else
                {
                    computePrimitiveField(pX, pY, pZ, rightChildField, idxRC);
                    rightChildField.store(&arrPrimFields[idxRC * PS_SIMD_FLEN]);
                }

                opType = m_blobOps.opType[idxOp];
                switch(opType)
                {
                case(opBlend):
                {
                    outField = leftChildField + rightChildField;
                }
                    break;
                case(opRicciBlend):
                {
                    Float_ ricciPower = Float_(m_blobOps.resY[idxOp]);
                    outField = SimdPower(leftChildField + rightChildField, ricciPower);
                }
                    break;
                case(opUnion):
                {
                    outField = SimdMax(leftChildField, rightChildField);
                }
                    break;
                case(opIntersect):
                {
                    outField = SimdMin(leftChildField, rightChildField);
                }
                    break;
                case(opDif):
                {
                    Float_ maxField(1.0f);
                    outField = SimdMin(leftChildField, maxField - rightChildField);
                }
                    break;
                case(opSmoothDif):
                {
                    Float_ maxField(1.0f);
                    outField = leftChildField * (maxField - rightChildField);
                }
                    break;

                case(opWarpBend):
                {
                    outField = leftChildField;
                }
                    break;
                case(opWarpTwist):
                {
                    outField = leftChildField;
                }
                    break;
                case(opWarpTaper):
                {
                    outField = leftChildField;
                }
                    break;
                case(opWarpShear):
                {
                    outField = leftChildField;
                }
                    break;
                }

                //Store Op Field
                arrOpsFieldComputed[idxOp] = 1;
                outField.store(&arrOpFields[idxOp * PS_SIMD_FLEN]);
            }
            else
            {
                //If lchild is op and not processed
                if(lChildIsOp && (arrOpsFieldComputed[idxLC] == 0))
                    stkOps.push(depth + 1, idxLC);

                //If rchild is op and not processed
                if(rChildIsOp && (arrOpsFieldComputed[idxRC] == 0))
                    stkOps.push(depth + 1, idxRC);
            }
        }
    }
    else
    {
        //NO Op - Just compute all primitive fields and Blend them
        Float_ curPrimField;
        for(U32 i=0; i<m_blobPrims.ctPrims; i++)
        {
            computePrimitiveField(pX, pY, pZ, curPrimField, i);
            outField = outField + curPrimField;

            //Store Fields
            curPrimField.store(&arrPrimFields[i * PS_SIMD_FLEN]);
        }
    }

    if(lpOutFieldPrims)
        memcpy(lpOutFieldPrims, arrPrimFields, szPrimFieldArrSize * sizeof(float));
    if(lpOutFieldOps)
        memcpy(lpOutFieldOps, arrOpFields, szOpFieldArrSize * sizeof(float));

    return 1;
}

int FieldComputer::fieldValueAndColor(const Float_& pX, const Float_& pY, const Float_& pZ,
                                      Float_& outField,	Float_& outColorX,
                                      Float_& outColorY, Float_& outColorZ) const
{
    //const U32 szPrimFieldArrSize = PS_SIMD_PADSIZE(m_blobPrims.ctPrims * PS_SIMD_FLEN);
    //const U32 szOpFieldArrSize   = PS_SIMD_PADSIZE(m_blobOps.ctOps * PS_SIMD_FLEN);

    float PS_SIMD_ALIGN(arrPrimFields[m_szConstFieldPadded]);
    float PS_SIMD_ALIGN(arrOpFields[m_szConstFieldPadded]);

    outColorX.setZero();
    outColorY.setZero();
    outColorZ.setZero();

    //The only Field Evaluation we do
    fieldValue(pX, pY, pZ, outField, arrPrimFields, arrOpFields);

    //Evaluate Operators
    if(m_blobOps.ctOps > 0)
    {
        float PS_SIMD_ALIGN(arrOpColorX[m_szConstFieldPadded]);
        float PS_SIMD_ALIGN(arrOpColorY[m_szConstFieldPadded]);
        float PS_SIMD_ALIGN(arrOpColorZ[m_szConstFieldPadded]);

        U8 arrOpsColorComputed[MAX_TREE_NODES];
        memset(arrOpsColorComputed, 0, MAX_TREE_NODES);
        //U8 arrOpsColorComputed[m_blobOps.ctOps];
        //memset(arrOpsColorComputed, 0, m_blobOps.ctOps);

        SIMPLESTACK<MAX_TREE_NODES> stkOps;
        stkOps.push(0);

        U16 idxOp, idxLC, idxRC;
        U8 childKind, opType;
        U8 lChildIsOp, rChildIsOp, bReady;

        while(!stkOps.empty())
        {
            idxOp = stkOps.id[stkOps.idxTop];
            idxLC = m_blobOps.opLeftChild[idxOp];
            idxRC = m_blobOps.opRightChild[idxOp];
            childKind = m_blobOps.opChildKind[idxOp];
            lChildIsOp = (childKind & 2) >> 1;
            rChildIsOp = (childKind & 1);

            bReady = !((lChildIsOp && (arrOpsColorComputed[idxLC] == 0))||
                       (rChildIsOp && (arrOpsColorComputed[idxRC] == 0)));
            if(bReady)
            {
                stkOps.pop();

                Float_ curOpField;
                Float_ leftChildField;
                Float_ rightChildField;
                Float_ leftChildColorX;
                Float_ leftChildColorY;
                Float_ leftChildColorZ;
                Float_ rightChildColorX;
                Float_ rightChildColorY;
                Float_ rightChildColorZ;


                curOpField = Float_(&arrOpFields[idxOp * PS_SIMD_FLEN]);

                if(lChildIsOp)
                {
                    leftChildField = Float_(&arrOpFields[idxLC * PS_SIMD_FLEN]);
                    leftChildColorX = arrOpColorX[idxLC * PS_SIMD_FLEN];
                    leftChildColorY = arrOpColorY[idxLC * PS_SIMD_FLEN];
                    leftChildColorZ = arrOpColorZ[idxLC * PS_SIMD_FLEN];
                }
                else
                {
                    leftChildField = Float_(&arrPrimFields[idxLC * PS_SIMD_FLEN]);
                    leftChildColorX = Float_(m_blobPrims.colorX[idxLC]);
                    leftChildColorY = Float_(m_blobPrims.colorY[idxLC]);
                    leftChildColorZ = Float_(m_blobPrims.colorZ[idxLC]);
                }

                if(rChildIsOp)
                {
                    rightChildField  = Float_(&arrOpFields[idxRC * PS_SIMD_FLEN]);
                    rightChildColorX = arrOpColorX[idxRC * PS_SIMD_FLEN];
                    rightChildColorY = arrOpColorY[idxRC * PS_SIMD_FLEN];
                    rightChildColorZ = arrOpColorZ[idxRC * PS_SIMD_FLEN];
                }
                else
                {
                    rightChildField = Float_(&arrPrimFields[idxRC * PS_SIMD_FLEN]);
                    rightChildColorX = Float_(m_blobPrims.colorX[idxRC]);
                    rightChildColorY = Float_(m_blobPrims.colorY[idxRC]);
                    rightChildColorZ = Float_(m_blobPrims.colorZ[idxRC]);
                }

                opType = m_blobOps.opType[idxOp];
                switch(opType)
                {
                case(opBlend): case(opRicciBlend):
                {
                    Float_ half(0.5f);
                    Float_ one(1.0f);
                    Float_ two(2.0f);

                    leftChildField  = two * (half + leftChildField) - one;
                    rightChildField = two * (half + rightChildField) - one;

                    outColorX = leftChildField * leftChildColorX + rightChildField * rightChildColorX;
                    outColorY = leftChildField * leftChildColorY + rightChildField * rightChildColorY;
                    outColorZ = leftChildField * leftChildColorZ + rightChildField * rightChildColorZ;
                }
                    break;
                case(opUnion): case(opIntersect):
                {
                    Float_ zero(0.0f);
                    Float_ one(1.0f);

                    leftChildField  = SimdAnd(SimdEQ(curOpField - leftChildField, zero), one);
                    rightChildField = SimdAnd(SimdEQ(curOpField - rightChildField, zero), one);

                    outColorX = leftChildField * leftChildColorX + rightChildField * rightChildColorX;
                    outColorY = leftChildField * leftChildColorY + rightChildField * rightChildColorY;
                    outColorZ = leftChildField * leftChildColorZ + rightChildField * rightChildColorZ;
                }
                    break;
                case(opDif): case(opSmoothDif):
                {
                    Float_ one(1.0f);

                    leftChildField  = SimdAnd(SimdEQ(leftChildField, curOpField), one);
                    rightChildField = SimdAnd(SimdEQ(one - rightChildField, curOpField), one);

                    outColorX = leftChildField * leftChildColorX + rightChildField * rightChildColorX;
                    outColorY = leftChildField * leftChildColorY + rightChildField * rightChildColorY;
                    outColorZ = leftChildField * leftChildColorZ + rightChildField * rightChildColorZ;
                }
                    break;

                case(opWarpBend):case(opWarpTwist):case(opWarpTaper):case(opWarpShear):
                {
                    outColorX = leftChildColorX;
                    outColorY = leftChildColorY;
                    outColorZ = leftChildColorZ;
                }
                    break;
                }

                //Set Compute Flag and store fields
                arrOpsColorComputed[idxOp] = 1;
                outColorX.store(&arrOpColorX[idxOp * PS_SIMD_FLEN]);
                outColorY.store(&arrOpColorY[idxOp * PS_SIMD_FLEN]);
                outColorZ.store(&arrOpColorZ[idxOp * PS_SIMD_FLEN]);
            }
            else
            {
                //If lchild is op and not processed
                if(lChildIsOp && (arrOpsColorComputed[idxLC] == 0))
                    stkOps.push(idxLC);

                //If rchild is op and not processed
                if(rChildIsOp && (arrOpsColorComputed[idxRC] == 0))
                    stkOps.push(idxRC);
            }
        }
    }
    else
    {
        outColorX = Float_(m_blobPrims.colorX[0]);
        outColorY = Float_(m_blobPrims.colorY[0]);
        outColorZ = Float_(m_blobPrims.colorZ[0]);
    }


    return 1;
}

int FieldComputer::fieldValueAndGradient(const Float_& pX, const Float_& pY, const Float_& pZ,
                                         Float_& outField, Float_& outGradX, Float_& outGradY, Float_& outGradZ,
                                         float delta) const
{
    fieldValue(pX, pY, pZ, outField);

    Float_ delta_(delta);
    Float_ deltaInv_(1.0f / delta);
    Float_ divX;
    Float_ divY;
    Float_ divZ;

    fieldValue(pX + delta, pY, pZ, divX);
    fieldValue(pX, pY + delta, pZ, divY);
    fieldValue(pX, pY, pZ + delta, divZ);

    outGradX = (divX - outField) * deltaInv_;
    outGradY = (divY - outField) * deltaInv_;
    outGradZ = (divZ - outField) * deltaInv_;

    return 4;
}

int FieldComputer::gradient(const Float_& pX, const Float_& pY, const Float_& pZ,
                            const Float_& inFieldValue,
                            Float_& outGradX, Float_& outGradY, Float_& outGradZ,
                            float delta) const
{
    Float_ delta_(delta);
    Float_ deltaInv_(1.0f / delta);
    Float_ divX;
    Float_ divY;
    Float_ divZ;

    fieldValue(pX + delta_, pY, pZ, divX);
    fieldValue(pX, pY + delta_, pZ, divY);
    fieldValue(pX, pY, pZ + delta, divZ);

    outGradX = (divX - inFieldValue) * deltaInv_;
    outGradY = (divY - inFieldValue) * deltaInv_;
    outGradZ = (divZ - inFieldValue) * deltaInv_;

    return 3;
}

int FieldComputer::normal(const Float_& pX,
                          const Float_& pY,
                          const Float_& pZ,
                          const Float_& inFieldValue,
                          Float_& outNormalX, Float_& outNormalY, Float_& outNormalZ,
                          float delta) const
{
    Float_ delta_(delta);
    Float_ deltaInv_(-1.0f / delta);
    Float_ divX;
    Float_ divY;
    Float_ divZ;

    fieldValue(pX + delta_, pY, pZ, divX);
    fieldValue(pX, pY + delta_, pZ, divY);
    fieldValue(pX, pY, pZ + delta, divZ);

    outNormalX = (divX - inFieldValue) * deltaInv_;
    outNormalY = (divY - inFieldValue) * deltaInv_;
    outNormalZ = (divZ - inFieldValue) * deltaInv_;

    SimdNormalize(outNormalX, outNormalY, outNormalZ);

    return 3;
}


int FieldComputer::computeRootNewtonRaphson(const svec3f& p1, const svec3f& p2,
                                            float fp1, float fp2,
                                            svec3f& output, float& outputField,
                                            float target_field, int iterations) const
{
    svec3f grad;
    Float_ rootX, rootY, rootZ;
    Float_ gradX, gradY, gradZ;
    Float_ field;
    //Float_ d;
    Float_ g;


    if(iterations <= 0) return -1;

    if(Absolutef(fp1 - target_field) < Absolutef(fp2 - target_field))
    {
        rootX = Float_(p1.x);
        rootY = Float_(p1.y);
        rootZ = Float_(p1.z);
        field = Float_(fp1);
    }
    else
    {
        rootX = Float_(p2.x);
        rootY = Float_(p2.y);
        rootZ = Float_(p2.z);
        field = Float_(fp2);
    }

    int i=0;
    for(i=0; i<iterations; i++)
    {
        //Use faster method to compute gradient at once
        gradient(rootX, rootY, rootZ, field, gradX, gradY, gradZ, FIELDVALUE_EPSILON);

        //Uses shrink-wrap method to converge to surface
        g = (Float_(target_field) - field) / (gradX * gradX + gradY * gradY + gradZ * gradZ);
        rootX = rootX + gradX * g;
        rootY = rootY + gradY * g;
        rootZ = rootZ + gradZ * g;

        //x = x + (d*grad) * g;
        fieldValue(rootX, rootY, rootZ, field);

        outputField = field[0];
        //outputField = cptblobPrims->fieldvalue(x, lpStoreFVOps, lpStoreFVPrims);
        //output = x;
        if(Absolutef(outputField - target_field) < FIELDVALUE_EPSILON)
            break;
    }

    output = svec3f(rootX[0], rootY[0], rootZ[0]);
    outputField = field[0];

    return (i+1)*4;
}

}
}

