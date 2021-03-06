#ifndef CINTERSECTION_H
#define CINTERSECTION_H

#include <math.h>
#include <cfloat>
#include <limits.h>
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

class  CIntersection : public CBlobNode
{
public:
    CIntersection() {;}
    CIntersection(CBlobNode * child)
    {
        addChild(child);
    }

    CIntersection(CBlobNode * child1, CBlobNode * child2)
    {
        addChild(child1);
        addChild(child2);
    }

    CIntersection(CBlobNode * child1, CBlobNode * child2, CBlobNode * child3)
    {
        addChild(child1);
        addChild(child2);
        addChild(child3);
    }

    CIntersection(BLOBNODECHILDREN children)
    {
        addChild(children);
    }

    float fieldValue(vec3f p)
    {
        float result = FLT_MAX;
        for(size_t i=0; i < m_children.size(); i++)
        {
            result = min(result, m_children[i]->fieldValue(p));
        }
        return result;
    }

    float curvature(vec3f p)
    {
        CBlobNode * minNode = getChildMin(p);
        if(minNode)
            return minNode->curvature(p);
        else
            return 0.0f;
    }

    CBlobNode * getChildMin(vec3f p)
    {
        if(countChildren() == 0)
            return NULL;

        float minField = m_children[0]->fieldValue(p);
        size_t iMin = 0;
        float curField;
        for(size_t i = 1; i < m_children.size(); i++)
        {
            curField = m_children[i]->fieldValue(p);
            if(curField < minField)
            {
                minField = curField;
                iMin = i;
            }
        }
        return m_children[iMin];
    }

    string getName()
    {
        return "INTERSECTION";
    }

    COctree computeOctree()
    {
        m_octree = m_children[0]->getOctree();
        for(size_t i = 1; i < m_children.size(); i++)
            m_octree.csgIntersection(m_children[i]->getOctree());
        return m_octree;
    }

    bool isOperator() { return true;}
    BlobNodeType getNodeType() {return bntOpIntersect;}
};

}
}

#endif

