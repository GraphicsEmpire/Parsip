#ifndef CSKELETONPOINT_H
#define CSKELETONPOINT_H

#include "CSkeleton.h"

namespace PS{
namespace BLOBTREE{

//Implementation of Skeleton types like point, line, disk, circle, cylinder, hollowCylinder
class  CSkeletonPoint: public CSkeleton 
{
private:
    vec3f m_position;
public:
    CSkeletonPoint() { m_position.set(0.0f, 0.0f, 0.0f);}
    CSkeletonPoint(vec3f position)
    {
        m_position = position;
    }

    void setParamFrom(CSkeleton* input)
    {
        CSkeletonPoint* pointN = dynamic_cast<CSkeletonPoint*>(input);
        this->m_position = pointN->m_position;
    }

    vec3f getPosition() const { return m_position;}
    void setPosition(vec3f pos) { m_position = pos;}

    float distance(vec3f p)
    {
        return m_position.distance(p);
    }

    float squareDistance(vec3f p)
    {
        return m_position.dist2(p);
    }

    vec3f normal(vec3f p)
    {
        vec3f n = p - m_position;
        n.normalize();
        return n;
    }

    float getDistanceAndNormal(vec3f p, vec3f& normal)
    {
        normal = p - m_position;
        normal.normalize();
        return m_position.distance(p);
    }

    string getName()
    {
        return "POINT";
    }

    bool getExtremes(vec3f& lower, vec3f& upper)
    {
        lower = m_position;
        upper = m_position;
        return false;
    }

    VOL::CVolume* getBoundingVolume(float range)
    {
        VOL::CVolumeSphere * s = new VOL::CVolumeSphere(m_position, range);
        return s;
    }

    vec3f getPolySeedPoint()
    {
        return m_position;
    }

    void translate(vec3f d)
    {
        m_position += d;
    }

    BlobNodeType getType()		{return bntPrimPoint;}

    bool saveScript(CSketchConfig* lpSketchScript, int id)
    {
        //Write parameters for RicciBlend
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        lpSketchScript->writeVec3f(strNodeName, "position", this->getPosition());
        return true;
    }

    bool loadScript(CSketchConfig* lpSketchScript, int id)
    {
        //Write parameters for RicciBlend
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        m_position = lpSketchScript->readVec3f(strNodeName, "position");
        return true;
    }


};

}
}
#endif
