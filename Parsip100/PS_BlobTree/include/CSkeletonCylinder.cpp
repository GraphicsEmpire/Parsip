#include "CSkeletonCylinder.h""
#include "CSkeletonPrimitive.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateCylinder()
    {
        return new CSkeletonPrimitive(new CSkeletonCylinder(), fftWyvill);
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("CYLINDER", CreateCylinder) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntPrimCylinder, CreateCylinder);
}


