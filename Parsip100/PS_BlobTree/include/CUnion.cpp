#include "CUnion.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateUnion()
    {
        return new CUnion();
    }

    CBlobNode* CloneUnion(const CBlobNode* b)
    {
        CUnion* clonned = new CUnion();
        clonned->copyGenericInfo(b);
        return clonned;
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("UNION", CreateUnion) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntOpUnion, CreateUnion) &&
                            TheBlobNodeCloneFactory::Instance().Register(typeid(CUnion), CloneUnion);

}

