#include "CSkeletonRing.h"
#include "CSkeletonPrimitive.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateRing()
    {
        return new CSkeletonPrimitive(new CSkeletonRing(), fftWyvill);
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("RING", CreateRing);
}


