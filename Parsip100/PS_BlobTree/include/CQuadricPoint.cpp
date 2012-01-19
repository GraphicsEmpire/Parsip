#include "CQuadricPoint.h"
#include "BlobTreeBuilder.h"

using namespace PS::BLOBTREE;

//Create
namespace {

    CBlobNode* CreateQuadricPoint()
    {
        return new CQuadricPoint();
    }

    CBlobNode* CloneQuadricPoint(const CBlobNode* b)
    {
        const CQuadricPoint* rhs = reinterpret_cast<const CQuadricPoint*>(b);
        CQuadricPoint* clonned = new CQuadricPoint(rhs->getPosition(), rhs->getFieldRadius(), rhs->getFieldScale());
        return clonned;
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("QUADRICPOINT", CreateQuadricPoint) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntPrimQuadricPoint, CreateQuadricPoint) &&
                            TheBlobNodeCloneFactory::Instance().Register(typeid(CQuadricPoint), CloneQuadricPoint);
}

