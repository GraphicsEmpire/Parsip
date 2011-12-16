#include "CSkeletonPoint.h"
#include "CSkeletonPrimitive.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreatePoint()
    {
        return new CSkeletonPrimitive(new CSkeletonPoint(), fftWyvill);
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("POINT", CreatePoint);
}



