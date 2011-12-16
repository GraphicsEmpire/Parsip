#include "CBlend.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateBlend()
    {
        return new CBlend();
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("BLEND", CreateBlend);
}
