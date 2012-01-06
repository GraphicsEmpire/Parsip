#ifndef PS_OCLPOLYGONIZER_H
#define PS_OCLPOLYGONIZER_H

#include <iostream>
#include "CBlobTree.h"

using namespace std;
using namespace PS;
using namespace PS::BLOBTREE;

#include "PS_SimdPoly/include/PS_Polygonizer.h"

using namespace PS::SIMDPOLY;

class SimdPoly{
private:
    SOABlobPrims        m_blobPrims;
    SOABlobOps          m_blobOps;
    SOABlobPrimMatrices m_blobPrimMatrices;
    SOABlobBoxMatrices  m_blobBoxMatrices;
    PolyMPUs            m_polyMPUs;

public:
    SimdPoly() {reset();}
    void reset();

    int linearizeBlobTree(CBlobNode* root);
    int run(float cellsize);
    void draw(bool bDrawNormals = false);

protected:
    int linearizeBlobTree(CBlobNode* root, int parentID, int& outIsOperator);
};

/*
void SIMDPOLY_Reset();
int SIMDPOLY_LinearizeBlobTree(CBlobNode* root, int parentID, int& outIsOperator);
int SIMDPOLY_Run(float cellsize);
void SIMDPOLY_Draw(bool bDrawNormals = false);
*/


int RunOclPolygonizer();


#endif // PS_OCLPOLYGONIZER_H
