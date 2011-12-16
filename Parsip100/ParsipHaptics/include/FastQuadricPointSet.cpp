#include "FastQuadricPointSet.h"

//#include <WmlExtVectorUtil.h>

namespace PS{
	namespace BLOBTREE{

float FastQuadricPointSet::fieldValue(vec3f p)
{	
	if (m_bValidOctree)
	{
		if(m_octree.isInside(p) == false)
			return 0.0f;
	}

	float fSumValue = 0.0;
	unsigned int nSize = (unsigned int)m_vPoints.size();
	for(unsigned int i = 0; i < nSize; ++i)
	{
		IPoint & point = m_vPoints[i];
		float fDist2 = point.ptPosition.dist2(p);
		if (fDist2 < point.fRadiusSqr)
			fSumValue += fDist2*fDist2*point.fCoeff1 + fDist2*point.fCoeff2 + point.fCoeff3;
	}
	return fSumValue;
}

int FastQuadricPointSet::fieldValueAndGradient(vec3f p, float delta, vec3f &outGradient, float &outField)
{
	outGradient.zero();	
	outField = 0.0;
	
	float dx,dy,dz;
	unsigned int nSize = (unsigned int)m_vPoints.size();

	for (unsigned int i = 0; i < nSize; ++i) 
	{		
		IPoint & point = m_vPoints[i];

		dx = (p.x - point.ptPosition.x); 
		dy = (p.y - point.ptPosition.y); 
		dz = (p.z - point.ptPosition.z); 
		float fDist2 = dx*dx + dy*dy + dz*dz;
		float fValue = (1.0f - (fDist2 / (point.fRadius*point.fRadius)) );

		if (fValue > 0.0f) {

			float fGradCoeff = (- 4.0f * point.fFieldScale * fValue) / (point.fRadius*point.fRadius);
			outGradient.x += dx * fGradCoeff;
			outGradient.y += dy * fGradCoeff;
			outGradient.z += dz * fGradCoeff;

			outField += (point.fFieldScale * fValue * fValue);
		}
	}

	//No Field Evaluation Done!
	return 0;
}


void FastQuadricPointSet::getSeedPoints( std::vector<vec3f> & seedPoints )
{
	unsigned int nSize = (unsigned int)m_vPoints.size();
	for (unsigned int i = 0; i < nSize; ++i) {
		seedPoints.push_back( m_vPoints[i].ptPosition );
	}
}

COctree FastQuadricPointSet::computeOctree()
{
	COctree tmp;

	getPointFieldBox(0, m_octree);
	unsigned int nSize = (unsigned int)m_vPoints.size();	
	for (unsigned int i = 1; i < nSize; ++i) 
	{
		getPointFieldBox(i, tmp);
		m_octree.csgUnion(tmp);
	}

	m_bValidOctree = true;
	return m_octree;
}


void FastQuadricPointSet::translate( const vec3f & bTranslate )
{
	unsigned int nSize = (unsigned int)m_vPoints.size();
	for (unsigned int i = 0; i < nSize; ++i) 
	{
		m_vPoints[i].ptPosition += bTranslate;
	}

	// invalidate field bounds cache...
	m_bValidOctree = false;
}


/*
 * private functions
 */

void FastQuadricPointSet::getPointFieldBox( unsigned int nPoint, COctree& dest )
{
	float fWidth = m_vPoints[nPoint].fRadius;

	dest.lower = m_vPoints[nPoint].ptPosition - fWidth;
	dest.upper = m_vPoints[nPoint].ptPosition + fWidth;
}

}
}