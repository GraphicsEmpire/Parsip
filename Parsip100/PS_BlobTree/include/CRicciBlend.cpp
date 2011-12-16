#include "CRicciBlend.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateRicci()
    {
        return new CRicciBlend();
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("RICCI BLEND", CreateRicci);
}

