#ifndef ADAPTIVE_UNIFORM_GRID
#define ADAPTIVE_UNIFORM_GRID

#include "CBlobTree.h"
#include <vector>
#include <map>
#include <set>
#include "CVolumeBox.h"

#include "PS_FrameWork/include/_parsDebug.h"

namespace PS{
namespace BLOBTREE {
using namespace VOL;

class CAdaptiveUniformGrid3D
{
public:

    class Block {
    public:
        unsigned int nBlockID;
        float * pCells;

        Block() { pCells = NULL; }

        Block(int nDim) {
            nBlockID = 0;
            pCells = new float[nDim*nDim*nDim];
            for (int i = 0; i < nDim*nDim*nDim; ++i)
                pCells[i] = CAdaptiveUniformGrid3D::INVALID_VALUE;
        }

        Block( int nDim, const Block & pCopy ) {
            nBlockID = pCopy.nBlockID;
            pCells = new float[nDim*nDim*nDim];
            memcpy(pCells, pCopy.pCells, sizeof(float)*nDim*nDim*nDim);
        }

        ~Block() { if (pCells) delete pCells; }

        void Invalidate( int nDim, const Block & pInvalid ) {
            memcpy( pCells, pInvalid.pCells, sizeof(float)*nDim*nDim*nDim);
        }
    };


    CAdaptiveUniformGrid3D( float fVoxelSize, const vec3f & vOrigin );
    virtual ~CAdaptiveUniformGrid3D();


    void SetOrigin( const vec3f & vNewOrigin );
    inline const vec3f & GetOrigin() const;

    // [TODO: this really should be based on feature size..]
    void SetVoxelSize( const PS::COctree& fieldBounds, unsigned int nMaxCells, bool bForce = false );

    inline float GetVoxelSize() const;
    inline float GetBlockSize() const;


    /*
  * evaluation functions
  */


    void SetValue( float x, float y, float z, float fValue );
    float GetValue( float x, float y, float z );


    float SampleTriLinear( float x, float y, float z, CBlobNode * pRoot);
    float SampleTriQuadratic( float x, float y, float z, CBlobNode * pRoot);

    void SampleGradientTriLinear( float x, float y, float z,
                                  float & gradX, float & gradY, float & gradZ,
                                  float * pFieldValue, CBlobNode * pRoot);
    void SampleGradientTriQuadratic( float x, float y, float z,
                                     float & gradX, float & gradY, float & gradZ,
                                     float * pFieldValue, CBlobNode * pRoot);


    // invalidation funcs
    void Invalidate( bool bDiscardMemory = true );
    void Invalidate(CVolumeBox & Bounds );


    // profiling funcs
    void CalculateCellAllocationStats( const CVolumeBox & bounds,
                                       unsigned int & nBlocksTotal, unsigned int & nBlocksAllocated,
                                       unsigned int & nCellsTotal, unsigned int & nCellsAllocated );
    void CalculateCellEvaluationStats(  const CVolumeBox & bounds,
                                        unsigned int & nCellsTotalGrid, unsigned int & nCellsTotalAllocated, unsigned int & nCellsEvaluated );

protected:

    float m_fVoxelSize;
    float m_fBlockSize;

    vec3f m_vOrigin;		// origin of grid passed in by user

    float m_vGridOrigin[3];			// bottom-left corner of grid in real space
    float m_fVoxelScale;

    // we could probably really improve lookup times here with a "last-block" cache...
    //  need to subclass std::map...
    std::map<unsigned int, Block *> m_blocks;

    // last-block cache. 50% speedup for trilinear interpolation...
    Block * m_pLastBlock;
    unsigned int m_nLastBlockID;

    // last-cell cache for trilinear value...
    struct LastCellCache {
        int celli;
        int cellj;
        int cellk;
        float fV000;
        float fV001;
        float fV010;
        float fV011;
        float fV100;
        float fV101;
        float fV110;
        float fV111;
    } m_lastCellCache;
    void InvalidateLastCellCache();

    static const unsigned int BLOCK_CELL_SIZE;
    static const unsigned int BLOCK_CELL_LOG2SIZE;
    static const unsigned int BLOCK_CELL_BITMASK;

    static const unsigned int BLOCK_COORD_BITS;
    static const unsigned int BLOCK_COORD_BITMASK;
    static const unsigned int MAXBLOCKS;

    static const float INVALID_VALUE;
    static const unsigned int INVALID_COORD;


    float GetValueOrCache( int nCellX, int nCellY, int nCellZ, CBlobNode* pField);
    float CacheValueAndReturn( Block * pBlock, int celli, int cellj, int cellk, CBlobNode* pField);

    inline Block * FindBlock( unsigned int nBlockID );
    inline Block * FindBlockOrCreate( unsigned int nBlockID );


    // use this for fast memcpy
    static Block * s_pInvalidBlock;
    static Block * GetInvalidBlock();


    //	float SampleTriLinear_SingleBlock float x, float y, float z, ScalarField * pField );
    //	float SampleTriLinear_Standard( float x, float y, float z, ScalarField * pField );
    //float SampleTriQuadratic( float x, float y, float z, ScalarField * pField );


    // deprecated...
    float GetValueOrCache( float x, float y, float z, CBlobNode* pField);
};



inline float CAdaptiveUniformGrid3D::GetVoxelSize() const
{
    return m_fVoxelSize;
}

inline float CAdaptiveUniformGrid3D::GetBlockSize() const
{
    return m_fBlockSize;
}

inline const vec3f & CAdaptiveUniformGrid3D::GetOrigin() const
{
    return m_vOrigin;
}

inline CAdaptiveUniformGrid3D::Block * CAdaptiveUniformGrid3D::FindBlock( unsigned int nBlockID )
{
    if (nBlockID == m_nLastBlockID)
        return m_pLastBlock;
    m_nLastBlockID = nBlockID;
    m_pLastBlock = m_blocks[nBlockID];
    return m_pLastBlock;
}

inline CAdaptiveUniformGrid3D::Block * CAdaptiveUniformGrid3D::FindBlockOrCreate( unsigned int nBlockID )
{
    if (m_nLastBlockID != nBlockID) {
        m_pLastBlock = m_blocks[nBlockID];
        m_nLastBlockID = nBlockID;
    }

    if (m_pLastBlock == NULL) {
        m_pLastBlock = new CAdaptiveUniformGrid3D::Block(BLOCK_CELL_SIZE, *GetInvalidBlock());
        m_pLastBlock->nBlockID = nBlockID;
        m_blocks[nBlockID] = m_pLastBlock;
    }

    return m_pLastBlock;
}

}
} 

#endif 
