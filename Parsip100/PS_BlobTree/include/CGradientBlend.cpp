#include "CGradientBlend.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateGradientBlend()
    {
        return new CGradientBlend();
    }

    CBlobNode* CloneGradientBlend(const CBlobNode* b)
    {
        CGradientBlend* clonned = new CGradientBlend();
        clonned->copyGenericInfo(b);
        return clonned;
    }


    const bool registered = TheBlobNodeFactoryName::Instance().Register("GRADIENT BLEND", CreateGradientBlend) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntOpGradientBlend, CreateGradientBlend) &&
                            TheBlobNodeCloneFactory::Instance().Register(typeid(CGradientBlend), CloneGradientBlend);
}


