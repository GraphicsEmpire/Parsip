#ifndef PS_OCLPOLYGONIZER_H
#define PS_OCLPOLYGONIZER_H

#include <iostream>
#include "CBlobTree.h"

using namespace std;
using namespace PS;
using namespace PS::BLOBTREE;

void SIMDPOLY_Reset();
int SIMDPOLY_LinearizeBlobTree(CBlobNode* root, int parentID, int& outIsOperator);
int SIMDPOLY_Run(float cellsize);

void SIMDPOLY_Draw(bool bDrawNormals = false);


int RunOclPolygonizer();


#endif // PS_OCLPOLYGONIZER_H
