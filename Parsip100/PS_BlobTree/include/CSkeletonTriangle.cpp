#include "CSkeletonTriangle.h"
#include "CSkeletonPrimitive.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateTriangle()
    {
        return new CSkeletonPrimitive(new CSkeletonTriangle(), fftWyvill);
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("TRIANGLE", CreateTriangle);
}



