#include "CDifference.h"
#include "BlobTreeBuilder.h"
using namespace PS::BLOBTREE;

namespace {

    CBlobNode* CreateDifference()
    {
        return new CDifference();
    }

    const bool registered = TheBlobNodeFactoryName::Instance().Register("DIFFERENCE", CreateDifference());
}
