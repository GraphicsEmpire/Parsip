#pragma once
#ifndef CPOLY_PARSIP_SERVER_H
#define CPOLY_PARSIP_SERVER_H
#include <vector>
#include <stack>

#include "_GlobalFunctions.h"
#include "CLayerManager.h"
#include "CCubeTable.h"
#include "PS_FrameWork/include/TaskManager.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

#include "CPolyHashGridEdgeTables.h"


using namespace tbb;

using namespace PS::BLOBTREE;
using namespace std;

#define CROSS_ALL	1
#define CROSS_NONE	0
//#define CROSS_SOME	2

/*
#define GRID_DIM_16

#ifdef GRID_DIM_32
#define GRID_DIM 32
#define CELLID_SHIFT_X 0
#define CELLID_SHIFT_Y 5
#define CELLID_SHIFT_Z 10
#define CELLID_BITMASK 0x1F
#endif

#ifdef GRID_DIM_16
#define GRID_DIM 16
#define CELLID_SHIFT_X 0
#define CELLID_SHIFT_Y 4
#define CELLID_SHIFT_Z 8
#define CELLID_BITMASK 0x0F
#endif

#ifdef GRID_DIM_8
#define GRID_DIM 8
#define CELLID_SHIFT_X 0
#define CELLID_SHIFT_Y 3
#define CELLID_SHIFT_Z 6
#define CELLID_BITMASK 0x07
#endif

#define CELLID_HASHSIZE (size_t)(1<<(3*CELLID_SHIFT_Y))

#define CORE_CACHE_SIZE	512*1024
#define MPU_FIELD_CACHE_SIZE CORE_CACHE_SIZE*2/12

#define CELLID_FROM_IDX(i,j,k) (((k) & CELLID_BITMASK) << CELLID_SHIFT_Z) | (((j) & CELLID_BITMASK) << CELLID_SHIFT_Y) | ((i) & CELLID_BITMASK)
*/


#define MAX_ATTEMPTS 32

namespace PS{

//Model Polygonizer Unit is assigned to each core for mesh extraction
class CMpu
{
private:	
	//Some stat vars
	size_t m_statFieldEvaluations;
	size_t m_statIntersectedCells;

	size_t m_threadID;

	I64	   m_statStartTick;
	I64	   m_statEndTick;
	//Time to process
	double m_statProcessTime;

	//Address of grid within all grids in layer
	vec3i m_globalAddress;

	//Layer id
	int m_id;

	//Origin of the grid which is LBN corner of it
	vec3f m_origin;	

	//Size of a single cell which controls the resolution of the mesh
	float m_cellsize;	

	//Hash Function	
	HASHFUNC m_hashFunc;

	//Adaptive Param
	float m_adaptiveParam;
		
	//Pointer to root blobtree
	CBlobNode* m_root;
	bool m_bTreeCompacted;

	//Color Code
	bool m_bColorCodeMPU;
	//Force MC
	bool m_bForceMC;

	//Flag to control if MPU has any portion of the surface
	bool m_bHasSurface;

	//Flag to show if this MPU has performed surface tracking
	bool m_bDoSurfaceTracking;

	//Perform Adaptive Method
	bool m_bUseAdaptiveSubDivision;
	
	//Flag is set when MPU is finished processing
	bool m_bReady;


	//Upper bound is the furthest position that fieldvalue will be computed for
	vec3f m_upperBound;

	//Variables used in both MC and ST (Marching Cubes and Surface Tracking)
	//List of processed edges
	CProcessedEdges m_processedEdges;

	//grid to cache to field values
	Grid m_grid;	

	//Bound
	vec3i m_gridBound;

	// Global list of corners (keeps track of memory)	
        std::vector<MPUCELL> m_stkTempCubes;
	
	//vector of a linked list means 2D table
	// cube center hash table
        vector<CENTERLIST> m_stHashTableProcessedCells;

	//Lows and highs for all primitives
        vector<vec3f> m_primitiveLos;
        vector<vec3f> m_primitiveHis;

	//List of all primitive seed points
        vector<vec3f> m_stPrimitiveSeeds;
	vec3f m_stSeedCellCenter;	

