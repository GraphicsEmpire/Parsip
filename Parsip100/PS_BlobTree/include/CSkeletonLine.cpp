#include "CSkeletonLine.h"
#include "CSkeletonPrimitive.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateLine()
    {
        return new CSkeletonPrimitive(new CSkeletonLine(), fftWyvill);
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("LINE", CreateLine) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntPrimLine, CreateLine);
}



