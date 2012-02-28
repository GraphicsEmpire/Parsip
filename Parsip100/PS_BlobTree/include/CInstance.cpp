#include "CInstance.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateInstance()
    {
        return new CInstance();
    }

    CBlobNode* CloneInstance(const CBlobNode* b)
    {
        CInstance* clonned = new CInstance();
        clonned->copyGenericInfo(b);
        return clonned;
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("INSTANCE", CreateInstance) &&
                            TheBlobNodeFactoryIndex::Instance().Register(bntOpInstance, CreateInstance) &&
                            TheBlobNodeCloneFactory::Instance().Register(typeid(CInstance), CloneInstance);
}