	//Surface Tracking
	MPUCORNER stSetCorner (int i, int j, int k);
	bool stSetProcessedCell(int i, int j, int k);	
	void stProcessCell(const MPUCELL& cell );

	bool stQueryCellFace(const MPUCELL& old, int i, int j, int k,  
						 int face, int c1, int c2, int c3, int c4);

	float fieldToDistance(float fieldvalue);

	//Surface Tracking
	bool doSurfaceTracking();

	//Marching Cubes
	bool doMarchingCubes();
public:	
	CMeshVV outputMesh;

	CMpu() 
	{ 
		m_bReady		= false;
		m_bHasSurface	= false;
		m_bForceMC		= false;
		m_bColorCodeMPU = false;
	}

	CMpu(vec3i globalAddress, vec3f origin, vec3f upperBound, int dim, int id, float cellsize, float adaptiveParam, CBlobNode* root)
	{	
		setup(globalAddress, origin, upperBound, dim, id, cellsize, adaptiveParam, root);
	}

	~CMpu()
	{
		cleanup();
	}

	//Compaction
	void compactBlobTree_Intersect(const COctree& mpuOct, CBlobNode* lpNode);
	int  compactBlobTree_Recursive( CBlobNode* lpInput, stack<CBlobNode*>& stkOperators, CBlobNode*& lpOutClone);
	//int  compactBlobTree_Remove( CBlobTree* lpInput, stack<CBlobTree*>& stkOperators, CBlobTree*& lpOutClone);
		

	//Adaptive SubDivision 
	size_t subDivide();
        size_t subDivide_Analyze(vector<int>& arrVertexCount, vector<int>& arrDecisionBits);
        size_t subDivide_Perform(vector<int>& arrVertexCount, vector<int>& arrDecisionBits);
	int  subDivide_MoveMidPointToSurface(vec3f& m, vec3f& outputNormal, vec4f& outputColor, float target_field = ISO_VALUE, int nIterations = DEFAULT_ITERATIONS );

        void setAllPrimitiveBoundsAndSeeds(const vector<vec3f> los, const vector<vec3f> his, const vector<vec3f> seeds)
	{
             m_primitiveLos.assign(los.begin(), los.end());
             m_primitiveHis.assign(his.begin(), his.end());
             m_stPrimitiveSeeds.assign(seeds.begin(), seeds.end());
	}

	void setAdaptiveSubDivision(bool bEnable) {m_bUseAdaptiveSubDivision = bEnable;}
	void setForceMC(bool bEnable) {m_bForceMC = bEnable;}
	void setColorCodeMPU(bool bEnable) {m_bColorCodeMPU = bEnable;}

	void cleanup();

	vec3f getOrigin() const { return m_origin;}

	float getGridSide() const {return static_cast<float>(m_hashFunc.dim - 1) * m_cellsize;}

	vec3i getGlobalAddress() const {return m_globalAddress;}

	vec3f getPos(int i, int j, int k) const
	{
		return m_origin + vec3f(m_cellsize*i, m_cellsize*j, m_cellsize*k);
	}

	int getId() const {return m_id;}	


	//Returns number of triangles produced
	//Fills edges array with the order of vertices along edges
	//int getCubeCaseTriangles(int idxCase, int edges[]);

	bool isReady() const {return m_bReady;}
	bool hasSurface() const {return m_bHasSurface;}
	
	bool statPerformedSurfaceTracking() const {return m_bDoSurfaceTracking;}
	double statProcessTime() const {return m_statProcessTime;}
	I64	   statGetStartTick() const {return m_statStartTick;}
	I64	   statGetEndTick() const {return m_statEndTick;}
	size_t statFieldEvaluations() const {return m_statFieldEvaluations;}
	size_t statIntersectedCells() const {return m_statIntersectedCells;}
	size_t statGetThreadID() const {return m_threadID;}

