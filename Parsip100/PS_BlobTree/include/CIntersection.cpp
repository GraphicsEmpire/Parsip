#include "CIntersection.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateIntersection()
    {
        return new CIntersection();
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("INTERSECTION", CreateIntersection);
}

