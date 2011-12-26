#pragma once
#ifndef CPARALLEL_ADAPTIVE_SUBD
#define CPARALLEL_ADAPTIVE_SUBD

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>

#include "PS_FrameWork/include/PS_MeshVV.h"
#include "PS_BlobTree/include/CBlobTree.h"

using namespace PS;
using namespace PS::BLOBTREE;
//////////////////////////////////////////////////////////////////////////
//Parallel Version of SubDivide Analyze
class SubdivideAnalyzeBody
{
private:
	PS::CMeshVV* m_lpInMesh;
	float m_adaptiveParam;

	int* m_lpOutVertexCount;
	int* m_lpOutDecisionBits;
	size_t m_ctOutCoarseFaces;

public:
	SubdivideAnalyzeBody(float adaptiveParam, 
						 PS::CMeshVV* lpInMesh,
						 int* lpOutVertexCount, int* lpOutDecisionBits)
	{
		m_adaptiveParam		= adaptiveParam;
		m_lpInMesh			= lpInMesh;
		m_lpOutVertexCount  = lpOutVertexCount;
		m_lpOutDecisionBits = lpOutDecisionBits;
		m_ctOutCoarseFaces  = 0;
	}

	SubdivideAnalyzeBody(SubdivideAnalyzeBody& parent, tbb::split)
	{		
		m_lpInMesh		= parent.m_lpInMesh;
		m_adaptiveParam = parent.m_adaptiveParam;

		m_lpOutVertexCount = parent.m_lpOutVertexCount;
		m_lpOutDecisionBits = parent.m_lpOutDecisionBits;
		m_ctOutCoarseFaces = 0;
	}

	size_t getCoarseFacesCount() const {return m_ctOutCoarseFaces;}

	void join(SubdivideAnalyzeBody& rhs)
	{
		m_ctOutCoarseFaces += rhs.m_ctOutCoarseFaces;
	}

	void operator() (const tbb::blocked_range<int>& range);
};

//////////////////////////////////////////////////////////////////////////

class SubDividePerformBody
{
private:
	CBlobNode* m_lpInRoot;
	int* m_lpInVertexCount;
	int* m_lpInVertexCountScanned;
	int* m_lpInDecisionBits;
	PS::CMeshVV* m_lpInMesh;
	
	PS::CMeshVV* m_lpOutMesh;
public:
	SubDividePerformBody(CBlobNode* lpInRoot, 
						 int* lpInVertexCount, 
						 int* lpInVertexCountScanned, 
						 int* lpInDecisionBits, 
						 PS::CMeshVV* lpInMesh, 
						 PS::CMeshVV* lpOutMesh)
	{
		m_lpInRoot		  = lpInRoot;
		m_lpInVertexCount = lpInVertexCount;
		m_lpInVertexCountScanned = lpInVertexCountScanned;
		m_lpInDecisionBits = lpInDecisionBits;
		m_lpInMesh = lpInMesh;
		m_lpOutMesh = lpOutMesh;
	}
	
	void operator() (const tbb::blocked_range<int>& range) const;
};

//////////////////////////////////////////////////////////////////////////
//Moves edges midpoint to iso-surface
int  SubDivide_MoveMidPointToSurface(CBlobNode* root, 
									 vec3f& m, 
									 vec3f& outputNormal, 
									 vec4f& outputColor, 
									 float target_field = ISO_VALUE, 
									 int nIterations = DEFAULT_ITERATIONS );

//Returns estimated number of field evaluations.
size_t SubDivide_ParallelPerform(CMeshVV& inMesh, CBlobNode* lpInBlob, float adaptiveParam);

size_t SubDivide_Analyze(CMeshVV& inMesh, float adaptiveParam, vector<int>& arrVertexCount, vector<int>& arrDecisionBits);

#endif
