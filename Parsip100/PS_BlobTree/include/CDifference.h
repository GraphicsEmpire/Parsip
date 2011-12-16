#ifndef CDIFFERENCE_H
#define CDIFFERENCE_H

#include "math.h"
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

class  CDifference : public CBlobNode
{
public:
    CDifference() {;}
    CDifference(CBlobNode * child)
    {
        addChild(child);
    }

    CDifference(CBlobNode * child1, CBlobNode * child2)
    {
        addChild(child1);
        addChild(child2);
    }

    CDifference(CBlobNode * child1, CBlobNode * child2, CBlobNode * child3)
    {
        addChild(child1);
        addChild(child2);
        addChild(child3);
    }

    CDifference(BLOBNODECHILDREN children)
    {
        addChild(children);
    }

    float fieldValue(vec3f p)
    {
        float result = m_children[0]->fieldValue(p);
        for(size_t i=1; i < m_children.size(); i++)
        {
            result = min(result, MAX_FIELD_VALUE - m_children[i]->fieldValue(p));
        }
        return result;
    }

    float curvature(vec3f p)
    {
        CBlobNode * minNode = getChildMinDif(p);
        if(minNode)
            return minNode->curvature(p);
        else
            return 0.0f;
    }

    CBlobNode * getChildMinDif(vec3f p)
    {
        if(countChildren() == 0)
            return NULL;

        float minField = m_children[0]->fieldValue(p);
        size_t iMin = 0;
        float curField;
        for(size_t i = 1; i < m_children.size(); i++)
        {
            curField = MAX_FIELD_VALUE - m_children[i]->fieldValue(p);
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
        return "DIFFERENCE";
    }

    COctree computeOctree()
    {
        m_octree = m_children[0]->getOctree();
        return m_octree;
    }


    bool isOperator() { return true;}
    BlobNodeType getNodeType() {return bntOpDif;}
};

}
}
#endif
