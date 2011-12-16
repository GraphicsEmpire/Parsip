#include "CSkeletonCube.h"
#include "CSkeletonPrimitive.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateCube()
    {
        return new CSkeletonPrimitive(new CSkeletonCube(), fftWyvill);
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("CUBE", CreateCube);
}

