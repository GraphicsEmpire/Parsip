#ifndef COMPACT_BLOBTREE_H
#define COMPACT_BLOBTREE_H
#include "PS_BlobTree/include/CBlobTree.h"
#include "PS_BlobTree/include/CSkeleton.h"
#include "PS_BlobTree/include/CSkeletonPrimitive.h"
//#include "PS_FrameWork/include/PS_SIMDVec.h"
#include "PS_FrameWork/include/PS_Interval.h"

using namespace PS::BLOBTREE;
using namespace PS::MATH;

//#define MAX_BLOB_ENTRIES PS_SIMD_LINES*3
#define MAX_BLOB_ENTRIES 100
#define MAX_COMPACT_KIDS_COUNT 8
#define MAX_TREENODE_FVCACHE   4
#define TREENODE_CACHE_STORETHRESHOLD   0.4f

struct TreeNodeCache
{
	vec4f xyzf[MAX_TREENODE_FVCACHE];
	float hashVal[MAX_TREENODE_FVCACHE];
	int ctFilled;
};

//Compact Structure for all primitives
struct BlobPrimitive
{
	int parent;
	SkeletonType skelet;
	vec4f color;
	vec4f pos;
	vec4f dir;
	vec4f res1;
	vec4f res2;
	vec4f octLo;
	vec4f octHi;	
	vec4f mtxBackwardR0;
	vec4f mtxBackwardR1;
	vec4f mtxBackwardR2;
	vec4f mtxBackwardR3;	
//	TreeNodeCache fvCache;
};

//Compact Structure for all operators
struct BlobOperator
{
	BlobNodeType type;
	int parent;
	int ctKids;
	//int kidIds[MAX_COMPACT_KIDS_COUNT];
	DVec<int> kidIds;
	vec4f params;	
	vec4f octLo;
	vec4f octHi;
//	TreeNodeCache fvCache;
};

//Compact BlobTree
class COMPACTBLOBTREE
{
public:
	int ctPrims;

	BlobPrimitive prims[MAX_BLOB_ENTRIES];
	int ctOps;
	BlobOperator ops[MAX_BLOB_ENTRIES];
	
	COMPACTBLOBTREE()
	{
		reset();
	}

	COMPACTBLOBTREE(const COMPACTBLOBTREE& rhs)
	{
		reset();
		copyFrom(rhs);
	}

	COMPACTBLOBTREE(CBlobTree* root)
	{
		reset();
		convert(root);
	}


	//FieldValue
	float fieldvalue(const vec4f& p, float* lpStoreFVOp = NULL, float* lpStoreFVPrim = NULL);	
	float fieldvaluePrim(const vec4f& p, int id, float* lpStoreFVPrim = NULL);
	float fieldvalueOp(const vec4f& p, int id, float* lpStoreFVOp = NULL, float* lpStoreFVPrim = NULL);

	//Base Color	
	vec4f baseColor(const vec4f& p, float* lpStoreFVOp = NULL, float* lpStoreFVPrim = NULL);		
	vec4f baseColorOp(const vec4f& p, int id, float* lpStoreFVOp = NULL, float* lpStoreFVPrim = NULL);		

	//Output normal vector
	vec4f normal(const vec4f& p, float inFieldValue, float delta);

	//Outputs gradient in [x, y, z] and fieldvalue in w part
	vec4f fieldValueAndGradient(const vec4f& p, float delta);

	void reset()
	{
		ctPrims = 0; 
		ctOps = 0;
	}

	int convert(CBlobTree* root);
	void copyFrom(const COMPACTBLOBTREE& rhs);
private:
	int convert(CBlobTree* root, int parentID /*, const CMatrix& mtxBranch*/);
	vec4f warpBend( const vec4f& pin, float bendRate, float bendCenter, const CInterval& bendRegion);
	vec4f warpTwist(const vec4f& pin, float factor, MajorAxices axis);

	vec3f taperAlongX(vec3f p, float factor, MajorAxices axisTaper);
	vec3f taperAlongY(vec3f p, float factor, MajorAxices axisTaper);
	vec3f taperAlongZ(vec3f p, float factor, MajorAxices axisTaper);
	vec4f warpTaper(const vec4f& pin, float factor, MajorAxices axisAlong, MajorAxices axisTaper);

	vec3f shearAlongX(vec3f p, float factor, MajorAxices axisDependent);
	vec3f shearAlongY(vec3f p, float factor, MajorAxices axisDependent);
	vec3f shearAlongZ(vec3f p, float factor, MajorAxices axisDependent);
	vec4f warpShear(const vec4f& pin, float factor, MajorAxices axisAlong, MajorAxices axisDependent);

};

//////////////////////////////////////////////////////////////////////////
/*
int ComputeRootNewtonRaphsonSIMD(COMPACTBLOBTREE* cptBlob, 
								 const svec4f& p1, const svec4f& p2, 
								 float fp1, float fp2, 
								 svec4f& output, float& outputField,
								 float target_field = ISO_VALUE, int iterations = DEFAULT_ITERATIONS);
								 */

int ComputeRootNewtonRaphsonVEC4(COMPACTBLOBTREE* cptBlob, 
								 float* lpStoreFVOps, 
								 float* lpStoreFVPrims,
								 const vec4f& p1, const vec4f& p2, 
								 float fp1, float fp2, 
								 vec4f& output, float& outputField,
								 float target_field = ISO_VALUE, int iterations = DEFAULT_ITERATIONS);





#endif