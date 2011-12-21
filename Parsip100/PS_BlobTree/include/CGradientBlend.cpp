#include "CGradientBlend.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateGradientBlend()
    {
        return new CGradientBlend();
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("GRADIENT BLEND", CreateGradientBlend) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntOpGradientBlend, CreateGradientBlend);
}


