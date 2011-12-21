#include "CRicciBlend.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateRicci()
    {
        return new CRicciBlend();
    }

    CBlobNode* CloneRicci(const CBlobNode* b)
    {
        CRicciBlend* clonned = new CRicciBlend();
        clonned->copyGenericInfo(b);
        clonned->setN(reinterpret_cast<const CRicciBlend*>(b)->getN());
        return clonned;
    }


    const bool registered = TheBlobNodeFactoryName::Instance().Register("RICCI BLEND", CreateRicci) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntOpRicciBlend, CreateRicci) &&
                            TheBlobNodeCloneFactory::Instance().Register(typeid(CRicciBlend), CloneRicci);
}

