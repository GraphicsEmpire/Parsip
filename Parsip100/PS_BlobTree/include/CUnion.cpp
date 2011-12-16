#include "CUnion.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateUnion()
    {
        return new CUnion();
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("UNION", CreateUnion());
}

