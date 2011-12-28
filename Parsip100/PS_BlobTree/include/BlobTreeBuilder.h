#ifndef BLOBTREEBUILDER_H
#define BLOBTREEBUILDER_H

#include "CBlobTree.h"
#include "_constSettings.h"
#include <string.h>

#include <loki/Function.h>
#include <loki/Factory.h>


using namespace Loki;
using namespace std;

namespace PS{
namespace BLOBTREE{

//FactoryByName to access BlobNode with their names
typedef SingletonHolder
<
    Factory<CBlobNode, std::string>,
    CreateUsingNew,    
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
TheBlobNodeFactoryName;

//FactoryByIndex to access BlobNodes with their indices
typedef SingletonHolder
<
    Factory<CBlobNode, BlobNodeType>,
    CreateUsingNew,    
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
TheBlobNodeFactoryIndex;

//Clone Factory to create a clone of a BlobNode
typedef SingletonHolder
<
    CloneFactory<CBlobNode>,
    CreateUsingNew,    
    Loki::LongevityLifetime::DieAsSmallObjectChild
>
TheBlobNodeCloneFactory;


//Actions and ActionManager are the only channel to modify a Blobtree
//1.Actions are all reversible for propert Undo/Redo
//2.Actions are serializeable for replay and record.
//3.Actions are persistent and (Can be queued for future execution)
enum BlobNodeActions {bnaAdd, bnaDelete, bnaTranslate, bnaRotate, bnaScale, bnaSetParam};
typedef Functor<bool, TL::MakeTypelist<BlobNodeType, vec3f> > FuncAdd;

class ReversibleAction
{


};

}
}
#endif
