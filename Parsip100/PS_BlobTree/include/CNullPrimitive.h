#ifndef CNULLPRIMITIVE_H
#define CNULLPRIMITIVE_H

#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

class  CNullPrimitive : public CBlobNode
{

public:
    CNullPrimitive() {;}

    float fieldValue(vec3f p)
    {
        return 0.0f;
    }

    std::string getName()
    {
        return "NULL";
    }

    COctree computeOctree()
    {
        vec3f zero(0.0f, 0.0f, 0.0f);
        m_octree.set(zero, zero);
        return m_octree;
    }

    bool isOperator() { return false;}

    BlobNodeType getNodeType() {return bntPrimNull;}
};
}
}

#endif // CNULLPRIMITIVE_H
