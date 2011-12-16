#include "CAdaptiveUniformGrid3d.h"

#include "PS_FrameWork/include/_parsProfile.h"

#include <limits>

using namespace PS::BLOBTREE;

const float CAdaptiveUniformGrid3D::INVALID_VALUE = numeric_limits<float>::infinity();
const unsigned int CAdaptiveUniformGrid3D::INVALID_COORD = 0xFFFFFFFF;

#define SIZE_8

// 8 cells along each axis of a block
#ifdef SIZE_8
const unsigned int CAdaptiveUniformGrid3D::BLOCK_CELL_SIZE = 8;
const unsigned int CAdaptiveUniformGrid3D::BLOCK_CELL_LOG2SIZE = 3;
const unsigned int CAdaptiveUniformGrid3D::BLOCK_CELL_BITMASK = 0x7;
#else
const unsigned int CAdaptiveUniformGrid3D::BLOCK_CELL_SIZE = 4;
const unsigned int CAdaptiveUniformGrid3D::BLOCK_CELL_LOG2SIZE = 2;
const unsigned int CAdaptiveUniformGrid3D::BLOCK_CELL_BITMASK = 0x3;
#endif

// 10 bits for each coord of the block ID. Limits to 1024 blocks along an axis (8096 cells) - should be lots...
const unsigned int CAdaptiveUniformGrid3D::BLOCK_COORD_BITS = 10;
const unsigned int CAdaptiveUniformGrid3D::BLOCK_COORD_BITMASK = 0x3FF;
const unsigned int CAdaptiveUniformGrid3D::MAXBLOCKS = 1024;

// #defines of above things for the macros
#define _BLOCK_ID_BITS_PER_COORD 10
#define _BLOCK_ID_SHIFT_Y _BLOCK_ID_BITS_PER_COORD
#define _BLOCK_ID_SHIFT_Z _BLOCK_ID_BITS_PER_COORD*2
#define _BLOCK_ID_BITMASK 0x3FF
#ifdef SIZE_8
#define _CELL_SHIFT 3
#define _CELL_BITMASK 0x7
#else
#define _CELL_SHIFT 2
#define _CELL_BITMASK 0x3
#endif


#define _BLOCK_ID(i,j,k) (((k) & _BLOCK_ID_BITMASK) << _BLOCK_ID_SHIFT_Z) | (((j) & _BLOCK_ID_BITMASK) << _BLOCK_ID_SHIFT_Y) | ((i) & _BLOCK_ID_BITMASK)

// this one can be improved I think....probably can do it with one shift (!)
#define _BLOCK_ID_FROM_CELL(i,j,k) _BLOCK_ID( (i)>>_CELL_SHIFT, (j)>>_CELL_SHIFT, (k)>>_CELL_SHIFT )

#define _BLOCK_ID_X(id) ( (id) & _BLOCK_ID_BITMASK )
#define _BLOCK_ID_Y(id) ( ((id) >> _BLOCK_ID_SHIFT_Y) & _BLOCK_ID_BITMASK )
#define _BLOCK_ID_Z(id) ( ((id) >> _BLOCK_ID_SHIFT_Z) & _BLOCK_ID_BITMASK )

#define _CELL_INDEX_BLOCK(i, j, k) ( (k)*BLOCK_CELL_SIZE*BLOCK_CELL_SIZE + (j)*BLOCK_CELL_SIZE + (i) )
//#define _CELL_INDEX_GLOBAL(i, j, k) _CELL_INDEX_BLOCK( (k)&_CELL_BITMASK, (j)&_CELL_BITMASK, (i)&_CELL_BITMASK )
#define _CELL_INDEX_GLOBAL(i, j, k) _CELL_INDEX_BLOCK( (i)&_CELL_BITMASK, (j)&_CELL_BITMASK, (k)&_CELL_BITMASK )
#define _CELL_GLOBAL_TO_LOCAL(coord) ((coord)&_CELL_BITMASK)




CAdaptiveUniformGrid3D::Block * CAdaptiveUniformGrid3D::s_pInvalidBlock = NULL;

CAdaptiveUniformGrid3D::CAdaptiveUniformGrid3D( float fVoxelSize, const vec3f & vOrigin )
{
	m_vOrigin = vOrigin;
	m_fVoxelSize = fVoxelSize;
	m_fBlockSize = fVoxelSize * (float)BLOCK_CELL_SIZE;

	m_fVoxelScale = (float)(1.0 / (double)m_fVoxelSize);

	// negate grid origin here so we can add it later. Reduces cancellation unless
	//  origin is heavily negative...
	SetOrigin( vOrigin );

        m_nLastBlockID = static_cast<U32>(1<<32);	// invalid!
	m_pLastBlock = NULL;

	InvalidateLastCellCache();
}

CAdaptiveUniformGrid3D::~CAdaptiveUniformGrid3D()
{
	std::map<unsigned int, Block *>::iterator cur(m_blocks.begin()), end(m_blocks.end());
	while (cur != end) 
		delete (*cur++).second;
}