	void setup(vec3i globalAddress, vec3f origin, vec3f upperBound, int dim, int id, float cellsize, float adaptiveParam, CBlobNode* root)
	{
		m_globalAddress = globalAddress;
		m_origin		= origin;
		m_upperBound	= upperBound;		
		m_hashFunc.setup(dim);
		m_id	   = id;	
		m_adaptiveParam = adaptiveParam;
		m_cellsize	  = cellsize;
		m_root		  = root;
		m_bReady	  = false;
		m_bHasSurface = false;
		m_bForceMC		= false;
		m_bColorCodeMPU = false;
		m_bUseAdaptiveSubDivision = true;		
	}

	//Runs the process of polygonization within current MPU
	void run();
};

struct CMPURunBody{
        vector<CMpu*> input;
	void operator()(const blocked_range<int>& range) const
	{
		for(int i=range.begin(); i != range.end(); i++)
			input[i]->run();
	}
};
//////////////////////////////////////////////////////////////////////////
//Parallel Implicit Polygonizer
class CParsipServer{
private:
        vector<CMpu*> m_lstMPUs;
	double m_tsStart;
	double m_tsEnd;
	double m_tsSetup;
	double m_tsPolygonize;

	bool m_bShowColorCodedMPUs;
	bool m_bForceMC;
	bool m_bUseAdaptiveSubDivision;
	int m_gridDim;

	void init()
	{
		m_bShowColorCodedMPUs = false;
		m_bForceMC = false;
		m_tsStart = 0.0;
		m_tsSetup = 0.0;
		m_tsPolygonize = 0.0;
		m_gridDim = DEFAULT_GRIM_DIM;
	}
public:	
	CParsipServer() 
	{ 
		init();
	}

	CParsipServer(CLayerManager* layerManager, int griddim)
	{
		init();
		setup(layerManager, griddim);
	}

	CParsipServer(CLayer* aLayer, int id, int griddim)
	{
		init();
		setup(aLayer, id, griddim);
	}


	~CParsipServer()
	{
		removeAllMPUs();
	}

	void reset();
	
	//Process
	void setup(CLayerManager* layerManager, int griddim);
	void setup(CLayer* aLayer, int id, int griddim);
	void run();

	bool getMPUExtent(size_t idxMPU, vec3f& lo, vec3f& hi) const;

	//Stats
	void   getStartEndTime(double& tsStart, double& tsEnd) const;
	double getTimingStats(double& tsSetup, double& tsPolygonize) const;			
	CMpu*  getLastestMPU() const;
	double getLastestMPUTime() const;
	size_t getIntersectedMPUs() const;
	void   getMeshStats(size_t& ctVertices, size_t& ctFaces) const;
	void   getMeshStats(int idxLayer, size_t& ctVertices, size_t& ctFaces) const;
	size_t getFieldEvalStats() const;

	size_t getIntersectedCellsStats() const;
	size_t getTotalCellsInAllMPUsStats() const {return (m_gridDim-1)*(m_gridDim-1)*(m_gridDim-1)*m_lstMPUs.size();}
	size_t getTotalCellsInIntersectedMPUsStats() const {return (m_gridDim-1)*(m_gridDim-1)*(m_gridDim-1)*getIntersectedMPUs();}
        int	   getCoreUtilizations(vector<size_t>& arrOutThreadIDs, vector<double>& arrOutUtilization);
	

	//Return number of MPUs that has computed some surface with ST
	int getSurfaceTrackedMPUs() const;	
	size_t countMPUs() const {return m_lstMPUs.size();}
	CMpu*  getMPU(int idx) const;

	//Set color coded MPUs	
	void setColorCodedMPUs(bool bEnable) {m_bShowColorCodedMPUs = bEnable;}
	void setForceMC(bool bEnable) {m_bForceMC = bEnable;}
	void setAdaptiveSubdivision(bool bEnable) {m_bUseAdaptiveSubDivision = bEnable;}


	//Remove and Cleanup
	void removeLayerMPUs( int idxLayer );
	void removeAllMPUs();

	//Draw calls
	void drawMesh(int iLayer = -1);
	void drawNormals(int iLayer, int normalLength);
};
//void renderMPU(CMpu* mpu);
void renderFrame(CLayer* aLayer);

}

#endif 

