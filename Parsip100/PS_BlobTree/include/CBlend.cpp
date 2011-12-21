#include "CBlend.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateBlend()
    {
        return new CBlend();
    }

    CBlobNode* CloneBlend(const CBlobNode* b)
    {
        CBlend* clonned = new CBlend();
        clonned->copyGenericInfo(b);
        return clonned;
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("BLEND", CreateBlend) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntOpBlend, CreateBlend) &&
                            TheBlobNodeCloneFactory::Instance().Register(typeid(CBlend), CloneBlend);
}