CAdaptiveUniformGrid3D::Block * CAdaptiveUniformGrid3D::GetInvalidBlock()
{
	if (s_pInvalidBlock == NULL)
		s_pInvalidBlock = new Block(BLOCK_CELL_SIZE);
	return s_pInvalidBlock;
}


void CAdaptiveUniformGrid3D::SetOrigin( const vec3f & vNewOrigin )
{
	m_vOrigin = vNewOrigin;
	m_vGridOrigin[0] = - ( vNewOrigin.x - (m_fBlockSize * (float)(MAXBLOCKS/2)) );
	m_vGridOrigin[1] = - ( vNewOrigin.y - (m_fBlockSize * (float)(MAXBLOCKS/2)) );
	m_vGridOrigin[2] = - ( vNewOrigin.z - (m_fBlockSize * (float)(MAXBLOCKS/2)) );
}

void CAdaptiveUniformGrid3D::SetVoxelSize( const PS::COctree &fieldBounds, unsigned int nGridDim, bool bForce )
{
        vec3f lower = fieldBounds.lower;
        vec3f upper = fieldBounds.upper;
	float fWidth  = upper.x - lower.x;
	float fHeight = upper.y - lower.y;
	float fDepth  = upper.z - lower.z;

	float fMaxDim = std::max<float>( fWidth, std::max<float>(fHeight,fDepth) );
	float fVoxelSize = fMaxDim / (float)(nGridDim-1);
	
	// if voxel size is < current voxel size, re-set voxel size and clear existing grid...
	float fRatio = fVoxelSize / m_fVoxelSize;
	if ( bForce || m_fVoxelSize < 0.00001f ||  fRatio < 0.5f || fRatio > 2.0f ) {

		m_fVoxelSize = fVoxelSize;
		m_fBlockSize = fVoxelSize * (float)BLOCK_CELL_SIZE;
		m_fVoxelScale = (float)(1.0 / (double)m_fVoxelSize);
		SetOrigin( m_vOrigin );

		Invalidate(true);
	}
}



void CAdaptiveUniformGrid3D::SetValue( float x, float y, float z, float fValue )
{
	int celli = (int)( (x + m_vGridOrigin[0]) * m_fVoxelScale );
	int cellj = (int)( (y + m_vGridOrigin[1]) * m_fVoxelScale );
	int cellk = (int)( (z + m_vGridOrigin[2]) * m_fVoxelScale );

	int nBlockID = _BLOCK_ID_FROM_CELL(celli, cellj, cellk);
	Block * pBlock = FindBlockOrCreate(nBlockID);

	pBlock->pCells[ _CELL_INDEX_GLOBAL(celli, cellj, cellk) ] = fValue;
}


float CAdaptiveUniformGrid3D::GetValue( float x, float y, float z )
{
	int celli = (int)( (x + m_vGridOrigin[0]) * m_fVoxelScale );
	int cellj = (int)( (y + m_vGridOrigin[1]) * m_fVoxelScale );
	int cellk = (int)( (z + m_vGridOrigin[2]) * m_fVoxelScale );

	int nBlockID = _BLOCK_ID_FROM_CELL(celli, cellj, cellk);
	Block * pBlock = FindBlock(nBlockID);
	if (pBlock != NULL)
		return pBlock->pCells[ _CELL_INDEX_GLOBAL(celli, cellj, cellk) ];
	else
		return 0.0;
}

float CAdaptiveUniformGrid3D::GetValueOrCache( float x, float y, float z, CBlobNode * pField )
{
	int celli = (int)( (x + m_vGridOrigin[0]) * m_fVoxelScale );
	int cellj = (int)( (y + m_vGridOrigin[1]) * m_fVoxelScale );
	int cellk = (int)( (z + m_vGridOrigin[2]) * m_fVoxelScale );

	int nBlockID = _BLOCK_ID_FROM_CELL(celli, cellj, cellk);

	if ( _BLOCK_ID_X(nBlockID) > 1000 || _BLOCK_ID_Y(nBlockID) > 1000 || _BLOCK_ID_Z(nBlockID) > 1000)
		parsDebugInfo("BAD BLOCK ID!\n");

	Block * pBlock = FindBlockOrCreate(nBlockID);

	if ( pBlock->pCells[ _CELL_INDEX_GLOBAL(celli, cellj, cellk) ] == INVALID_VALUE ) 
		pBlock->pCells[ _CELL_INDEX_GLOBAL(celli, cellj, cellk) ] = pField->fieldValue(x,y,z);

	return pBlock->pCells[ _CELL_INDEX_GLOBAL(celli, cellj, cellk) ];
}


