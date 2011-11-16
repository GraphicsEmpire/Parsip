#ifndef CPCM_H
#define CPCM_H

#include <math.h>
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

//***********************************************************
//PCM: Precise Contact Modeling
class  CPcm : public CBlobTree
{
private:
    float m_wPropagateLeft;
    float m_wPropagateRight;

    //Attenuation value for propagation region
    float m_alphaLeft;
    float m_alphaRight;

public:
    CPcm() { resetParams(); }

    float getPropagateLeft() const {return m_wPropagateLeft;}
    void  setPropagateLeft(float val) { m_wPropagateLeft = val;}

    float getPropagateRight() const {return m_wPropagateRight;}
    void  setPropagateRight(float val) { m_wPropagateRight = val;}

    float getAlphaLeft() const {return m_alphaLeft;}
    void  setAlphaLeft(float val) { m_alphaLeft = val;}

    float getAlphaRight() const {return m_alphaRight;}
    void  setAlphaRight(float val) { m_alphaRight = val;}

    void resetParams()
    {
        m_wPropagateLeft = m_wPropagateRight = PCM_PROPAGATION_WIDTH;
        m_alphaLeft = m_alphaRight = PCM_ATTENUATION;
    }

    void setParams(float pl, float pr, float al, float ar)
    {
        m_wPropagateLeft = pl;
        m_wPropagateRight = pr;
        m_alphaLeft = al;
        m_alphaRight = ar;
    }

    //FieldValue computation
    float fieldValue(vec3f p)
    {
        if(m_children.size() == 2)
            return MATHMAX(m_children[0]->fieldValue(p), m_children[1]->fieldValue(p));
        else
            return 0.0;
    }

    float curvature(vec3f p)
    {
        return m_children[0]->curvature(p);
    }

    vec4f baseColor(vec3f p)
    {
        return m_children[0]->baseColor(p);
    }

    CMaterial baseMaterial(vec3f p)
    {
        return m_children[0]->baseMaterial(p);
    }


    void getName(char * chrName)
    {
        strncpy(chrName, "PCM", MAX_NAME_LEN);
    }

    COctree computeOctree()
    {
        m_octree = m_children[0]->getOctree();
        for(size_t i = 1; i < m_children.size(); i++)
            m_octree.csgUnion(m_children[i]->getOctree());
        return m_octree;
    }

    bool isOperator() { return true;}

    BlobNodeType getNodeType() { return bntOpPCM;}
};

}
}


#endif
