#include "CSkeletonDisc.h"
#include "CSkeletonPrimitive.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateDisc()
    {
        return new CSkeletonPrimitive(new CSkeletonDisc(), fftWyvill);
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("DISC", CreateDisc) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntPrimDisc, CreateDisc);
}



