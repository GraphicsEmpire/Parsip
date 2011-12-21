#ifndef BLOBTREEBUILDER_H
#define BLOBTREEBUILDER_H

#include "CBlobTree.h"
#include "_constSettings.h"
#include <string.h>

#include <loki/Factory.h>

using namespace Loki;
using namespace std;

namespace PS{
namespace BLOBTREE{

typedef SingletonHolder
<
    Factory<CBlobNode, std::string>,
    CreateUsingNew,    
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
TheBlobNodeFactoryName;


typedef SingletonHolder
<
    Factory<CBlobNode, BlobNodeType>,
    CreateUsingNew,    
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
TheBlobNodeFactoryIndex;


typedef SingletonHolder
<
    CloneFactory<CBlobNode>,
    CreateUsingNew,    
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
TheBlobNodeCloneFactory;


}
}
#endif
