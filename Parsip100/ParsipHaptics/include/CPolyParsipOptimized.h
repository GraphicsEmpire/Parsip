#pragma once
#ifndef CPOLYPARSIP_OPTIMIZED_H
#define CPOLYPARSIP_OPTIMIZED_H

#include "_GlobalFunctions.h"
//#include "CLayerManager.h"

#include "CCubeTable.h"

#include "tbb/parallel_reduce.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "CompactBlobTree.h"
#include "CPolyHashGridEdgeTables.h"
#include "PS_FrameWork/include/PS_HWUtils.h"
#include "PS_FrameWork/include/PS_MeshVV.h"

using namespace tbb;

using namespace PS::BLOBTREE;
using namespace std;

#define MAX_ATTEMPTS 32
#define GRID_DIM_8

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

#define EDGETABLE_DEPTH 8

namespace PS{
	
	class CSIMDMPU
	{
	private:
		//Origin of this MPU
		vec3f m_origin;

		//Sides Lengths
		vec3f m_sides;

		//Stats
		double m_statStartTime;		
		double m_statProcessTime;

		//Statistics		
		size_t m_statFieldEvaluations;
		size_t m_statIntersectedCells;

		//Control Flags		
		bool m_bReady;

		//Thread ID
		size_t m_threadID;
	public:
		CMeshVV* lpOutputMesh;
		

		CSIMDMPU(const vec3f& origin, const vec3f& sides)
		{			
			m_origin   = origin;			
			m_sides	   = sides;
			lpOutputMesh = new CMeshVV();
		}

		~CSIMDMPU()
		{
			lpOutputMesh->removeAll();
			SAFE_DELETE(lpOutputMesh);
		}

		//Accessors		
		bool isReady() const {return m_bReady;}
		bool hasSurface() const {return (lpOutputMesh->countFaces() > 0);}		
		size_t getThreadId() const {return m_threadID;}
		
		vec3f getOrigin() const {return m_origin;}		
		vec3f getSides() const {return m_sides;}
		void  setSides(const vec3f& s) {m_sides = s;}

		//Statistics
		double statsStartTime() const {return m_statStartTime;}		
		double statsProcessTime() const {return m_statProcessTime;}
		size_t statsFieldEvals() const {return m_statFieldEvaluations;}
		size_t statsIntersectedCells() const {return m_statIntersectedCells;}

		//Runs current MPU
		void start();
		void finish(size_t ctFieldEvals, size_t ctIntersectedCells);
	};
	//////////////////////////////////////////////////////////////////////////
	//A range on list of MPUs is assigned to the core which will be
	//processed using core resources

	//Processed Edges
	struct SIMDEDGEELEMENTS
	{
		vec3i start[EDGETABLE_DEPTH];
		vec3i end[EDGETABLE_DEPTH];
		int vid[EDGETABLE_DEPTH];
	};

	class CSIMDMPURunBody
	{
	private:
		size_t m_ctExtra;
		//FieldValue Cache
		//__declspec(align(16)) float m_fvCache[GRID_DIM*GRID_DIM*GRID_DIM];	
		float*	m_fvCache;

		COMPACTBLOBTREE* m_cptBlob;
		//Edge Elements
		//SIMDEDGEELEMENTS m_edgeTable[2*CELLID_HASHSIZE];
		//int m_edgeTableSizes[2*CELLID_HASHSIZE];		
		SIMDEDGEELEMENTS* m_edgeTable;
		int* m_edgeTableSizes;		

		//Input to PAR_FOR
		float m_isovalue;
		float m_cellsize;
		vec3f m_modelUpperCorner;		
		DVec<CSIMDMPU*>  m_vPartitions;


	private:
		int  getEdge(vec3i start, vec3i end) const;
		void setEdge(vec3i start, vec3i end, int vid) const;
		bool doMarchingCubes(CSIMDMPU* aMPU) const;
		__inline bool intersects( const vec4f& lo1, const vec4f& hi1, const vec4f& lo2, const vec4f& hi2 ) const;
	public:
		CSIMDMPURunBody(COMPACTBLOBTREE* lpCptBlob, 
						const vec3f& upperCorner, 
						const DVec<CSIMDMPU*>& vPartitions,
						float cellsize = DEFAULT_CELL_SIZE, 
						float isovalue = ISO_VALUE)
		{
			m_ctExtra = 0;
			m_cellsize		   = cellsize;
			m_isovalue		   = isovalue;
			m_modelUpperCorner = upperCorner;						
			m_vPartitions.copyFrom(vPartitions);	

			//Get Memory
			m_cptBlob = new COMPACTBLOBTREE(*lpCptBlob);
			//m_cptBlob = lpCptBlob;
			m_edgeTable = new SIMDEDGEELEMENTS[2*CELLID_HASHSIZE];
			m_edgeTableSizes = new int[2*CELLID_HASHSIZE];
			m_fvCache = new float[GRID_DIM*GRID_DIM*GRID_DIM];
		}

