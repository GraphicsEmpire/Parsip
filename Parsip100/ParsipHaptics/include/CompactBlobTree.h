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
#define MIN_BLOB_NODES 32
#define MAX_COMPACT_KIDS_COUNT 1024
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
    int orgID;
    BlobNodeType type;
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
    int orgID;
    int ctKids;
    //int kidIds[MAX_COMPACT_KIDS_COUNT];
    U16 kidIds[MAX_COMPACT_KIDS_COUNT];
    U8  kidIsOp[MAX_COMPACT_KIDS_COUNT];
    vec4f params;
    vec4f octLo;
    vec4f octHi;
    //	TreeNodeCache fvCache;
};

struct PCMCONTEXT
{
    int idPCM;
    float maxCompressionLeft;
    float maxCompressionRight;

    vec4f forceLeft;
    vec4f forceRight;
};

//Compact BlobTree
//template<int szNodes>
class COMPACTBLOBTREE
{
private:
    int m_ctPrims;
    int m_szAllocatedPrims;
    BlobPrimitive* m_lpPrims;

    int m_ctOps;
    int m_szAllocatedOps;
    BlobOperator* m_lpOps;

    int m_ctPCMNodes;
    PCMCONTEXT m_pcmCONTEXT;
public:

    COMPACTBLOBTREE()
    {
        init();
    }

    COMPACTBLOBTREE(const COMPACTBLOBTREE& rhs)
    {
        init();
        copyFrom(rhs);
    }

    COMPACTBLOBTREE(CBlobNode* root)
    {
        init();
        convert(root);
    }

    ~COMPACTBLOBTREE()
    {
        SAFE_DELETE(m_lpPrims);
        SAFE_DELETE(m_lpOps);
    }

    //Count
    inline int countPrimitives() const {return m_ctPrims;}
    inline int countOperators() const {return m_ctOps;}
    inline BlobPrimitive& getPrimitive(int i) {return m_lpPrims[i];}
    inline BlobOperator& getOperator(int i) {return m_lpOps[i];}

    //FieldValue
    float fieldvalue(const vec4f& p, float* lpStoreFVOp = NULL, float* lpStoreFVPrim = NULL);
    float fieldvaluePrim(const vec4f& p, int id, float* lpStoreFVPrim = NULL);
    float fieldvalueOp(const vec4f& p, int id, float* lpStoreFVOp = NULL, float* lpStoreFVPrim = NULL);
    vec4f gradientAtNode(bool isOp, int id, const vec4f& p, float fp, float delta );
    float fieldAtNode(bool isOp, int id, const vec4f& p);
    int   getID(bool isOp, int treeOrgID);
    //float fieldValueAndGradientAtNodecopyFrom(int id, const vec4f& p, vec4f& grad, float delta );

    float computePCM(const vec4f& p,
                     const vec4f& pcmParam,
                     const vec4f& oct1Lo,
                     const vec4f& oct1Hi,
                     const vec4f& oct2Lo,
                     const vec4f& oct2Hi,
                     int idSelf,
                     int idChild1, int idChild2,
                     U8 isOpChild1, U8 isOpChild2,
                     float fp1, float fp2);

    vec3f marchTowardNode(bool isOp, int id, const vec3f& p, const vec3f& grad, float& fp);
    //vec3f marchTowardNode(bool isOp, int id, vec3f& p, float& fp);
    float computePropagationDeformation(float dist, float k, float a0, float w);

    //Base Color
    vec4f baseColor(const vec4f& p, float* lpStoreFVOp = NULL, float* lpStoreFVPrim = NULL);
    vec4f baseColorOp(const vec4f& p, int id, float* lpStoreFVOp = NULL, float* lpStoreFVPrim = NULL);

    //Output normal vector
    vec4f normal(const vec4f& p, float inFieldValue, float delta);

    //Outputs gradient in [x, y, z] and fieldvalue in w part
    vec4f fieldValueAndGradient(const vec4f& p, float delta);

    int convert(CBlobNode* root);
    void copyFrom(const COMPACTBLOBTREE& rhs);
private:
    void init();

    int convert(CBlobNode* root, int parentID /*, const CMatrix& mtxBranch*/);
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

/*!
 * Return true if two bounding boxes intersect.
 */
/*
inline bool intersects( const vec4f& lo1, const vec4f& hi1, const vec4f& lo2, const vec4f& hi2 )
{
        if ((lo1.x >= hi2.x) || (hi1.x <= lo2.x))
                return false;
        if ((lo1.y >= hi2.y) || (hi1.y <= lo2.y))
                return false;
        if ((lo1.z >= hi2.z) || (hi1.z <= lo2.z))
                return false;
        return true;
}
*/

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
