#include "CNullPrimitive.h"
#include "BlobTreeBuilder.h"

using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateNull()
    {
        return new CNullPrimitive();
    }

    CBlobNode* CloneNull(const CBlobNode* b)
    {
        CNullPrimitive* clonned = new CNullPrimitive();
        clonned->copyGenericInfo(b);
        return clonned;
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("NULL", CreateNull) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntPrimNull, CreateNull) &&
                            TheBlobNodeCloneFactory::Instance().Register(typeid(CNullPrimitive), CloneNull);
}