		CSIMDMPURunBody(CSIMDMPURunBody& parent, split)
		{
			m_ctExtra  = 0;
			m_cellsize		   = parent.m_cellsize;
			m_isovalue		   = parent.m_isovalue;
			m_modelUpperCorner = parent.m_modelUpperCorner;			
			m_vPartitions.copyFrom(parent.m_vPartitions);

			//Get Memory
			m_cptBlob = new COMPACTBLOBTREE(*parent.m_cptBlob);
			//m_cptBlob = parent.m_cptBlob;
			m_edgeTable = new SIMDEDGEELEMENTS[2*CELLID_HASHSIZE];
			m_edgeTableSizes = new int[2*CELLID_HASHSIZE];
			m_fvCache = new float[GRID_DIM*GRID_DIM*GRID_DIM];
		}

		~CSIMDMPURunBody()
		{
			SAFE_DELETE(m_cptBlob);
			SAFE_DELETE(m_edgeTable);
			SAFE_DELETE(m_edgeTableSizes);
			SAFE_DELETE(m_fvCache);
		}

		void join(CSIMDMPURunBody& rhs)
		{
			m_ctExtra += rhs.m_ctExtra;
		}

		size_t getExtraCount() const {return m_ctExtra;}

		void operator()(const blocked_range<int>& range) 
		{			
			PREFETCH(m_cptBlob);
			PREFETCH(g_triTableCache);

			for(int i=range.begin(); i != range.end(); i++)
			{
				doMarchingCubes(m_vPartitions[i]);
				if(!m_vPartitions[i]->hasSurface())
					m_ctExtra++;
			}
		}
	};


	//////////////////////////////////////////////////////////////////////////
	class CParsipOptimized
	{
	private:
		DVec<CSIMDMPU*> m_lstMPUs;
		double m_tsStart;		
		double m_tsPolygonize;
		double m_tsSetup;
		//CLayer* m_layer;

		float m_inCellSize;
		float m_inIsoValue;
		COctree m_inOctree;
		COMPACTBLOBTREE* m_inlpCompactTree;
		int m_inAssignedID;


		//Removes all PUs that donot generate surface
		size_t removeExtraPUs();		
	public:
		CParsipOptimized()	{resetStats();}
		
		~CParsipOptimized()
		{
			removeAllMPUs();
		}

		void resetStats()
		{
			m_tsStart = 0.0;			
			m_tsPolygonize = 0.0;
			m_tsSetup = 0.0;
		}

		//Remove Layers		
		void removeAllMPUs();

		//Setup		
		void setup(COMPACTBLOBTREE* lpCompactBlobTree, 
				   const COctree& oct, 
				   int id = 0, 
				   float cellsize = DEFAULT_CELL_SIZE, 
				   float isovalue = ISO_VALUE);
		

		//Run
		void run();
		void drawMesh();
		void drawNormals(int normalLength);

		//Accessors
		CSIMDMPU* getMPU(int i) {return m_lstMPUs[i];}
		bool getMPUExtent(size_t idxMPU, vec3f& lo, vec3f& hi) const;
		size_t countMPUs() const { return m_lstMPUs.size();}


		/**
		* exports scattered mesh to a single CMeshVV instance for further manipulation
		* @param lpOutputMesh Pointer to the output mesh structure holding the result.
		* @return true if the function succeeds.
		*/
		bool exportMesh(CMeshVV* lpOutputMesh);

		//Stats
		double statsSetupTime() const{ return m_tsSetup;}
		double statsPolyTime() const{ return m_tsPolygonize;}
		double statsStartTime() const {return m_tsStart;}
		double statsEndTime() const {return m_tsStart + m_tsPolygonize;}

		CSIMDMPU*  statsLatestMPU() const;
		double statsLatestMPUTime() const;
		size_t statsIntersectedMPUs() const;		
		void   statsMeshInfo(size_t& ctVertices, size_t& ctFaces) const;		
		size_t statsTotalFieldEvals() const;

		size_t statsIntersectedCellsCount() const;
		size_t statsTotalCellInAllMPUs() const {return (GRID_DIM-1)*(GRID_DIM-1)*(GRID_DIM-1)*m_lstMPUs.size();}
		size_t statsTotalCellsInIntersectedMPUs() const {return (GRID_DIM-1)*(GRID_DIM-1)*(GRID_DIM-1)*statsIntersectedMPUs();}
		int	   statsCoreUtilizations(DVec<size_t>& arrOutThreadIDs, DVec<double>& arrOutUtilization);

	};
	

	/**
	* Polygonize a BlobTree by converting it to a compact tree and
	* running an optimized polygonization method. 
	* @param input The Blobtree input for polygonization
	* @param cellsize cube size parameter in space subdivision algorithm
	* @param isovalue the field value at the isosurface for which the mesh will be produced.
	* @return outputs a pointer to an instance of the polygonizer 
	*/
	CParsipOptimized* Run_Polygonizer(CBlobNode* input, float cellSize = DEFAULT_CELL_SIZE, float isovalue = ISO_VALUE);

	/**
	* Polygonize a BlobTree by converting it to a compact tree and
	* running an optimized polygonization method. 
	* @param input The Blobtree input for polygonization
	* @param lpMesh the pointer to the mesh object for the output mesh
	* @param cellsize cube size parameter in space subdivision algorithm
	* @param isovalue the field value at the isosurface for which the mesh will be produced.
	* @return true if succeeded.
	*/
	bool Run_PolygonizerExportMesh(CBlobNode* input, 
								   CMeshVV* lpMesh, 
								   float cellSize = DEFAULT_CELL_SIZE, 
								   float isovalue = ISO_VALUE);
}

#endif 


