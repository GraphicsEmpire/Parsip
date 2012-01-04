#ifndef POLYGONIZER_H
#define POLYGONIZER_H

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/enumerable_thread_specific.h"
#include "tbb/task_scheduler_observer.h"
#include "tbb/tick_count.h"

#include "PS_VectorMath.h"
#include "PS_SIMDVecN.h"

using namespace tbb;
using namespace PS::MATHFUNCTIONAL;
using namespace PS::MATHSIMD;

namespace PS{
namespace SIMDPOLY{


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

#define RET_PARAM_ERROR -1
#define RET_NOT_ENOUGH_MEM -2
#define RET_INVALID_BVH -3
#define RET_SUCCESS 1

#define CELLID_HASHSIZE (size_t)(1<<(3*CELLID_SHIFT_Y))
#define CELLID_FROM_IDX(i,j,k) ((((k) & CELLID_BITMASK) << CELLID_SHIFT_Z) | (((j) & CELLID_BITMASK) << CELLID_SHIFT_Y) | ((i) & CELLID_BITMASK))

#define EDGETABLE_DEPTH 8

//FieldValue Definitions
#define ISO_VALUE 0.5f
#define ISO_DIST  0.45420206f
#define FIELDVALUE_EPSILON 0.001f
#define NORMAL_DELTA 0.001f
#define DEFAULT_ROOTFINDER_ITERATIONS 5

//CellSize
#define MIN_CELL_SIZE	0.01f
#define MAX_CELL_SIZE	0.20f
#define DEFAULT_CELL_SIZE 0.14f

//BlobTree
#define MAX_TREE_NODES 128
#define MAX_MATRIX_COUNT MAX_TREE_NODES
#define PRIM_MATRIX_STRIDE 12
#define BOX_MATRIX_STRIDE 16

#define MAX_BVH_DEPTH_TO_MPU 6

//MPU
#define MAX_MPU_COUNT	24000
#define MAX_MPU_VERTEX_COUNT 512
#define MAX_MPU_TRIANGLE_COUNT 512
#define MAX_THREADS_COUNT 128

//////////////////////////////////////////////////////////
#ifndef BLOBTREE_DEFINITIONS
enum SkeletonType {sktPoint, sktLine, sktCylinder, sktDisc, sktRing, sktCube, sktTriangle};
enum OperatorType {opUnion, opIntersect, opDif, opSmoothDif, opBlend, opRicciBlend, opGradientBlend,
				   opAffine, opWarpTwist, opWarpTaper, opWarpBend, opWarpShear};
#endif

//Total Data Structure Per Each Core : Including Input BlobPrims, Ops, MPU mesh
//Aligned SOA structure for primitives
//SS = 18 * 128 * 4 + 2 * 128 * 1 + 7 * 4 = 9500 Bytes
struct PS_BEGIN_ALIGNED(PS_SIMD_FLEN) SOABlobPrims
{
	float posX[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float posY[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float posZ[PS_SIMD_PADSIZE(MAX_TREE_NODES)];

	float dirX[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float dirY[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float dirZ[PS_SIMD_PADSIZE(MAX_TREE_NODES)];

	float resX[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float resY[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float resZ[PS_SIMD_PADSIZE(MAX_TREE_NODES)];

	float colorX[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float colorY[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float colorZ[PS_SIMD_PADSIZE(MAX_TREE_NODES)];

	float vPrimBoxLoX[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float vPrimBoxLoY[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float vPrimBoxLoZ[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float vPrimBoxHiX[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float vPrimBoxHiY[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float vPrimBoxHiZ[PS_SIMD_PADSIZE(MAX_TREE_NODES)];

	U8 skeletType[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	U8 idxMatrix[PS_SIMD_PADSIZE(MAX_TREE_NODES)];

	svec3f bboxLo;
	svec3f bboxHi;
	U32 ctPrims;
} PS_END_ALIGNED(PS_SIMD_FLEN);

//Aligned SOA structure for operators
//SS= 5636
struct PS_BEGIN_ALIGNED(PS_SIMD_FLEN) SOABlobOps
{
	U8 opType[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	U8 opLeftChild[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
   	U8 opRightChild[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
   	U8 opChildKind[PS_SIMD_PADSIZE(MAX_TREE_NODES)];

	float vBoxLoX[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float vBoxLoY[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float vBoxLoZ[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float vBoxHiX[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float vBoxHiY[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float vBoxHiZ[PS_SIMD_PADSIZE(MAX_TREE_NODES)];

	float resX[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float resY[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float resZ[PS_SIMD_PADSIZE(MAX_TREE_NODES)];
	float resW[PS_SIMD_PADSIZE(MAX_TREE_NODES)];

	U32 ctOps;
} PS_END_ALIGNED(PS_SIMD_FLEN);


/*!
 * Matrices in primitive are aggregated matrices of their branch in BlobTree.
 * Most of the primitives has identity matrices.
 */
struct PS_BEGIN_ALIGNED(PS_SIMD_FLEN) SOABlobPrimMatrices
{
	float matrix[PS_SIMD_PADSIZE(MAX_MATRIX_COUNT)*PRIM_MATRIX_STRIDE];
	U32 count;
} PS_END_ALIGNED(PS_SIMD_FLEN);


/*!
 * Matrices to transform boxes
 */
struct PS_BEGIN_ALIGNED(PS_SIMD_FLEN) SOABlobBoxMatrices
{
	float matrix[PS_SIMD_PADSIZE(MAX_MATRIX_COUNT)*BOX_MATRIX_STRIDE];
	U32 count;
} PS_END_ALIGNED(PS_SIMD_FLEN);



//////////////////////////////////////////////////////////
//Each cell per each MPU will at most contribute to 5 triangles
//Number of Cells are (GRIM_DIM - 1)^ 3
//SS = 3 * 512 * 3 * 4 + 1 * 512 * 3 * 2 + 5 + 4 * 4 =  21528
struct MPU{
	//48 Bytes Per Each Entry
	float vPos[MAX_MPU_VERTEX_COUNT*3];
	float vNorm[MAX_MPU_VERTEX_COUNT*3];
	float vColor[MAX_MPU_VERTEX_COUNT*3];
	U16	  triangles[MAX_MPU_TRIANGLE_COUNT*3];

	//Number of vertices and triangles in output mesh
	U16	  ctVertices;
	U16	  ctTriangles;

	svec3f bboxLo;
	U32 ctFieldEvals;
};

struct PolyMPUs{
	MPU vMPUs[MAX_MPU_COUNT];
	U32 ctMPUs;
};

//Statistics for studying distribution of MPUs
struct MPUSTATS{
	int idxThread;
	int bIntersected;
	tbb_thread::id threadID;
	tbb::tick_count tickStart;
	tbb::tick_count tickEnd;
};

//////////////////////////////////////////////////////////
//SS = 5120
struct EDGETABLE
{
	//Check ctEdges to see if there is an edge associated with vertex s
	U8 ctEdges[GRID_DIM * GRID_DIM * GRID_DIM];

	//1 if corner (i,j,k) has edge with corners i+1, j+1, k+1
	U8 hasEdgeWithHiNeighbor[GRID_DIM * GRID_DIM * GRID_DIM * 3];

	//Each internal vertex is connected to at most 6 others
	//Each internal vertex has at most 3 adjacent vertices with higher address
	U16 idxVertices[GRID_DIM * GRID_DIM * GRID_DIM * 3];
};

//////////////////////////////////////////////////////////
template<int _size>
struct SIMPLESTACK
{
	SIMPLESTACK() { idxTop = -1;}
	void push(U32 id_)
	{
		assert(idxTop < _size);
		idxTop++;
		id[idxTop] = id_;
	}

	void pop() {idxTop--;}
	bool empty() { return (idxTop == -1);}
	U32 id[_size];
	int idxTop;
};

template<int _size>
struct KDTREE_TRAVERSE_STACK
{
	KDTREE_TRAVERSE_STACK() { idxTop = -1;}
	void push(U32 depth_, U32 id_)
	{
		assert(idxTop < _size);
		idxTop++;
		depth[idxTop] = depth_;
		id[idxTop] = id_;
	}

	void pop() {idxTop--;}
	bool empty() { return (idxTop == -1);}

	U32 depth[_size];
	U32 id[_size];
	int idxTop;
};


//////////////////////////////////////////////////////////

/*!
 * Computes fieldvalue, normal and gradient in the course of polygonization
 */
class FieldComputer
{
public:
	FieldComputer() {}
	FieldComputer(SOABlobPrims* lpPrims,
				  SOABlobBoxMatrices* lpPrimMatrices,
				  SOABlobOps* lpOps);

	void setup(SOABlobPrims* lpPrims,
			   SOABlobBoxMatrices* lpPrimMatrices,
			   SOABlobOps* lpOps);

	/*!
	 * Computes field-value using SIMD instructions for SIMD Length number of points.
	 * Can store field due to every node in the BlobTree.
	 * @param pX x coordinate for all points in SIMD
	 * @param pY y coordinate for all points in SIMD
	 * @param pZ z coordinate for all points in SIMD
	 * @param outField output field values for all points
	 * @param lpOutFieldPrims optional output for storing field for all primitives
	 * @param lpOutFieldOps optional output for storing field for all operators
	 */
	int fieldValue(const Float_& pX, const Float_& pY, const Float_& pZ,
				   Float_& outField, float* lpOutFieldPrims = NULL, float* lpOutFieldOps = NULL) const;

	int normal(const Float_& pX, const Float_& pY, const Float_& pZ,
			   const Float_& inFieldValue,
			   Float_& outNormalX, Float_& outNormalY, Float_& outNormalZ,
			   float delta) const;

	int gradient(const Float_& pX, const Float_& pY, const Float_& pZ,
	 		     const Float_& inFieldValue,
				 Float_& outGradX, Float_& outGradY, Float_& outGradZ,
				 float delta) const;


	int fieldValueAndColor(const Float_& pX, const Float_& pY, const Float_& pZ,
						   Float_& outField, Float_& outColorX, Float_& outColorY, Float_& outColorZ) const;

	int fieldValueAndGradient(const Float_& pX, const Float_& pY, const Float_& pZ,
						     Float_& outField, Float_& outGradX, Float_& outGradY, Float_& outGradZ,
							float delta) const;

	int computeRootNewtonRaphson(const svec3f& p1, const svec3f& p2,
								 float fp1, float fp2,
								 svec3f& output, float& outputField,
								 float target_field, int iterations) const;

public:
	static const U32 m_szConstFieldPadded = PS_SIMD_PADSIZE(MAX_TREE_NODES*PS_SIMD_FLEN);
	SOABlobPrims m_blobPrims;
	SOABlobPrimMatrices m_blobPrimMatrices;
	SOABlobOps 	 m_blobOps;

private:
	void computePrimitiveField(const Float_& pX, const Float_& pY, const Float_& pZ,
							   Float_& primField, U32 idxPrimitive) const;

};

/*!
 * @brief We need this task schedular observer to create per thread BlobTree data
 */
class ThreadStartSetup : public tbb::task_scheduler_observer
{
public:
	ThreadStartSetup(const SOABlobPrims* lpPrims,
			  	  	 const SOABlobPrimMatrices* lpMatrices,
			  	  	 const SOABlobOps* lpOps);


	void on_scheduler_entry(bool is_worker);
private:
	SOABlobPrims* 	m_lpBlobPrims;
	SOABlobOps* 	m_lpBlobOps;
	SOABlobPrimMatrices* m_lpBlobMatrices;
};


/*!
 * @brief Body code for parallel processing of MPUs on multi-core
 */
class CMPUProcessor
{
public:
	CMPUProcessor(float cellsize,
				  size_t ctMPUs, MPU* lpMPU, MPUSTATS* lpStats);

	void operator()(const blocked_range<size_t>& range) const;


	void process_cells_simd(const FieldComputer& fc, MPU& mpu) const;
        //void process_cells_fieldsimd_cellserial(const FieldComputer& fc, MPU& mpu) const;
        //void process_cells_scalar(const FieldComputer& fc, MPU& mpu) const;
private:

	/*!
	 * Get a vertex on an edge with the indices of one corner and its axis
	 * (0=x,1=y and 2=z)
	 */
	int getEdge(const EDGETABLE& edgeTable , int i, int j, int k, int edgeAxis) const;
	void setEdge(EDGETABLE& edgeTable , int i, int j, int k, int edgeAxis, int vid) const;

	int getEdge(const EDGETABLE& edgeTable, svec3i& start, svec3i& end) const;
	void setEdge(EDGETABLE& edgeTable, svec3i& start, svec3i& end, int vid) const;

private:

	MPUSTATS* m_lpStats;
	MPU* m_lpMPU;
	size_t m_ctMPUs;
	float m_cellsize;
};


//////////////////////////////////////////////////////////
U32 CountMPUNeeded(float cellsize, const svec3f& lo, const svec3f& hi);
int PrepareBBoxes(float cellsize, SOABlobPrims& blobPrims, SOABlobBoxMatrices& boxMatrices, SOABlobOps& blobOps);
int Polygonize(float cellsize,
			   const SOABlobPrims& blobPrims,
			   const SOABlobPrimMatrices& blobPrimMatrices,
			   const SOABlobOps& blobOps,
			   PolyMPUs& polyMPUs,
			   MPUSTATS* lpProcessStats = NULL);

void PrintThreadResults(int ctAttempts, U32* lpThreadProcessed = NULL, U32* lpThreadCrossed = NULL);



inline void ComputeWyvillFieldValueSquare_(const Float_& dd, Float_& arrFields)
{
	Float_ allOne(1.0f);
	Float_ allZero(0.0f);
	Float_ res;
	res = (allOne - dd);
	arrFields = (res * res) * res;

	//Field less than zero is undefined. in case distance is greater than one
	arrFields = SimdMax(allZero, arrFields);
}

//Handy Functions
inline float ComputeWyvillFieldValueSquare(float dd)
{
	if(dd >= 1.0f)
		return 0.0f;
	float t = (1.0f - dd);
	return t*t*t;
}

//Returns the distance to the skeleton
inline float ComputeWyvillIsoDistance(float fv)
{
	if(fv == 0.0f) return 1.0f;
	float oneThird = 1.0f / 3.0f;
	return FastSqrt(1.0f - std::pow(fv, oneThird));
}

inline bool BoxIntersect(const svec3f& lo1, const svec3f& hi1,
   					     const svec3f& lo2, const svec3f& hi2)
{
	if ((lo1.x >= hi2.x) || (hi1.x <= lo2.x))
		return false;
	if ((lo1.y >= hi2.y) || (hi1.y <= lo2.y))
		return false;
	if ((lo1.z >= hi2.z) || (hi1.z <= lo2.z))
		return false;

	return true;
}


}
}


#endif
