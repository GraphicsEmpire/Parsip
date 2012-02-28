#ifndef CInstance_H
#define CInstance_H

#include <math.h>
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

//Instancing is essential for building massive models
class  CInstance : public CBlobNode
{

public:
    CInstance() { m_bDeleteChildrenUponCleanup = false;}
    CInstance(CBlobNode * child)
    {
        m_bDeleteChildrenUponCleanup = false;
        addChild(child);
    }

    float fieldValue(vec3f p)
    {
        vec3f pt = this->getTransform().applyBackwardTransform(p);
        return m_children[0]->fieldValue(pt);
    }

    std::string getName()
    {
        return "INSTANCE";
    }

    COctree computeOctree()
    {
        if(m_children.size() == 0) return m_octree;
        m_octree = m_children[0]->getOctree();        
        m_octree.transform(this->getTransform().getForwardMatrix());
        return m_octree;
    }

    bool isOperator() { return true;}

    BlobNodeType getNodeType() {return bntOpInstance;}
};

}
}
#endif // CInstance_H
