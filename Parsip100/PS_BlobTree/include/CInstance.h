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
    CInstance() { m_lpOrigin = NULL;}
    CInstance(CBlobNode * lpOrigin)
    {     
        this->m_lpOrigin = lpOrigin;
    }

    ~CInstance()
    {
        m_lpOrigin = NULL;
    }

    CBlobNode* getOriginalNode() const {return m_lpOrigin;}
    void setOriginalNode(CBlobNode* lpOrigin) { this->m_lpOrigin = lpOrigin;}

    float fieldValue(vec3f p)
    {
        if(m_lpOrigin)
        {
            vec3f pt = this->getTransform().applyBackwardTransform(p);
            return m_lpOrigin->fieldValue(pt);
        }
    }

    std::string getName()
    {
        return "INSTANCE";
    }

    COctree computeOctree()
    {
        if(m_lpOrigin == NULL) return m_octree;
        m_octree = m_lpOrigin->getOctree();

        //Return to local coordinate system
        //m_octree.transform(m_lpOrigin->getTransform().getBackwardMatrix());

        //Will be transformed to instanced space
        return m_octree;
    }

    bool isOperator() { return false;}

    BlobNodeType getNodeType() {return bntPrimInstance;}

    bool saveScript(CSketchConfig* lpSketchScript)
    {
        //Write parameters for RicciBlend
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", this->getID());
        lpSketchScript->writeInt(strNodeName, "OriginalNodeIndex", m_lpOrigin->getID());
        return this->saveGenericInfoScript(lpSketchScript);
    }

    bool loadScript(CSketchConfig* lpSketchScript, int id)
    {
        //Write parameters for RicciBlend
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        int idxOrigin = lpSketchScript->readInt(strNodeName, "OriginalNodeIndex");
        return this->loadGenericInfoScript(lpSketchScript, id);
    }


private:
    //Instance is not an operator with children
    CBlobNode* m_lpOrigin;

};

}
}
#endif // CInstance_H