float CAdaptiveUniformGrid3D::GetValueOrCache( int celli, int cellj, int cellk, CBlobNode * pField )
{
	int nBlockID = _BLOCK_ID_FROM_CELL(celli, cellj, cellk);
	Block * pBlock = FindBlockOrCreate(nBlockID);

	if ( pBlock->pCells[ _CELL_INDEX_GLOBAL(celli, cellj, cellk) ] == INVALID_VALUE ) {
		float fX = (celli >> BLOCK_CELL_LOG2SIZE) * m_fBlockSize - m_vGridOrigin[0] + (float)(celli & BLOCK_CELL_BITMASK) * m_fVoxelSize;
		float fY = (cellj >> BLOCK_CELL_LOG2SIZE) * m_fBlockSize - m_vGridOrigin[1] + (float)(cellj & BLOCK_CELL_BITMASK) * m_fVoxelSize;
		float fZ = (cellk >> BLOCK_CELL_LOG2SIZE) * m_fBlockSize - m_vGridOrigin[2] + (float)(cellk & BLOCK_CELL_BITMASK) * m_fVoxelSize;
		pBlock->pCells[ _CELL_INDEX_GLOBAL(celli, cellj, cellk) ] = pField->fieldValue(fX,fY,fZ);
	}

	return pBlock->pCells[ _CELL_INDEX_GLOBAL(celli, cellj, cellk) ];
}


float CAdaptiveUniformGrid3D::CacheValueAndReturn( Block * pBlock, int celli, int cellj, int cellk, CBlobNode * pField )
{
	float fX = _BLOCK_ID_X(pBlock->nBlockID) * m_fBlockSize - m_vGridOrigin[0] + (float)celli * m_fVoxelSize;
	float fY = _BLOCK_ID_Y(pBlock->nBlockID) * m_fBlockSize - m_vGridOrigin[1] + (float)cellj * m_fVoxelSize;
	float fZ = _BLOCK_ID_Z(pBlock->nBlockID) * m_fBlockSize - m_vGridOrigin[2] + (float)cellk * m_fVoxelSize;
	int nCellIndex = _CELL_INDEX_BLOCK(celli, cellj, cellk);
	pBlock->pCells[ nCellIndex ] = pField->fieldValue(fX, fY, fZ);
	return pBlock->pCells[ nCellIndex ];
}



#if 1

// This version tries to be more clever. When we can guarantee that all accesses are confined
// to a single block (about 65% of cases), we can do the value lookups directly. Still has to 
// drop to a function  call if the value is not there...
//
// Could try to optimize it so it would do this for each coordinate - ie, for each fV???, there is
// a specific case. But all the extra branches might kill any benefit...
//
// also uses last-cell cache, which gives a decent speedup. It gives the most benefit when
// doing things like binary searches inside one voxel (happens often for MC polygonizers, etc).
// Otherwise causes some overhead, but I think it's negligible...

