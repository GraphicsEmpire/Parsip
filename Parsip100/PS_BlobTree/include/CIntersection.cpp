#include "CIntersection.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateIntersection()
    {
        return new CIntersection();
    }

    CBlobNode* CloneIntersection(const CBlobNode* b)
    {
        CIntersection* clonned = new CIntersection();
        clonned->copyGenericInfo(b);
        return clonned;
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("INTERSECTION", CreateIntersection) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntOpIntersect, CreateIntersection) &&
                            TheBlobNodeCloneFactory::Instance().Register(typeid(CIntersection), CloneIntersection);
}

