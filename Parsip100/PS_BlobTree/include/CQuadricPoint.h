#ifndef CQUARICPOINT_H
#define CQUARICPOINT_H

#include "math.h"
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

class  CQuadricPoint : public CBlobNode
{
protected:
    vec3f   m_position;
    float   m_fRadius;
    float   m_fScale;

public:
    CQuadricPoint()
    {
        m_position = vec3f(0.0f, 0.0f, 0.0f);
        m_fScale = 1.0f;
        m_fRadius = 1.0f;
    }

    CQuadricPoint(vec3f pos, float fieldRadius, float fieldScale)
    {
        m_position = pos;
        m_fRadius = fieldRadius;
        m_fScale = fieldScale;
    }      

    //Accessor Functions
    vec3f getPosition() const {return m_position;}
    void setPosition(const vec3f& p) {m_position = p;}

    float getFieldRadius() const {return m_fRadius;}
    void setFieldRadius(float radius) {m_fRadius = radius;}

    float getFieldScale() const {return m_fScale;}
    void setFieldScale(float scale) {m_fScale = scale;}


    float fieldValue(vec3f p)
    {
        float fDist2 = m_position.dist2(p);

        float fValue = (1.0f - (fDist2 / (m_fRadius * m_fRadius)));
        if(fValue <= 0.0f)
            return 0.0f;

        return m_fScale  * fValue * fValue;
    }

    std::string getName()
    {
        return "QUADRICPOINT";
    }

    COctree computeOctree()
    {
        m_octree.set(m_position - m_fRadius, m_position + m_fRadius);
        return m_octree;
    }


    bool isOperator() { return false;}
    BlobNodeType getNodeType() {return bntPrimQuadricPoint;}

    bool saveScript(CSketchConfig* lpSketchScript)
    {      
        //Write parameters for RicciBlend
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", this->getID());
        lpSketchScript->writeVec3f(strNodeName, "position", this->getPosition());
        lpSketchScript->writeFloat(strNodeName, "scale", this->getFieldScale());
        lpSketchScript->writeFloat(strNodeName, "radius", this->getFieldRadius());
        return this->saveGenericInfoScript(lpSketchScript);
    }

    bool loadScript(CSketchConfig* lpSketchScript, int id)
    {
        //Write parameters for RicciBlend
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        m_position = lpSketchScript->readVec3f(strNodeName, "position");
        m_fScale   = lpSketchScript->readFloat(strNodeName, "scale");
        m_fRadius  = lpSketchScript->readFloat(strNodeName, "radius");
        return this->loadGenericInfoScript(lpSketchScript, id);
    }

};

}
}
#endif
