#include "CPolyParsipServer.h"
#include <map>

#include "PS_FrameWork/include/PS_PerfLogger.h"
#include "CParallelAdaptiveSubdivision.h"
#include "PS_BlobTree/include/CRicciBlend.h"
#include "BlobTreeBuilder.h"


#ifdef linux
#include <sys/types.h>
#include <pthread.h>
#endif

#define DEBUGMESH 1

namespace PS{

void renderMPU(CMpu* polygonizerUnit)
{
    polygonizerUnit->run();
}

//**********************************************************************************************
void CMpu::run()
{
    m_statStartTick = CPerfLogger::getPerfCounter();

    //Reset Stat
    m_bReady = false;
    m_bTreeCompacted = false;
    m_statFieldEvaluations = 0;
    m_statIntersectedCells = 0;

#ifdef WIN32
    m_threadID = GetCurrentThreadId();
#else
    m_threadID = pthread_self();
#endif

    //Reset all data structures
    //m_processedEdges.reset();
    outputMesh.removeAll();
    outputMesh.initMesh(CMeshVV::TRIANGLES, 3, 4, 0, 0);

    //Check if we really need to find fieldvalues for all splats in this grid
    float invCellSize = 1.0f / m_cellsize;
    vec3f sides  = m_upperBound - m_origin;
    sides *= invCellSize;

    //Bounds
    m_gridBound.x = MATHMIN((int)m_hashFunc.dim, static_cast<int>(ceil(sides.x)));
    m_gridBound.y = MATHMIN((int)m_hashFunc.dim, static_cast<int>(ceil(sides.y)));
    m_gridBound.z = MATHMIN((int)m_hashFunc.dim, static_cast<int>(ceil(sides.z)));

    //Return immediately if there is no overlap with primitive boxes
    vec3f hi = m_origin + vec3f(m_cellsize * m_gridBound.x, m_cellsize * m_gridBound.y, m_cellsize * m_gridBound.z);
    COctree mpuOct(m_origin, hi);
    bool bIntersectsPrimOctree = false;
    for(size_t i=0; i < m_primitiveLos.size(); i++)
    {
        if(mpuOct.intersect(m_primitiveLos[i], m_primitiveHis[i]))
        {
            bIntersectsPrimOctree = true;
            break;
        }
    }

    if(!bIntersectsPrimOctree)
    {
        m_statEndTick = CPerfLogger::getPerfCounter();
        m_statProcessTime = CPerfLogger::convertTimeTicksToMS(m_statEndTick - m_statStartTick);
        return;
    }

    //Compact BlobTree
    //m_root = CompactBlobTree(m_root);

    //Try to do surface tracking
    m_bDoSurfaceTracking = false;
    if(!m_bForceMC)
    {
        bool bFoundSeedPoint = false;
        vec3f seedPoint;
        for(size_t i=0; i < m_stPrimitiveSeeds.size(); i++)
        {
            if(mpuOct.isInside(m_stPrimitiveSeeds[i]))
            {
                seedPoint = m_stPrimitiveSeeds[i];
                bFoundSeedPoint = true;
                break;
            }
        }

        //Do Surface Tracking
        if(!bFoundSeedPoint) seedPoint = mpuOct.center();

        m_statFieldEvaluations++;
        float fvSeed = m_root->fieldValue(seedPoint);
        vec3f hot, cold;
        float fpHot, fpCold;

        hot    = seedPoint;
        cold   = seedPoint;
        fpHot  = fvSeed;
        fpCold = fvSeed;
        bool bFoundHot = FindSeedPoint(m_root, true, ISO_VALUE, MAX_ATTEMPTS, hot, fpHot, m_statFieldEvaluations);
        bool bFoundCold = FindSeedPoint(m_root, false, ISO_VALUE, MAX_ATTEMPTS, cold, fpCold, m_statFieldEvaluations);
        if(bFoundHot && bFoundCold == false)
        {
            vec3f dir;
            for(int i=0; i<8; i++)
            {
                hot = mpuOct.getCorner(i);
                cold = hot;
                fpHot = m_root->fieldValue(hot);
                fpCold = fpHot;
                dir = hot - seedPoint;
                dir.normalize();

                bFoundHot = FindSeedPoint(m_root, true, ISO_VALUE, MAX_ATTEMPTS, 1.1f, dir, hot, fpHot, m_statFieldEvaluations);
                bFoundCold = FindSeedPoint(m_root, false, ISO_VALUE, MAX_ATTEMPTS, 1.1f, dir, cold, fpCold, m_statFieldEvaluations);
                if(bFoundHot && bFoundCold)
                    break;
            }
        }

        //Now if found then place seedCell and do surface tracking

        if(bFoundHot && bFoundCold)
        {
            m_statFieldEvaluations += ComputeRootBiSection(m_root, hot, cold, fpHot, fpCold, seedPoint, fvSeed);

            //Check if seed-cell is inside
            if(mpuOct.isInside(seedPoint))
            {
                m_stSeedCellCenter = seedPoint;
                m_bDoSurfaceTracking = true;
            }
        }
    }

    //Setup
    m_processedEdges.setup(m_hashFunc.dim);

    if(m_bDoSurfaceTracking)
    {
        m_stkTempCubes.resize(0);
        for(size_t i=0; i<m_stHashTableProcessedCells.size(); i++)
            m_stHashTableProcessedCells[i].clear();
        m_stHashTableProcessedCells.resize(m_hashFunc.cellid_hashsize);

        if(m_grid.bValid == false)
            m_grid.setup(m_hashFunc.dim);

        doSurfaceTracking();
    }
    else
    {
        //Last thing is to do the very costy O(n3) MC
        //Setup grid memory
        if(m_grid.bValid == false)
            m_grid.setup(m_hashFunc.dim);
        doMarchingCubes();
    }


    //Result
    m_bHasSurface = (outputMesh.countFaces() > 0);

    //Perform Adaptive Subdivision Technique
    if(m_bHasSurface && m_bUseAdaptiveSubDivision)
        m_statFieldEvaluations += SubDivide_ParallelPerform(outputMesh, m_root, m_adaptiveParam);
    //subDivide();
    m_bReady = true;

    m_statEndTick = CPerfLogger::getPerfCounter();
    m_statProcessTime = CPerfLogger::convertTimeTicksToMS(m_statEndTick - m_statStartTick);
}

size_t CMpu::subDivide()
{
    vector<int> arrVertexCount;
    vector<int> arrDecisionBits;

    size_t ctNeedsProcessing = 0;
    int attemps = 0;
    while(attemps < 5)
    {
        ctNeedsProcessing = subDivide_Analyze(arrVertexCount, arrDecisionBits);
        if(ctNeedsProcessing > 0)
            m_statFieldEvaluations += subDivide_Perform(arrVertexCount, arrDecisionBits);
        else
            break;
        attemps++;
    }

    return ctNeedsProcessing;
}

size_t CMpu::subDivide_Analyze(vector<int>& arrVertexCount, vector<int>& arrDecisionBits)
{
    size_t ctFaceElements = outputMesh.m_lstFaces.size();
    int ctFaces = ctFaceElements / 3;

    vec3i tri;
    vec3f n[3];
    float alpha[3];
    int decisionbits;

    size_t ctCoarseFaces = 0;
    arrVertexCount.resize(ctFaces);
    arrDecisionBits.resize(ctFaces);

    int ctOutVertices;
    int iFace;

    //OpenMP parallel
    //#pragma omp parallel for
    for(iFace=0; iFace<ctFaces; iFace++)
    {
        //loadControlPoints
        tri.x = outputMesh.m_lstFaces[iFace*3+0];
        tri.y = outputMesh.m_lstFaces[iFace*3+1];
        tri.z = outputMesh.m_lstFaces[iFace*3+2];

        n[0] = outputMesh.getNormal3(tri.x);
        n[1] = outputMesh.getNormal3(tri.y);
        n[2] = outputMesh.getNormal3(tri.z);

        //Compute Normal Angles
        alpha[0] = VAngleDegree(n[0], n[1]);
        alpha[1] = VAngleDegree(n[1], n[2]);
        alpha[2] = VAngleDegree(n[2], n[0]);

        if(n[0] == n[1])	alpha[0] = 0.0f;
        if(n[1] == n[2])	alpha[1] = 0.0f;
        if(n[2] == n[0])	alpha[2] = 0.0f;

        ctOutVertices = 3;
        decisionbits = 0;
        for(int j=0;j<3;j++)
        {
            if(alpha[j] > m_adaptiveParam)
            {
                ctOutVertices++;
                decisionbits = decisionbits | (1 << j);
            }
        }

        if(decisionbits > 0)
            ctCoarseFaces++;

        arrVertexCount[iFace]  = ctOutVertices;
        arrDecisionBits[iFace] = decisionbits;
    }

    //return number of faces to be subdivided
    return ctCoarseFaces;
}

size_t CMpu::subDivide_Perform(vector<int>& arrVertexCount, vector<int>& arrDecisionBits)
{
    //Compute Storage Requirements
    size_t ctTotalVertices = 0;
    size_t ctTotalTriangles = 0;
    size_t ctFieldEvaluations = 0;
    int ctFaces = arrVertexCount.size();
    int iFace;

    vector<int> arrVertexCountScanned;
    arrVertexCountScanned.resize(ctFaces + 1);
    arrVertexCountScanned[0] = 0;
    for(iFace=0; iFace < ctFaces; iFace++)
    {
        ctTotalVertices += arrVertexCount[iFace];
        arrVertexCountScanned[iFace+1] = ctTotalVertices;
    }
    ctTotalTriangles = ctTotalVertices - ctFaces*2;
    ctFieldEvaluations = (ctTotalVertices - ctFaces*3)*8;

    //For Loading Control Points
    vec3i tri;
    vec3f v[6];
    vec3f n[6];
    vec4f c[3];

    //Vertices, Normals, Faces
    vector<float> lstOutVertices;
    vector<float> lstOutNormals;
    vector<float> lstOutColors;
    vector<unsigned int> lstOutFaces;
    int bits;
    size_t offsetV, offsetT;

    //Reserve memory
    lstOutVertices.resize(ctTotalVertices*3);
    lstOutNormals.resize(ctTotalVertices*3);
    lstOutColors.resize(ctTotalVertices*4);
    lstOutFaces.resize(ctTotalTriangles*3);

    //Process face by face
    //ToDo: Make it Parallel_For
    //#pragma omp parallel for
    for(iFace=0; iFace < ctFaces; iFace++)
    {
        //loadControlPoints
        tri.x = outputMesh.m_lstFaces[iFace*3+0];
        tri.y = outputMesh.m_lstFaces[iFace*3+1];
        tri.z = outputMesh.m_lstFaces[iFace*3+2];

        v[0] = outputMesh.getVertex3(tri.x);
        v[1] = outputMesh.getVertex3(tri.y);
        v[2] = outputMesh.getVertex3(tri.z);

        n[0] = outputMesh.getNormal3(tri.x);
        n[1] = outputMesh.getNormal3(tri.y);
        n[2] = outputMesh.getNormal3(tri.z);

        c[0] = vec4f(outputMesh.getColor(tri.x));
        c[1] = vec4f(outputMesh.getColor(tri.y));
        c[2] = vec4f(outputMesh.getColor(tri.z));

        //Vertices:
        //First output original control points
        offsetV = arrVertexCountScanned[iFace];
        for(int i=0; i<3; i++)
        {
            lstOutVertices[(offsetV + i)*3+0] = v[i].x;
            lstOutVertices[(offsetV + i)*3+1] = v[i].y;
            lstOutVertices[(offsetV + i)*3+2] = v[i].z;

            lstOutNormals[(offsetV + i)*3+0] = n[i].x;
            lstOutNormals[(offsetV + i)*3+1] = n[i].y;
            lstOutNormals[(offsetV + i)*3+2] = n[i].z;

            lstOutColors[(offsetV + i)*4+0] = c[i].x;
            lstOutColors[(offsetV + i)*4+1] = c[i].y;
            lstOutColors[(offsetV + i)*4+2] = c[i].z;
            lstOutColors[(offsetV + i)*4+3] = c[i].w;
        }

        //Add MidPoints to list of Points
        offsetV += 3;
        bits = arrDecisionBits[iFace];

        vec3f mid, midNormal;
        vec4f midColor;
        int incr = 0;
        if(bits != 0)
        {
            for(int i=0; i<3; i++)
            {
                if((bits & (1 << i)) != 0)
                {
                    mid = (v[i] + v[(i+1)%3]) * 0.5f;
                    subDivide_MoveMidPointToSurface(mid, midNormal, midColor);

                    lstOutVertices[offsetV*3 + 0] = mid.x;
                    lstOutVertices[offsetV*3 + 1] = mid.y;
                    lstOutVertices[offsetV*3 + 2] = mid.z;

                    lstOutNormals[offsetV*3 + 0] = midNormal.x;
                    lstOutNormals[offsetV*3 + 1] = midNormal.y;
                    lstOutNormals[offsetV*3 + 2] = midNormal.z;

                    lstOutColors[offsetV*4 + 0] = midColor.x;
                    lstOutColors[offsetV*4 + 1] = midColor.y;
                    lstOutColors[offsetV*4 + 2] = midColor.z;
                    lstOutColors[offsetV*4 + 3] = midColor.w;

                    v[3 + incr] = mid;
                    n[3 + incr] = midNormal;

                    incr++;
                    offsetV++;
                }
            }
        }


        //Faces:
        int nTris = arrVertexCount[iFace] - 2;
        offsetV = arrVertexCountScanned[iFace];
        offsetT = arrVertexCountScanned[iFace] - (iFace * 2);
        if(nTris == 1)
        {
            for(int i=0; i<3; i++)
                lstOutFaces[offsetT*3 + i] = offsetV + i;
        }
        else if(nTris == 2)
        {
            if(bits == 1)
            {
                lstOutFaces[offsetT*3 + 0] = offsetV + 3;
                lstOutFaces[offsetT*3 + 1] = offsetV + 2;
                lstOutFaces[offsetT*3 + 2] = offsetV + 0;
                offsetT++;
                lstOutFaces[offsetT*3 + 0] = offsetV + 1;
                lstOutFaces[offsetT*3 + 1] = offsetV + 2;
                lstOutFaces[offsetT*3 + 2] = offsetV + 3;
            }
            else if(bits == 2)
            {
                lstOutFaces[offsetT*3 + 0] = offsetV + 0;
                lstOutFaces[offsetT*3 + 1] = offsetV + 3;
                lstOutFaces[offsetT*3 + 2] = offsetV + 2;
                offsetT++;
                lstOutFaces[offsetT*3 + 0] = offsetV + 1;
                lstOutFaces[offsetT*3 + 1] = offsetV + 3;
                lstOutFaces[offsetT*3 + 2] = offsetV + 0;
            }
            else if(bits == 4)
            {
                lstOutFaces[offsetT*3 + 0] = offsetV + 0;
                lstOutFaces[offsetT*3 + 1] = offsetV + 1;
                lstOutFaces[offsetT*3 + 2] = offsetV + 3;
                offsetT++;
                lstOutFaces[offsetT*3 + 0] = offsetV + 1;
                lstOutFaces[offsetT*3 + 1] = offsetV + 2;
                lstOutFaces[offsetT*3 + 2] = offsetV + 3;
            }
        }
        else if(nTris == 3)
        {
            if(bits == 3)
            {
                lstOutFaces[offsetT*3 + 0] = offsetV + 1;
                lstOutFaces[offsetT*3 + 1] = offsetV + 4;
                lstOutFaces[offsetT*3 + 2] = offsetV + 3;
                offsetT++;
                if(v[4].dist2(v[0]) < v[3].dist2(v[2]))
                {
                    lstOutFaces[offsetT*3 + 0] = offsetV + 3;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 4;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 0;
                    offsetT++;
                    lstOutFaces[offsetT*3 + 0] = offsetV + 0;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 4;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 2;
                }
                else
                {
                    lstOutFaces[offsetT*3 + 0] = offsetV + 3;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 2;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 0;
                    offsetT++;
                    lstOutFaces[offsetT*3 + 0] = offsetV + 3;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 4;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 2;
                }
            }
            else if(bits == 5)
            {
                lstOutFaces[offsetT*3 + 0] = offsetV + 0;
                lstOutFaces[offsetT*3 + 1] = offsetV + 3;
                lstOutFaces[offsetT*3 + 2] = offsetV + 4;
                offsetT++;
                if(v[3].dist2(v[2]) < v[4].dist2(v[1]))
                {
                    lstOutFaces[offsetT*3 + 0] = offsetV + 1;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 2;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 3;
                    offsetT++;
                    lstOutFaces[offsetT*3 + 0] = offsetV + 3;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 2;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 4;
                }
                else
                {
                    lstOutFaces[offsetT*3 + 0] = offsetV + 1;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 4;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 3;
                    offsetT++;
                    lstOutFaces[offsetT*3 + 0] = offsetV + 1;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 2;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 4;
                }

            }
            else if(bits == 6)
            {
                lstOutFaces[offsetT*3 + 0] = offsetV + 4;
                lstOutFaces[offsetT*3 + 1] = offsetV + 3;
                lstOutFaces[offsetT*3 + 2] = offsetV + 2;
                offsetT++;
                if(v[0].dist2(v[3]) < v[1].dist2(v[4]))
                {
                    lstOutFaces[offsetT*3 + 0] = offsetV + 1;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 3;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 0;
                    offsetT++;
                    lstOutFaces[offsetT*3 + 0] = offsetV + 3;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 4;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 0;
                }
                else
                {
                    lstOutFaces[offsetT*3 + 0] = offsetV + 1;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 4;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 0;
                    offsetT++;
                    lstOutFaces[offsetT*3 + 0] = offsetV + 1;
                    lstOutFaces[offsetT*3 + 1] = offsetV + 3;
                    lstOutFaces[offsetT*3 + 2] = offsetV + 4;
                }
            }
        }
        else if(nTris == 4)
        {
            lstOutFaces[offsetT*3 + 0] = offsetV + 0;
            lstOutFaces[offsetT*3 + 1] = offsetV + 3;
            lstOutFaces[offsetT*3 + 2] = offsetV + 5;
            offsetT++;
            lstOutFaces[offsetT*3 + 0] = offsetV + 3;
            lstOutFaces[offsetT*3 + 1] = offsetV + 1;
            lstOutFaces[offsetT*3 + 2] = offsetV + 4;
            offsetT++;
            lstOutFaces[offsetT*3 + 0] = offsetV + 3;
            lstOutFaces[offsetT*3 + 1] = offsetV + 4;
            lstOutFaces[offsetT*3 + 2] = offsetV + 5;
            offsetT++;
            lstOutFaces[offsetT*3 + 0] = offsetV + 5;
            lstOutFaces[offsetT*3 + 1] = offsetV + 4;
            lstOutFaces[offsetT*3 + 2] = offsetV + 2;
        }
    }


    //Copy to mesh structure
    outputMesh.m_lstFaces.assign(lstOutFaces.begin(), lstOutFaces.end());
    outputMesh.m_lstVertices.assign(lstOutVertices.begin(), lstOutVertices.end());
    outputMesh.m_lstNormals.assign(lstOutNormals.begin(), lstOutNormals.end());
    outputMesh.m_lstColors.assign(lstOutColors.begin(), lstOutColors.end());

    //Cleanup
    lstOutFaces.clear();
    lstOutVertices.clear();
    lstOutColors.clear();
    lstOutNormals.clear();
    arrDecisionBits.clear();
    arrVertexCount.clear();
    arrVertexCountScanned.clear();

    return ctFieldEvaluations;
}

int CMpu::subDivide_MoveMidPointToSurface( vec3f& m, vec3f& outputNormal, vec4f& outputColor, float target_field /*= ISO_VALUE*/, int nIterations /*= DEFAULT_ITERATIONS */ )
{
    vec3f grad;
    float d;

    //Compute Initial FieldValue
    float f = m_root->fieldValue(m);

    int i;
    for(i=0; i<nIterations; i++)
    {
        //Use faster method to compute fieldvalue and gradient at once
        //m_root->fieldValueAndGradient(m, FIELD_VALUE_EPSILON, grad, f);

        //New Method
        grad = m_root->gradient(m, FIELD_VALUE_EPSILON, f);

        //Compute Distance
        d = (target_field - f);

        //Move Point to new position. Uses shrink-wrap method to converge to surface
        m = m + ((d*grad)/grad.dot(grad));

        //New Field
        f = m_root->fieldValue(m);
        d = fabsf(target_field - f);
        if(d < FIELD_VALUE_EPSILON)
            break;
    }

    outputNormal = m_root->normal(m, f, NORMAL_DELTA);
    outputColor = m_root->getMaterial().diffused;

    return (i+2)*4;
}

void CMpu::compactBlobTree_Intersect(const COctree& mpuOct, CBlobNode* lpNode)
{
    if(lpNode->isOperator())
    {
        int ctCrossed = 0;
        bool bChildCrosses;
        for(size_t i=0;i<lpNode->countChildren(); i++)
        {
            compactBlobTree_Intersect(mpuOct, lpNode->getChild(i));
            bChildCrosses = (lpNode->getChild(i)->getID() == CROSS_ALL);
            if(bChildCrosses) ctCrossed++;
        }

        //lpNode->setID(bCrosses?CROSS_ALL:CROSS_NONE);
        lpNode->setID(ctCrossed);
    }
    else
    {
        COctree oct = lpNode->getOctree();
        if(mpuOct.intersect(oct.lower, oct.upper))
            lpNode->setID(CROSS_ALL);
        else
            lpNode->setID(CROSS_NONE);
    }
}

int CMpu::compactBlobTree_Recursive( CBlobNode* lpInput, stack<CBlobNode*>& stkOperators, CBlobNode*& lpOutClone)
{
    CBlobNode* clonned = lpOutClone;
    if(lpInput->isOperator())
    {
        if(lpInput->getID() != CROSS_NONE)
        {
            clonned = TheBlobNodeCloneFactory::Instance().CreateObject(lpInput);
            if(lpOutClone == NULL)
                lpOutClone = clonned;
            else
                lpOutClone->addChild(clonned);

            stkOperators.push(lpInput);

            size_t ctKids = lpInput->countChildren();
            int res = 0;
            for(size_t i=0; i<ctKids; i++)
            {
                stack<CBlobNode*> stkLocal(stkOperators);
                //Call next round
                res += compactBlobTree_Recursive(lpInput->getChild(i), stkLocal, clonned);
            }
        }
    }
    else
    {
        if(lpInput->getID() != CROSS_NONE)
        {
            clonned = TheBlobNodeCloneFactory::Instance().CreateObject(lpInput);
            if(lpOutClone == NULL)
                lpOutClone = clonned;
            else
                lpOutClone->addChild(clonned);
            return 1;
        }
    }

    return 0;
}

// testface: given cube at lattice (i, j, k), and four corners of face,
// if surface crosses face, compute other four corners of adjacent cube
// and add new cube to cube stack
bool CMpu::stQueryCellFace(const MPUCELL& old, int i, int j, int k,
                           int face, int c1, int c2, int c3, int c4)
{
    static int facebit[6] = {2, 2, 1, 1, 0, 0};
    int n;
    int bit = facebit[face];
    int pos = old.corners[c1].value > ISO_VALUE ? 1 : 0;


    //test if no surface crossing, cube out of bounds, or already visited:
    if ((old.corners[c2].value > ISO_VALUE) == pos &&
            (old.corners[c3].value > ISO_VALUE) == pos &&
            (old.corners[c4].value > ISO_VALUE) == pos)
        return false;

    //If cell is out of bounds of this MPU grid then return
    if ((i >= (m_gridBound.x - 1)) ||
            (j >= (m_gridBound.y - 1)) ||
            (k >= (m_gridBound.z - 1)))
        return false;

    if (i < 0 || j < 0 || k < 0)
        return false;

    //If already processed then return immediately
    if (stSetProcessedCell(i, j, k))
        return false;

    // create new_obj cube:
    MPUCELL adjacent;
    int set[8];
    adjacent.i = i;
    adjacent.j = j;
    adjacent.k = k;

    for (n = 0; n < 8; n++)
        set[n] = 0;

    adjacent.corners[FLIP(c1, bit)] = old.corners[c1];
    adjacent.corners[FLIP(c2, bit)] = old.corners[c2];
    adjacent.corners[FLIP(c3, bit)] = old.corners[c3];
    adjacent.corners[FLIP(c4, bit)] = old.corners[c4];
    set[FLIP(c1, bit)] = 1;
    set[FLIP(c2, bit)] = 1;
    set[FLIP(c3, bit)] = 1;
    set[FLIP(c4, bit)] = 1;

    for (n = 0; n < 8; n++)
    {
        //if (adjacent->corners[n] == NULL)
        if(set[n] == 0)
            adjacent.corners[n] = stSetCorner(i+BIT(n,2), j+BIT(n,1), k+BIT(n,0));
    }

    m_stkTempCubes.push_back(adjacent);
    return true;
}

// setCorner: return corner with the given lattice location
// set (and cache) its function value
MPUCORNER CMpu::stSetCorner (int i, int j, int k)
{
    MPUCORNER c;

    //Fill corner with index in lattice
    c.i = i;
    c.j = j;
    c.k = k;

    //position of the corner in 3D
    c.pos.x = static_cast<float>(m_origin.x + (float)i * m_cellsize);
    c.pos.y = static_cast<float>(m_origin.y + (float)j * m_cellsize);
    c.pos.z = static_cast<float>(m_origin.z + (float)k * m_cellsize);

    //Compute Hash Signature
    int index = m_hashFunc.hash(i, j, k);

    //Get Field From Grid
    float fp = m_grid.pValues[index];

    //If not set then evaluate
    if(fp == FLT_MIN)
    {
        m_statFieldEvaluations++;
        fp = m_root->fieldValue(c.pos);
        m_grid.pValues[index] = fp;
    }

    c.value = fp;
    return c;
}

bool CMpu::stSetProcessedCell(int i, int j, int k)
{
    int index = m_hashFunc.hash(i,j,k);
    size_t ctCenters = m_stHashTableProcessedCells[index].size();
    CENTERELEMENT element;

    for (size_t iCenter=0; iCenter < ctCenters; iCenter++)
    {
        element = m_stHashTableProcessedCells[index][iCenter];
        if(element.i == i && element.j == j && element.k == k)
            return true;
    }

    element.i = i;
    element.j = j;
    element.k = k;

    m_stHashTableProcessedCells[index].push_back(element);
    return false;
}


void CMpu::cleanup()
{
    if(m_bTreeCompacted)
        SAFE_DELETE(m_root);
    m_grid.clearAll();
    m_processedEdges.clearAll();
    outputMesh.removeAll();

    m_primitiveHis.clear();
    m_primitiveLos.clear();
    m_stPrimitiveSeeds.clear();
    m_stkTempCubes.resize(0);

    for(size_t i=0; i<m_stHashTableProcessedCells.size(); i++)
        m_stHashTableProcessedCells[i].clear();
    m_stHashTableProcessedCells.clear();
}

bool CMpu::doSurfaceTracking()
{
    vec3f pos = m_stSeedCellCenter - m_origin;
    vec3i idx = vec3i(floorf(pos.x / m_cellsize), floorf(pos.y / m_cellsize), floorf(pos.z / m_cellsize));

    //push initial cube on stack:
    MPUCELL rootCell;
    rootCell.i = idx.x;
    rootCell.j = idx.y;
    rootCell.k = idx.z;
    for (int n = 0; n < 8; n++)
        rootCell.corners[n] = stSetCorner(idx.x + BIT(n,2), idx.y + BIT(n,1), idx.z + BIT(n,0));
    m_stkTempCubes.push_back(rootCell);

    //stSetProcessedCell(0, 0, 0);

    //set corners of initial cube:
    stSetProcessedCell(idx.x, idx.y, idx.z);

    //vector<CELL*> lstCells;
    //PASS 1: Tracking all the cubes on the surface
    while (m_stkTempCubes.size() > 0)
    {
        //Process Cube Faces and find nearby cubes
        MPUCELL cell = m_stkTempCubes.back();

        //Remove front item from stack
        m_stkTempCubes.pop_back();

        //test six faces then add to stack
        //Left face
        stQueryCellFace(cell, cell.i-1, cell.j, cell.k, L, LBN, LBF, LTN, LTF);

        //Right face
        stQueryCellFace(cell, cell.i+1, cell.j, cell.k, R, RBN, RBF, RTN, RTF);

        //Bottom face
        stQueryCellFace(cell, cell.i, cell.j-1, cell.k, B, LBN, LBF, RBN, RBF);

        //Top face
        stQueryCellFace(cell, cell.i, cell.j+1, cell.k, T, LTN, LTF, RTN, RTF);

        //Near face
        stQueryCellFace(cell, cell.i, cell.j, cell.k-1, N, LBN, LTN, RBN, RTN);

        //Far face
        stQueryCellFace(cell, cell.i, cell.j, cell.k+1, F, LBF, LTF, RBF, RTF);

        //Add to final Cubes
        //lstCells.push_back(c);
        stProcessCell(cell);
    }

    return false;
}

bool CMpu::doMarchingCubes()
{
    float fp;
    size_t ctInside = 0;
    vec3i bounds = m_gridBound;

    //Yi Zhang Optimization method
    //float halfDiagonal = 0.866f * m_cellsize;
    //vec3f center;
    //float d;
    //const float oneThird = 1.0f / 3.0f;
    //const float isoDist = sqrt(1 - pow(ISO_VALUE, oneThird));
    //const float asp = isoDist / ISO_VALUE;
    //const float FieldHalfDiag =

    //Now we need to extract cubes
    int	  cellCornerKey[8];
    vec3f cellCornerPos[8];
    vec3i cellCornerIDX[8];
    float cellCornerFields[8];

    int icase;
    int idxCellConfig;

    //Process all cubes
    for(int i=0; i<bounds.x-1; i++)
    {
        for(int j=0; j<bounds.y-1; j++)
        {
            for(int k=0; k<bounds.z-1; k++)
            {
                //Yi Zhang Optimization method
                //First ignore cell if it is not in the ring region
                /*
                                        center = m_origin + m_cellsize * vec3f(static_cast<float>(i) + 0.5f,
                                                                                                                   static_cast<float>(j) + 0.5f,
                                                                                                                   static_cast<float>(k) + 0.5f);

                                        //Compute field
                                        m_statFieldEvaluations++;
                                        fp = m_root->fieldValue(center);
                                        d = fieldToDistance(fp);
                                        if(d > halfDiagonal)
                                                continue;
                                        */

                cellCornerKey[0] = m_hashFunc.hash(i, j, k);
                cellCornerIDX[0] = vec3i(i, j, k);
                cellCornerPos[0] = m_origin + vec3f(m_cellsize*i, m_cellsize*j, m_cellsize*k);

                cellCornerKey[1] = m_hashFunc.hash(i, j, k+1);
                cellCornerIDX[1] = vec3i(i, j, k+1);
                cellCornerPos[1] = m_origin + vec3f(m_cellsize*i, m_cellsize*j, m_cellsize*(k+1));

                cellCornerKey[2] = m_hashFunc.hash(i, j+1, k);
                cellCornerIDX[2] = vec3i(i, j+1, k);
                cellCornerPos[2] = m_origin + vec3f(m_cellsize*i, m_cellsize*(j+1), m_cellsize*k);

                cellCornerKey[3] = m_hashFunc.hash(i, j+1, k+1);
                cellCornerIDX[3] = vec3i(i, j+1, k+1);
                cellCornerPos[3] = m_origin + vec3f(m_cellsize*i, m_cellsize*(j+1), m_cellsize*(k+1));

                cellCornerKey[4] = m_hashFunc.hash(i+1, j, k);
                cellCornerIDX[4] = vec3i(i+1, j, k);
                cellCornerPos[4] = m_origin + vec3f(m_cellsize*(i+1), m_cellsize*j, m_cellsize*k);

                cellCornerKey[5] = m_hashFunc.hash(i+1, j, k+1);
                cellCornerIDX[5] = vec3i(i+1, j, k+1);
                cellCornerPos[5] = m_origin + vec3f(m_cellsize*(i+1), m_cellsize*j, m_cellsize*(k+1));

                cellCornerKey[6] = m_hashFunc.hash(i+1, j+1, k);
                cellCornerIDX[6] = vec3i(i+1, j+1, k);
                cellCornerPos[6] = m_origin + vec3f(m_cellsize*(i+1), m_cellsize*(j+1), m_cellsize*k);

                cellCornerKey[7] = m_hashFunc.hash(i+1, j+1, k+1);
                cellCornerIDX[7] = vec3i(i+1, j+1, k+1);
                cellCornerPos[7] = m_origin + vec3f(m_cellsize*(i+1), m_cellsize*(j+1), m_cellsize*(k+1));


                //Get the field for the cube from cache
                //Find cube-case
                idxCellConfig = 0;
                for(icase=0; icase<8; icase++)
                {
                    fp = m_grid.pValues[cellCornerKey[icase]];
                    //fp = m_grid.getValue(cellCornerIDX[icase]);

                    //If not set then evaluate
                    if(fp == FLT_MIN)
                    {
                        m_statFieldEvaluations++;
                        fp = m_root->fieldValue(cellCornerPos[icase]);
                        m_grid.pValues[cellCornerKey[icase]] = fp;
                        //m_grid.setValue(cellCornerIDX[icase], fp);
                    }

                    cellCornerFields[icase] = fp;
                    if(fp > ISO_VALUE)
                        idxCellConfig += (1 << icase);
                }

                if((idxCellConfig != 0)&&(idxCellConfig != 255))
                {
                    //Increment number of processed cells
                    m_statIntersectedCells++;

                    //Compute surface vertex, normal and material on each crossing edge
                    int idxMeshVertex[16];
                    int idxEdgeStart, idxEdgeEnd;
                    int candidates[16];

                    //Read case
                    size_t ctTotalPolygons = 0;
                    size_t ctEdges = 0;
                    vec3f p;
                    vec3f n;
                    vec4f diffused;
                    float fp;

                    for(icase=0; icase<16; icase++)
                    {
                        candidates[icase] = g_triTableCache[idxCellConfig][icase];
                        if(candidates[icase] != -1)
                            ctEdges++;
                    }
                    ctTotalPolygons = ctEdges / 3;

                    for(icase = 0; icase < (int)ctEdges; icase++)
                    {
                        idxEdgeStart = corner1[candidates[icase]];
                        idxEdgeEnd	 = corner2[candidates[icase]];
                        idxMeshVertex[icase] = m_processedEdges.getEdge(cellCornerIDX[idxEdgeStart], cellCornerIDX[idxEdgeEnd]);
                        if(idxMeshVertex[icase] == -1)
                        {
                            m_statFieldEvaluations += ComputeRootNewtonRaphson(m_root, cellCornerPos[idxEdgeStart], cellCornerPos[idxEdgeEnd],
                                                                               m_grid.pValues[cellCornerKey[idxEdgeStart]],
                                                                               m_grid.pValues[cellCornerKey[idxEdgeEnd]], p, fp);

                            //Setup mesh
                            m_statFieldEvaluations += 3;
                            n		 = m_root->normal(p, fp, NORMAL_DELTA);
                            diffused = m_root->getMaterial().diffused;

                            outputMesh.addVertex(p);
                            outputMesh.addNormal(n);
                            outputMesh.addColor(diffused);

                            //Save PolyVertex
                            //Get vertex v index from list. It is the last one
                            idxMeshVertex[icase] = outputMesh.countVertices() - 1;
                            m_processedEdges.setEdge(cellCornerIDX[idxEdgeStart], cellCornerIDX[idxEdgeEnd], idxMeshVertex[icase]);
                        }
                    }

                    for(icase = 0; icase < (int)ctTotalPolygons; icase++)
                    {
                        outputMesh.addTriangle(idxMeshVertex[icase*3+0], idxMeshVertex[icase*3+1], idxMeshVertex[icase*3+2]);
                    }
                }
            }
        }
    }

    return (ctInside > 0);
}

void CMpu::stProcessCell(const MPUCELL& cell )
{
    int icase;
    int idxCellConfig = 0;
    for(icase=0; icase<8; icase++)
    {
        if(cell.corners[icase].value > ISO_VALUE)
            idxCellConfig += (1 << icase);
    }

    if((idxCellConfig != 0)&&(idxCellConfig != 255))
    {
        //Increment number of processed cells
        m_statIntersectedCells++;

        //Compute surface vertex, normal and material on each crossing edge
        int idxMeshVertex[16];
        int idxEdgeStart, idxEdgeEnd;
        int candidates[16];

        //Read case
        size_t ctTotalPolygons = 0;
        size_t ctEdges = 0;
        vec3f p;
        vec3f n;
        vec4f diffused;
        CMaterial mtrl;
        float fp;
        vec3i startIDX, endIDX;

        for(icase=0; icase<16; icase++)
        {
            candidates[icase] = g_triTableCache[idxCellConfig][icase];
            if(candidates[icase] != -1)
                ctEdges++;
        }
        ctTotalPolygons = ctEdges / 3;

        for(icase = 0; icase < (int)ctEdges; icase++)
        {
            idxEdgeStart = corner1[candidates[icase]];
            idxEdgeEnd	 = corner2[candidates[icase]];

            startIDX.x = cell.corners[idxEdgeStart].i;
            startIDX.y = cell.corners[idxEdgeStart].j;
            startIDX.z = cell.corners[idxEdgeStart].k;
            endIDX.x   = cell.corners[idxEdgeEnd].i;
            endIDX.y   = cell.corners[idxEdgeEnd].j;
            endIDX.z   = cell.corners[idxEdgeEnd].k;

            idxMeshVertex[icase] = m_processedEdges.getEdge(startIDX, endIDX);
            if(idxMeshVertex[icase] == -1)
            {
                m_statFieldEvaluations += ComputeRootNewtonRaphson(m_root,
                                                                   cell.corners[idxEdgeStart].pos,
                                                                   cell.corners[idxEdgeEnd].pos,
                                                                   cell.corners[idxEdgeStart].value,
                                                                   cell.corners[idxEdgeEnd].value, p, fp);

                //Setup mesh
                m_statFieldEvaluations += 3;
                n		 = m_root->normal(p, fp, NORMAL_DELTA);
                diffused = m_root->getMaterial().diffused;

                outputMesh.addVertex(p);
                outputMesh.addNormal(n);
                outputMesh.addColor(diffused);

                //Save PolyVertex
                //Get vertex v index from list. It is the last one
                idxMeshVertex[icase] = outputMesh.countVertices() - 1;
                m_processedEdges.setEdge(startIDX, endIDX, idxMeshVertex[icase]);
            }
        }

        for(icase = 0; icase < (int)ctTotalPolygons; icase++)
        {
            outputMesh.addTriangle(idxMeshVertex[icase*3+0], idxMeshVertex[icase*3+1], idxMeshVertex[icase*3+2]);
        }
    }
}

float CMpu::fieldToDistance( float fieldvalue )
{
    static const float oneThird = 1.0f / 3.0f;
    if(fieldvalue >= 1.0f)
        return ISO_DISTANCE;
    else if(fieldvalue <= 0)
        return 1.0f - ISO_DISTANCE;

    return Absolutef(sqrt(1 - pow(fieldvalue, oneThird)) - ISO_DISTANCE);
}
//******************************************************************************
void CParsipServer::removeAllMPUs()
{
    if(m_lstMPUs.size() == 0) return;
    CMpu* ampu;
    for(size_t i=0; i<m_lstMPUs.size(); i++)
    {
        ampu = m_lstMPUs[i];
        SAFE_DELETE(ampu);
    }
    m_lstMPUs.resize(0);
}

void CParsipServer::setup(CLayerManager* layerManager, int griddim)
{
    for(size_t i=0; i<layerManager->countLayers(); i++)
        setup(layerManager->getLayer(i), i, griddim);
}

void CParsipServer::setup(CLayer* aLayer, int id, int griddim)
{
    if(aLayer == NULL) return;

    I64 tsStart = CPerfLogger::getPerfCounter();

    //Process BlobTree
    CBlobNode* reducedRoot = aLayer->getBlob();
    COctree oct		    = aLayer->getOctree();
    float cellsize		= aLayer->getCellSize();
    float normalAngle   = aLayer->getAdaptiveParam();
    //CellShape cellshape = aLayer->getCellShape();



    //Get all primitive octrees and seeds in this layer
    vector<vec3f> lstOctreeLos;
    vector<vec3f> lstOctreeHis;
    vector<vec3f> lstSeeds;
    aLayer->queryGetAllOctrees(lstOctreeLos, lstOctreeHis);
    aLayer->getAllSeeds(lstSeeds);
    vec3f allSides		= oct.upper - oct.lower;
    vec3i ctCellsNeeded = vec3i(static_cast<int>(ceil(allSides.x / cellsize)),
                                static_cast<int>(ceil(allSides.y / cellsize)),
                                static_cast<int>(ceil(allSides.z / cellsize)));


    m_gridDim = griddim;
    int cellsPerMPU = griddim - 1;
    vec3i ctMPUNeeded;
    ctMPUNeeded.x = ctCellsNeeded.x / cellsPerMPU;
    ctMPUNeeded.y = ctCellsNeeded.y / cellsPerMPU;
    ctMPUNeeded.z = ctCellsNeeded.z / cellsPerMPU;
    if(ctCellsNeeded.x % cellsPerMPU != 0)
        ctMPUNeeded.x++;
    if(ctCellsNeeded.y % cellsPerMPU != 0)
        ctMPUNeeded.y++;
    if(ctCellsNeeded.z % cellsPerMPU != 0)
        ctMPUNeeded.z++;

    //Create all MPUs
    CMpu* aPolygnizerUnit = NULL;
    float gridSide = static_cast<float>(cellsPerMPU) * cellsize;
    for(int i=0; i<ctMPUNeeded.x; i++)
    {
        for(int j=0; j<ctMPUNeeded.y; j++)
        {
            for(int k=0; k<ctMPUNeeded.z; k++)
            {
                vec3f mpuOrigin = oct.lower + vec3f(gridSide*i, gridSide*j, gridSide*k);

                aPolygnizerUnit = new CMpu(vec3i(i, j, k), mpuOrigin, oct.upper, m_gridDim, id, cellsize, normalAngle, reducedRoot);
                aPolygnizerUnit->setAllPrimitiveBoundsAndSeeds(lstOctreeLos, lstOctreeHis, lstSeeds);
                aPolygnizerUnit->setColorCodeMPU(m_bShowColorCodedMPUs);
                aPolygnizerUnit->setForceMC(m_bForceMC);
                aPolygnizerUnit->setAdaptiveSubDivision(m_bUseAdaptiveSubDivision);
                m_lstMPUs.push_back(aPolygnizerUnit);
            }
        }
    }

    //Cleanup
    lstOctreeHis.clear();
    lstOctreeLos.clear();
    lstSeeds.clear();

    //Compute Time
    I64 tsEnd = CPerfLogger::getPerfCounter();
    double current = CPerfLogger::convertTimeTicksToMS(tsEnd - tsStart);
    if(current > m_tsSetup)
        m_tsSetup = current;
}

void CParsipServer::run()
{
    m_tsStart = CPerfLogger::convertTimeTicksToMS(CPerfLogger::getPerfCounter());

    CMPURunBody body;
    body.input = m_lstMPUs;
    parallel_for(blocked_range<int>(0, m_lstMPUs.size()), body, tbb::auto_partitioner());

    m_tsEnd = CPerfLogger::convertTimeTicksToMS(CPerfLogger::getPerfCounter());
    m_tsPolygonize = CPerfLogger::convertTimeTicksToMS(m_tsEnd - m_tsStart);
}

void CParsipServer::drawMesh(int iLayer)
{
    if(iLayer == -1)
    {
        for(size_t i=0; i<m_lstMPUs.size(); i++)
        {
            if(m_lstMPUs[i]->isReady())
                m_lstMPUs[i]->outputMesh.drawBuffered();
        }
    }
    else
    {
        CMpu* ampu = NULL;
        for(size_t i=0; i<m_lstMPUs.size(); i++)
        {
            ampu = m_lstMPUs[i];
            if(ampu->isReady() && (ampu->getId() == iLayer))
                m_lstMPUs[i]->outputMesh.drawBuffered();
        }
    }
}

void CParsipServer::drawNormals(int iLayer, int normalLength)
{
    if(iLayer == -1)
    {
        for(size_t i=0; i<m_lstMPUs.size(); i++)
        {
            if(m_lstMPUs[i]->isReady())
                m_lstMPUs[i]->outputMesh.drawNormals(normalLength);
        }
    }
    else
    {
        CMpu* ampu = NULL;
        for(size_t i=0; i<m_lstMPUs.size(); i++)
        {
            ampu = m_lstMPUs[i];
            if(ampu->isReady() && (ampu->getId() == iLayer))
                m_lstMPUs[i]->outputMesh.drawNormals(normalLength);
        }
    }
}

void CParsipServer::reset()
{
    m_tsStart = 0.0;
    m_tsSetup = 0.0;
    m_tsPolygonize = 0.0;
}

double CParsipServer::getTimingStats( double& tsSetup, double& tsPolygonize ) const
{
    tsSetup = m_tsSetup;
    tsPolygonize = m_tsPolygonize;
    return tsSetup + tsPolygonize;
}

void CParsipServer::getStartEndTime( double& tsStart, double& tsEnd ) const
{
    tsStart = m_tsStart;
    tsEnd = m_tsEnd;
}


bool CParsipServer::getMPUExtent(size_t idxMPU, vec3f& lo, vec3f& hi) const
{
    lo = vec3f(0.0f, 0.0f, 0.0f);
    hi = lo;
    if((idxMPU >= 0) && (idxMPU < m_lstMPUs.size()))
    {
        float gridSide = m_lstMPUs[idxMPU]->getGridSide();
        lo = m_lstMPUs[idxMPU]->getOrigin();
        hi = lo + vec3f(gridSide, gridSide, gridSide);
        return true;
    }

    return false;
}

void CParsipServer::removeLayerMPUs( int idxLayer )
{
    size_t i=0;
    while(i < m_lstMPUs.size())
    {
        if(m_lstMPUs[i]->getId() == idxLayer)
        {
            CMpu* ampu = m_lstMPUs[i];
            m_lstMPUs.erase(m_lstMPUs.begin() + i);
            SAFE_DELETE(ampu);
        }
        else
            i++;
    }

}

void CParsipServer::getMeshStats( size_t& ctVertices, size_t& ctFaces ) const
{
    ctVertices = 0;
    ctFaces = 0;
    for(size_t i=0; i<m_lstMPUs.size(); i++)
    {
        ctVertices += m_lstMPUs[i]->outputMesh.countVertices();
        ctFaces += m_lstMPUs[i]->outputMesh.countFaces();
    }
}

void CParsipServer::getMeshStats(int idxLayer, size_t& ctVertices, size_t& ctFaces) const
{
    ctVertices = 0;
    ctFaces = 0;
    for(size_t i=0; i<m_lstMPUs.size(); i++)
    {
        if(m_lstMPUs[i]->getId() == idxLayer)
        {
            ctVertices += m_lstMPUs[i]->outputMesh.countVertices();
            ctFaces += m_lstMPUs[i]->outputMesh.countFaces();
        }
    }
}

size_t CParsipServer::getIntersectedMPUs() const
{
    size_t ctIntersected = 0;
    for(size_t i=0; i<m_lstMPUs.size(); i++)
    {
        if(m_lstMPUs[i]->hasSurface())
            ctIntersected++;
    }
    return ctIntersected;
}

int CParsipServer::getSurfaceTrackedMPUs() const
{
    size_t ctSurfaceTracked = 0;
    for(size_t i=0; i<m_lstMPUs.size(); i++)
    {
        if((m_lstMPUs[i]->hasSurface())&&
                (m_lstMPUs[i]->statPerformedSurfaceTracking()))
            ctSurfaceTracked++;
    }
    return ctSurfaceTracked;
}

size_t CParsipServer::getFieldEvalStats() const
{
    size_t ctFieldEvals = 0;
    for(size_t i=0; i<m_lstMPUs.size(); i++)
    {
        ctFieldEvals += m_lstMPUs[i]->statFieldEvaluations();
    }
    return ctFieldEvals;
}

size_t CParsipServer::getIntersectedCellsStats() const
{
    size_t ctProcessedCells = 0;
    for(size_t i=0; i<m_lstMPUs.size(); i++)
    {
        ctProcessedCells += m_lstMPUs[i]->statIntersectedCells();
    }
    return ctProcessedCells;
}

double CParsipServer::getLastestMPUTime() const
{
    double tsProcess = 0.0;
    for(size_t i=0; i<m_lstMPUs.size(); i++)
    {
        if(tsProcess < m_lstMPUs[i]->statProcessTime())
            tsProcess = m_lstMPUs[i]->statProcessTime();
    }
    return tsProcess;
}

CMpu* CParsipServer::getLastestMPU() const
{
    double tsProcess = 0.0;
    CMpu* latestMPU = NULL;
    for(size_t i=0; i<m_lstMPUs.size(); i++)
    {
        if(tsProcess < m_lstMPUs[i]->statProcessTime())
        {
            tsProcess = m_lstMPUs[i]->statProcessTime();
            latestMPU = m_lstMPUs[i];
        }
    }
    return latestMPU;
}

int CParsipServer::getCoreUtilizations( vector<size_t>& arrOutThreadIDs, vector<double>& arrOutUtilization )
{
    size_t id;

    typedef pair<size_t,double> threadpair;
    std::map<size_t, double> m;
    std::map<size_t, double>::iterator mIter;

    double t;
    int ctActiveCores = 0;

    for(size_t i=0; i<m_lstMPUs.size(); i++)
    {
        id = m_lstMPUs[i]->statGetThreadID();
        if(m.find(id) == m.end())
        {
            m.insert(threadpair(id, m_lstMPUs[i]->statProcessTime()));
            ctActiveCores++;
        }
        else
        {
            t = m[id];
            m[id] = t + m_lstMPUs[i]->statProcessTime();
        }
    }

    //Resize output
    arrOutThreadIDs.resize(ctActiveCores);
    arrOutUtilization.resize(ctActiveCores);

    int iCore = 0;
    for(mIter = m.begin(); mIter != m.end(); mIter++)
    {
        arrOutThreadIDs[iCore] = mIter->first;
        arrOutUtilization[iCore] = mIter->second / m_tsPolygonize;
        iCore++;
    }
    m.clear();
    return ctActiveCores;
}

CMpu* CParsipServer::getMPU( int idx ) const
{
    if(idx >=0 && idx < (int)m_lstMPUs.size())
        return m_lstMPUs[idx];
    else
        return NULL;
}

}