float CAdaptiveUniformGrid3D::SampleTriLinear( float x, float y, float z, CBlobNode * pField )
{
	int celli = (int)( (x + m_vGridOrigin[0]) * m_fVoxelScale );
	int cellj = (int)( (y + m_vGridOrigin[1]) * m_fVoxelScale );
	int cellk = (int)( (z + m_vGridOrigin[2]) * m_fVoxelScale );

	// check that the last-cell cache is valid...
	if (m_lastCellCache.celli != celli || m_lastCellCache.cellj != cellj || m_lastCellCache.cellk != cellk ) {

		// get local coords
		int voxeli = _CELL_GLOBAL_TO_LOCAL(celli);
		int voxelj = _CELL_GLOBAL_TO_LOCAL(cellj);
		int voxelk = _CELL_GLOBAL_TO_LOCAL(cellk);

		if ( voxeli < BLOCK_CELL_SIZE-1 && voxelj < BLOCK_CELL_SIZE-1 && voxelk < BLOCK_CELL_SIZE-1 ) {

			// get block
			Block * pBlock = FindBlockOrCreate( _BLOCK_ID_FROM_CELL(celli, cellj, cellk) );

			// for each vertex, create if it doesn't exist. Then get value
			m_lastCellCache.fV000 = pBlock->pCells[ _CELL_INDEX_BLOCK(voxeli, voxelj, voxelk) ];
			if (m_lastCellCache.fV000 == INVALID_VALUE)
				m_lastCellCache.fV000 = CacheValueAndReturn( pBlock, voxeli, voxelj, voxelk, pField );

			m_lastCellCache.fV001 = pBlock->pCells[ _CELL_INDEX_BLOCK(voxeli, voxelj, voxelk+1) ];
			if (m_lastCellCache.fV001 == INVALID_VALUE)
				m_lastCellCache.fV001 = CacheValueAndReturn( pBlock, voxeli, voxelj, voxelk+1, pField );

			m_lastCellCache.fV010 = pBlock->pCells[ _CELL_INDEX_BLOCK(voxeli, voxelj+1, voxelk) ];
			if (m_lastCellCache.fV010 == INVALID_VALUE)
				m_lastCellCache.fV010 = CacheValueAndReturn( pBlock, voxeli, voxelj+1, voxelk, pField );

			m_lastCellCache.fV011 = pBlock->pCells[ _CELL_INDEX_BLOCK(voxeli, voxelj+1, voxelk+1) ];
			if (m_lastCellCache.fV011 == INVALID_VALUE)
				m_lastCellCache.fV011 = CacheValueAndReturn( pBlock, voxeli, voxelj+1, voxelk+1, pField );

			m_lastCellCache.fV100 = pBlock->pCells[ _CELL_INDEX_BLOCK(voxeli+1, voxelj, voxelk) ];
			if (m_lastCellCache.fV100 == INVALID_VALUE)
				m_lastCellCache.fV100 = CacheValueAndReturn( pBlock, voxeli+1, voxelj, voxelk, pField );

			m_lastCellCache.fV101 = pBlock->pCells[ _CELL_INDEX_BLOCK(voxeli+1, voxelj, voxelk+1) ];
			if (m_lastCellCache.fV101 == INVALID_VALUE)
				m_lastCellCache.fV101 = CacheValueAndReturn( pBlock, voxeli+1, voxelj, voxelk+1, pField );

			m_lastCellCache.fV110 = pBlock->pCells[ _CELL_INDEX_BLOCK(voxeli+1, voxelj+1, voxelk) ];
			if (m_lastCellCache.fV110 == INVALID_VALUE)
				m_lastCellCache.fV110 = CacheValueAndReturn( pBlock, voxeli+1, voxelj+1, voxelk, pField );

			m_lastCellCache.fV111 = pBlock->pCells[ _CELL_INDEX_BLOCK(voxeli+1, voxelj+1, voxelk+1) ];
			if (m_lastCellCache.fV111 == INVALID_VALUE)
				m_lastCellCache.fV111 = CacheValueAndReturn( pBlock, voxeli+1, voxelj+1, voxelk+1, pField );

		} else {

			m_lastCellCache.fV000 = GetValueOrCache(celli, cellj, cellk        , pField);
			m_lastCellCache.fV001 = GetValueOrCache(celli, cellj, cellk+1      , pField);
			m_lastCellCache.fV010 = GetValueOrCache(celli, cellj+1, cellk      , pField);
			m_lastCellCache.fV011 = GetValueOrCache(celli, cellj+1, cellk+1    , pField);
			m_lastCellCache.fV100 = GetValueOrCache(celli+1, cellj, cellk      , pField);
			m_lastCellCache.fV101 = GetValueOrCache(celli+1, cellj, cellk+1    , pField);
			m_lastCellCache.fV110 = GetValueOrCache(celli+1, cellj+1, cellk    , pField);
			m_lastCellCache.fV111 = GetValueOrCache(celli+1, cellj+1, cellk+1  , pField);
		}

		m_lastCellCache.celli = celli;
		m_lastCellCache.cellj = cellj;
		m_lastCellCache.cellk = cellk;
	}

	// can probably improve this...
	int xi = celli >> BLOCK_CELL_LOG2SIZE;
	float fLeft = xi * m_fBlockSize - m_vGridOrigin[0] + (float)(celli & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAx = (x - fLeft) * m_fVoxelScale;
	int yi = cellj >> BLOCK_CELL_LOG2SIZE;
	float fBottom = yi * m_fBlockSize - m_vGridOrigin[1] + (float)(cellj & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAy = (y - fBottom) * m_fVoxelScale;
	int zi = cellk >> BLOCK_CELL_LOG2SIZE;
	float fBack = zi * m_fBlockSize - m_vGridOrigin[2] + (float)(cellk & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAz = (z - fBack) * m_fVoxelScale;

	return 
		m_lastCellCache.fV000 * (1-fAx) * (1-fAy) * (1-fAz) +
		m_lastCellCache.fV001 * (1-fAx) * (1-fAy) * (fAz) +
		m_lastCellCache.fV010 * (1-fAx) * (fAy)   * (1-fAz) +
		m_lastCellCache.fV011 * (1-fAx) * (fAy)   * (fAz) +
		m_lastCellCache.fV100 * (fAx)   * (1-fAy) * (1-fAz) +
		m_lastCellCache.fV101 * (fAx)   * (1-fAy) * (fAz) +
		m_lastCellCache.fV110 * (fAx)   * (fAy)   * (1-fAz) +
		m_lastCellCache.fV111 * (fAx)   * (fAy)   * (fAz);
}


#else

// this is the simple implementation...

float CAdaptiveUniformGrid3D::SampleTriLinear( float x, float y, float z, ScalarField * pField )
{
	int celli = (int)( (x + m_vGridOrigin[0]) * m_fVoxelScale );
	int cellj = (int)( (y + m_vGridOrigin[1]) * m_fVoxelScale );
	int cellk = (int)( (z + m_vGridOrigin[2]) * m_fVoxelScale );

	float fV000 = GetValueOrCache(celli, cellj, cellk        , pField);
	float fV001 = GetValueOrCache(celli, cellj, cellk+1      , pField);
	float fV010 = GetValueOrCache(celli, cellj+1, cellk      , pField);
	float fV011 = GetValueOrCache(celli, cellj+1, cellk+1    , pField);
	float fV100 = GetValueOrCache(celli+1, cellj, cellk      , pField);
	float fV101 = GetValueOrCache(celli+1, cellj, cellk+1    , pField);
	float fV110 = GetValueOrCache(celli+1, cellj+1, cellk    , pField);
	float fV111 = GetValueOrCache(celli+1, cellj+1, cellk+1  , pField);

	// can probably improve this...
	int xi = celli >> BLOCK_CELL_LOG2SIZE;
	float fLeft = xi * m_fBlockSize - m_vGridOrigin[0] + (float)(celli & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAx = (x - fLeft) * m_fVoxelScale;
	int yi = cellj >> BLOCK_CELL_LOG2SIZE;
	float fBottom = yi * m_fBlockSize - m_vGridOrigin[1] + (float)(cellj & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAy = (y - fBottom) * m_fVoxelScale;
	int zi = cellk >> BLOCK_CELL_LOG2SIZE;
	float fBack = zi * m_fBlockSize - m_vGridOrigin[2] + (float)(cellk & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAz = (z - fBack) * m_fVoxelScale;

	return 
		fV000 * (1-fAx) * (1-fAy) * (1-fAz) +
		fV001 * (1-fAx) * (1-fAy) * (fAz) +
		fV010 * (1-fAx) * (fAy)   * (1-fAz) +
		fV011 * (1-fAx) * (fAy)   * (fAz) +
		fV100 * (fAx)   * (1-fAy) * (1-fAz) +
		fV101 * (fAx)   * (1-fAy) * (fAz) +
		fV110 * (fAx)   * (fAy)   * (1-fAz) +
		fV111 * (fAx)   * (fAy)   * (fAz);
}


#endif


void CAdaptiveUniformGrid3D::SampleGradientTriLinear( float x, float y, float z, 
													  float & gradX, float & gradY, float & gradZ, 
													  float * pFieldValue, CBlobNode * pField)
{
	int celli = (int)( (x + m_vGridOrigin[0]) * m_fVoxelScale );
	int cellj = (int)( (y + m_vGridOrigin[1]) * m_fVoxelScale );
	int cellk = (int)( (z + m_vGridOrigin[2]) * m_fVoxelScale );

	float fV000 = GetValueOrCache(celli, cellj, cellk        , pField);
	float fV001 = GetValueOrCache(celli, cellj, cellk+1      , pField);
	float fV010 = GetValueOrCache(celli, cellj+1, cellk      , pField);
	float fV011 = GetValueOrCache(celli, cellj+1, cellk+1    , pField);
	float fV100 = GetValueOrCache(celli+1, cellj, cellk      , pField);
	float fV101 = GetValueOrCache(celli+1, cellj, cellk+1    , pField);
	float fV110 = GetValueOrCache(celli+1, cellj+1, cellk    , pField);
	float fV111 = GetValueOrCache(celli+1, cellj+1, cellk+1  , pField);

	// can probably improve this...
	int xi = celli >> BLOCK_CELL_LOG2SIZE;
	float fLeft = xi * m_fBlockSize - m_vGridOrigin[0] + (float)(celli & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAx = (x - fLeft) * m_fVoxelScale;
	int yi = cellj >> BLOCK_CELL_LOG2SIZE;
	float fBottom = yi * m_fBlockSize - m_vGridOrigin[1] + (float)(cellj & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAy = (y - fBottom) * m_fVoxelScale;
	int zi = cellk >> BLOCK_CELL_LOG2SIZE;
	float fBack = zi * m_fBlockSize - m_vGridOrigin[2] + (float)(cellk & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAz = (z - fBack) * m_fVoxelScale;	

	gradX = 
		-fV000 * (1-fAy) * (1-fAz) +
		-fV001 * (1-fAy) * (fAz) +
		-fV010 * (fAy)   * (1-fAz) +
		-fV011 * (fAy)   * (fAz) +
		fV100  * (1-fAy) * (1-fAz) +
		fV101  * (1-fAy) * (fAz) +
		fV110  * (fAy)   * (1-fAz) +
		fV111  * (fAy)   * (fAz);

	gradY = 
		-fV000 * (1-fAx) * (1-fAz) +
		-fV001 * (1-fAx) * (fAz) +
		fV010 * (1-fAx) * (1-fAz) +
		fV011 * (1-fAx) * (fAz) +
		-fV100 * (fAx)   * (1-fAz) +
		-fV101 * (fAx)   * (fAz) +
		fV110 * (fAx)   * (1-fAz) +
		fV111 * (fAx)   * (fAz);

	gradZ = 
		-fV000 * (1-fAx) * (1-fAy) +
		fV001 * (1-fAx) * (1-fAy) +
		-fV010 * (1-fAx) * (fAy) +
		fV011 * (1-fAx) * (fAy) +
		-fV100 * (fAx)   * (1-fAy) +
		fV101 * (fAx)   * (1-fAy) +
		-fV110 * (fAx)   * (fAy) +
		fV111 * (fAx)   * (fAy);

	if (pFieldValue)
		*pFieldValue = 
		fV000 * (1-fAx) * (1-fAy) * (1-fAz) +
		fV001 * (1-fAx) * (1-fAy) * (fAz) +
		fV010 * (1-fAx) * (fAy)   * (1-fAz) +
		fV011 * (1-fAx) * (fAy)   * (fAz) +
		fV100 * (fAx)   * (1-fAy) * (1-fAz) +
		fV101 * (fAx)   * (1-fAy) * (fAz) +
		fV110 * (fAx)   * (fAy)   * (1-fAz) +
		fV111 * (fAx)   * (fAy)   * (fAz);
}





float CAdaptiveUniformGrid3D::SampleTriQuadratic( float x, float y, float z, CBlobNode * pField )
{
	int celli = (int)( (x + m_vGridOrigin[0]) * m_fVoxelScale );
	int cellj = (int)( (y + m_vGridOrigin[1]) * m_fVoxelScale );
	int cellk = (int)( (z + m_vGridOrigin[2]) * m_fVoxelScale );

	// calculate params
	int xi = celli >> BLOCK_CELL_LOG2SIZE;
	float fLeft = xi * m_fBlockSize - m_vGridOrigin[0] + (float)(celli & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAx = (x - fLeft) * m_fVoxelScale;
	int yi = cellj >> BLOCK_CELL_LOG2SIZE;
	float fBottom = yi * m_fBlockSize - m_vGridOrigin[1] + (float)(cellj & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAy = (y - fBottom) * m_fVoxelScale;
	int zi = cellk >> BLOCK_CELL_LOG2SIZE;
	float fBack = zi * m_fBlockSize - m_vGridOrigin[2] + (float)(cellk & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAz = (z - fBack) * m_fVoxelScale;

	// shift left by half cell width
	if ( fAx > 0.5f) {
		fAx -= 0.5f;
		celli++;
	} else 
		fAx += 0.5f;

	if ( fAy > 0.5f) {
		fAy -= 0.5f;
		cellj++;
	} else 
		fAy += 0.5f;

	if ( fAz > 0.5f) {
		fAz -= 0.5f;
		cellk++;
	} else 
		fAz += 0.5f;


	// compute values for x directions...

	float fXCoeff1 = 0.5f - fAx + 0.5f*fAx*fAx;
	float fXCoeff2 = 0.5f + fAx - fAx*fAx;
	float fXCoeff3 = 0.5f * fAx * fAx;

	float Fx[3][3];
	for (int zOff = -1; zOff <= 1; ++zOff) {
		for (int yOff = -1; yOff <= 1; ++yOff) {
			float fXLeft = GetValueOrCache( celli-1, cellj+yOff, cellk+zOff, pField );
			float fXMiddle = GetValueOrCache( celli, cellj+yOff, cellk+zOff, pField );
			float fXRight = GetValueOrCache( celli+1, cellj+yOff, cellk+zOff, pField );

			Fx[zOff+1][yOff+1] = fXCoeff1 * fXLeft  +  fXCoeff2 * fXMiddle  + fXCoeff3 * fXRight;
		}
	}


	// now y directions...

	float fYCoeff1 = 0.5f - fAy + 0.5f*fAy*fAy;
	float fYCoeff2 = 0.5f + fAy - fAy*fAy;
	float fYCoeff3 = 0.5f * fAy * fAy;

	float Fy[3];
	for (int zOff = -1; zOff <= 1; ++zOff) {
		Fy[zOff+1] = fYCoeff1 * Fx[zOff+1][0]  +  fYCoeff2 * Fx[zOff+1][1]  +  fYCoeff3 * Fx[zOff+1][2];
	}


	// now z direction
	float fZCoeff1 = 0.5f - fAz + 0.5f*fAz*fAz;
	float fZCoeff2 = 0.5f + fAz - fAz*fAz;
	float fZCoeff3 = 0.5f * fAz * fAz;

	float Fz = fZCoeff1 * Fy[0]  +  fZCoeff2 * Fy[1]  +  fZCoeff3 * Fy[2];

	return Fz;
}



void CAdaptiveUniformGrid3D::SampleGradientTriQuadratic( float x, float y, float z, 
													    float & gradX, float & gradY, float & gradZ, 
														float * pFieldValue, CBlobNode * pField)
{
	int celli = (int)( (x + m_vGridOrigin[0]) * m_fVoxelScale );
	int cellj = (int)( (y + m_vGridOrigin[1]) * m_fVoxelScale );
	int cellk = (int)( (z + m_vGridOrigin[2]) * m_fVoxelScale );

	// calculate params
	int xi = celli >> BLOCK_CELL_LOG2SIZE;
	float fLeft = xi * m_fBlockSize - m_vGridOrigin[0] + (float)(celli & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAx = (x - fLeft) * m_fVoxelScale;
	int yi = cellj >> BLOCK_CELL_LOG2SIZE;
	float fBottom = yi * m_fBlockSize - m_vGridOrigin[1] + (float)(cellj & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAy = (y - fBottom) * m_fVoxelScale;
	int zi = cellk >> BLOCK_CELL_LOG2SIZE;
	float fBack = zi * m_fBlockSize - m_vGridOrigin[2] + (float)(cellk & BLOCK_CELL_BITMASK) * m_fVoxelSize;
	float fAz = (z - fBack) * m_fVoxelScale;

	// shift left by half cell width
	if ( fAx > 0.5f) {
		fAx -= 0.5f;
		celli++;
	} else 
		fAx += 0.5f;

	if ( fAy > 0.5f) {
		fAy -= 0.5f;
		cellj++;
	} else 
		fAy += 0.5f;

	if ( fAz > 0.5f) {
		fAz -= 0.5f;
		cellk++;
	} else 
		fAz += 0.5f;


	// compute values for x directions...

	float fXCoeff1 = 0.5f - fAx + 0.5f*fAx*fAx;
	float fXCoeff2 = 0.5f + fAx - fAx*fAx;
	float fXCoeff3 = 0.5f * fAx * fAx;

	float fDXCoeff1 = -1.0f + fAx;
	float fDXCoeff2 = 1.0f - 2.0f*fAx;
	float fDXCoeff3 = fAx;

	float Fx[3][3];
	float Dx[3][3];
	for (int zOff = -1; zOff <= 1; ++zOff) {
		for (int yOff = -1; yOff <= 1; ++yOff) {
			float fXLeft = GetValueOrCache( celli-1, cellj+yOff, cellk+zOff, pField );
			float fXMiddle = GetValueOrCache( celli, cellj+yOff, cellk+zOff, pField );
			float fXRight = GetValueOrCache( celli+1, cellj+yOff, cellk+zOff, pField );

			Fx[zOff+1][yOff+1] = fXCoeff1 * fXLeft  +  fXCoeff2 * fXMiddle  + fXCoeff3 * fXRight;
			Dx[zOff+1][yOff+1] = fDXCoeff1 * fXLeft  +  fDXCoeff2 * fXMiddle  + fDXCoeff3 * fXRight;
		}
	}


	// now y directions...

	float fYCoeff1 = 0.5f - fAy + 0.5f*fAy*fAy;
	float fYCoeff2 = 0.5f + fAy - fAy*fAy;
	float fYCoeff3 = 0.5f * fAy * fAy;

	float fDYCoeff1 = -1.0f + fAy;
	float fDYCoeff2 = 1.0f - 2.0f*fAy;
	float fDYCoeff3 = fAy;	

	float Fy[3];
	float Dx_y[3];
	float Dy[3];
	for (int zOff = -1; zOff <= 1; ++zOff) {
		Fy[zOff+1] = fYCoeff1 * Fx[zOff+1][0]  +  fYCoeff2 * Fx[zOff+1][1]  +  fYCoeff3 * Fx[zOff+1][2];
		Dx_y[zOff+1] = fYCoeff1 * Dx[zOff+1][0]  +  fYCoeff2 * Dx[zOff+1][1]  +  fYCoeff3 * Dx[zOff+1][2];
		Dy[zOff+1] = fDYCoeff1 * Fx[zOff+1][0]  +  fDYCoeff2 * Fx[zOff+1][1]  +  fDYCoeff3 * Fx[zOff+1][2];
	}


	// now z direction
	float fZCoeff1 = 0.5f - fAz + 0.5f*fAz*fAz;
	float fZCoeff2 = 0.5f + fAz - fAz*fAz;
	float fZCoeff3 = 0.5f * fAz * fAz;

	float fDZCoeff1 = -1.0f + fAz;
	float fDZCoeff2 = 1.0f - 2.0f*fAz;
	float fDZCoeff3 = fAz;	

	float Fz = fZCoeff1 * Fy[0]  +  fZCoeff2 * Fy[1]  +  fZCoeff3 * Fy[2];

	float FDx = fZCoeff1 * Dx_y[0]  +  fZCoeff2 * Dx_y[1]  +  fZCoeff3 * Dx_y[2];
	float FDy = fZCoeff1 * Dy[0]  +  fZCoeff2 * Dy[1]  +  fZCoeff3 * Dy[2];
	float FDz = fDZCoeff1 * Fy[0]  +  fDZCoeff2 * Fy[1]  +  fDZCoeff3 * Fy[2];

	gradX = m_fVoxelScale * FDx;
	gradY = m_fVoxelScale * FDy;
	gradZ = m_fVoxelScale * FDz;

	if (pFieldValue)
		*pFieldValue = Fz;
}


void CAdaptiveUniformGrid3D::InvalidateLastCellCache( )
{
	memset(&m_lastCellCache, 0, sizeof(LastCellCache) );
	m_lastCellCache.celli = INVALID_COORD;
	m_lastCellCache.cellj = INVALID_COORD;
	m_lastCellCache.cellk = INVALID_COORD;
}


void CAdaptiveUniformGrid3D::Invalidate( bool bDiscardMemory )
{
	std::map<unsigned int, Block *>::iterator cur(m_blocks.begin()), end(m_blocks.end());
	while (cur != end) {

		if (!bDiscardMemory) {
			(*cur).second->Invalidate( BLOCK_CELL_SIZE, *GetInvalidBlock() );
		} else {
			delete (*cur).second;
			(*cur).second = NULL;
		}
		++cur;
	}

	if (bDiscardMemory) 
	{
		m_blocks.clear();
                m_nLastBlockID =  static_cast<U32>(1<<32);	// invalid!
		m_pLastBlock = NULL;
	}

	InvalidateLastCellCache();
}


void CAdaptiveUniformGrid3D::Invalidate(Vol::CVolumeBox & Bounds)
{
//	PARS_PROFILER_start(2);

	std::map<unsigned int, Block *>::iterator cur(m_blocks.begin()), end(m_blocks.end()), tmp;
	while (cur != end) {

		int nBlockID = (*cur).second->nBlockID;
		float fXMin = _BLOCK_ID_X(nBlockID) * m_fBlockSize - m_vGridOrigin[0];
		float fYMin = _BLOCK_ID_Y(nBlockID) * m_fBlockSize - m_vGridOrigin[1];
		float fZMin = _BLOCK_ID_Z(nBlockID) * m_fBlockSize - m_vGridOrigin[2];

		Vol::CVolumeBox box(fXMin, fYMin, fZMin, fXMin + m_fBlockSize, fYMin + m_fBlockSize, fZMin + m_fBlockSize);
		if (Bounds.intersect(&box)) {
			(*cur).second->Invalidate( BLOCK_CELL_SIZE, *GetInvalidBlock() );
			++cur;
			//delete (*cur).second;
			//tmp = cur;
			//tmp++;
			//m_blocks.erase(cur);
			//cur = tmp;
		} else
			++cur;
	}

	InvalidateLastCellCache();

//	PARS_PROFILER_end(2);

//	_RMSInfo("Invalidate took %f seconds\r\n", PARS_PROFILER_time(2) );
}


void CAdaptiveUniformGrid3D::CalculateCellAllocationStats( const CVolumeBox & bounds,
														  unsigned int & nBlocksTotal, unsigned int & nBlocksAllocated,
														  unsigned int & nCellsTotal, unsigned int & nCellsAllocated )
{
        vec3f lower = bounds.lower();
        vec3f upper = bounds.upper();
	float fWidth  = upper.x - lower.x;
	float fHeight = upper.y - lower.y;
	float fDepth  = upper.z - lower.z;

	int nBlocksX = (int)(fWidth / m_fBlockSize) + 1;
	int nBlocksY = (int)(fHeight / m_fBlockSize) + 1;
	int nBlocksZ = (int)(fDepth / m_fBlockSize) + 1;

	nBlocksTotal = nBlocksX * nBlocksY * nBlocksZ;
	nBlocksAllocated = (unsigned int)m_blocks.size();

	int nCellsX = (int)(fWidth / m_fVoxelSize) + 1;
	int nCellsY = (int)(fHeight / m_fVoxelSize) + 1;
	int nCellsZ = (int)(fDepth / m_fVoxelSize) + 1;

	parsDebugInfo("BlockSize: [%d %d %d]   Cell Size:  [%d %d %d]\n",
		nBlocksX, nBlocksY, nBlocksZ, nCellsX, nCellsY, nCellsZ );

	nCellsTotal = nCellsX * nCellsY * nCellsZ;

	nCellsAllocated = nBlocksAllocated * BLOCK_CELL_SIZE * BLOCK_CELL_SIZE * BLOCK_CELL_SIZE;
}

void CAdaptiveUniformGrid3D::CalculateCellEvaluationStats(  const CVolumeBox & bounds,
								  unsigned int & nCellsTotalGrid, unsigned int & nCellsTotalAllocated, unsigned int & nCellsEvaluated )
{
        vec3f lower = bounds.lower();
        vec3f upper = bounds.upper();
	float fWidth  = upper.x - lower.x;
	float fHeight = upper.y - lower.y;
	float fDepth  = upper.z - lower.z;

	unsigned int nBlocksAllocated = (unsigned int)m_blocks.size();

	int nCellsX = (int)(fWidth / m_fVoxelSize) + 1;
	int nCellsY = (int)(fHeight / m_fVoxelSize) + 1;
	int nCellsZ = (int)(fDepth / m_fVoxelSize) + 1;

	nCellsTotalGrid = nCellsX * nCellsY * nCellsZ;
	nCellsTotalAllocated = nBlocksAllocated * BLOCK_CELL_SIZE * BLOCK_CELL_SIZE * BLOCK_CELL_SIZE;

	nCellsEvaluated = 0;
	std::map<unsigned int, Block *>::iterator cur(m_blocks.begin()), end(m_blocks.end());
	while (cur != end) {

		Block * pBlock = (*cur++).second;

		for (int i = 0; i < BLOCK_CELL_SIZE * BLOCK_CELL_SIZE * BLOCK_CELL_SIZE; ++i)
			if (pBlock->pCells[i] != INVALID_VALUE)
				++nCellsEvaluated;
	}

}


