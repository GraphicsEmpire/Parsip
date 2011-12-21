#include "CDifference.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateDifference()
    {
        return new CDifference();
    }

    CBlobNode* CloneDif(const CBlobNode* b)
    {
        CDifference* clonned = new CDifference();
        clonned->copyGenericInfo(b);
        return clonned;
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("DIFFERENCE", CreateDifference) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntOpDif, CreateDifference) &&
                            TheBlobNodeCloneFactory::Instance().Register(typeid(CDifference), CloneDif);
}
