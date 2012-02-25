#include "CFastQuadricPointSet.h"
#include "BlobTreeBuilder.h"

using namespace PS::BLOBTREE;

//Create
namespace {

    CBlobNode* CreateFastQuadricPointSet()
    {
        return new CFastQuadricPointSet();
    }

    CBlobNode* CloneFastQuadricPointSet(const CBlobNode* b)
    {
        const CFastQuadricPointSet* rhs = reinterpret_cast<const CFastQuadricPointSet*>(b);
        CFastQuadricPointSet* clonned = new CFastQuadricPointSet(rhs->isPerPointColors());
        return clonned;
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("FASTQUADRICPOINTSET", CreateFastQuadricPointSet) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntOpFastQuadricPointSet, CreateFastQuadricPointSet) &&
                            TheBlobNodeCloneFactory::Instance().Register(typeid(CFastQuadricPointSet), CloneFastQuadricPointSet);
}



namespace PS{
namespace BLOBTREE{

void CFastQuadricPointSet::prepare()
{
    cleanup();
    for(U32 i=0; i<this->countChildren(); i++)
    {
        CQuadricPoint* lpPoint = reinterpret_cast<CQuadricPoint*>(this->getChild(i));
        IPoint pt(lpPoint->getPosition(), lpPoint->getFieldScale(), lpPoint->getFieldRadius());
        m_vPoints.push_back(pt);
        if (m_bUsePerPointColors)
            m_vColors.push_back(lpPoint->getMaterial().diffused.xyz());
    }
    m_bValidOctree = false;
}


float CFastQuadricPointSet::fieldValue(vec3f p)
{
    if (m_bValidOctree)
    {
        if(m_octree.isInside(p) == false)
            return 0.0f;
    }

    if((m_vPoints.size() == 0)&&(this->countChildren() > 0))
        this->prepare();

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

int CFastQuadricPointSet::fieldValueAndGradient(vec3f p, float delta, vec3f &outGradient, float &outField)
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

COctree CFastQuadricPointSet::computeOctree()
{
    //Call Prepare to update internal data
    prepare();

    if(m_vPoints.size() > 0)
    {
        COctree tmp;
        getPointFieldBox(0, m_octree);
        for (U32 i = 1; i < m_vPoints.size(); i++)
        {
            getPointFieldBox(i, tmp);
            m_octree.csgUnion(tmp);
        }
    }

    m_bValidOctree = true;
    return m_octree;
}


bool CFastQuadricPointSet::getPointFieldBox(U32 idxPoint, COctree& dest )
{
    if(idxPoint >= m_vPoints.size())
        return false;

    float fWidth = m_vPoints[idxPoint].fRadius;
    dest.lower = m_vPoints[idxPoint].ptPosition - fWidth;
    dest.upper = m_vPoints[idxPoint].ptPosition + fWidth;
    return true;
}

}
}
