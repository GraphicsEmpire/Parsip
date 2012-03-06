#ifndef CSKELETONRING_H
#define CSKELETONRING_H

#include "CSkeleton.h"

namespace PS{
namespace BLOBTREE{

//Implementation of Skeleton types like point, line, disk, circle, cylinder, hollowCylinder
class  CSkeletonRing: public CSkeleton 
{
private:
    vec3f m_center;
    vec3f m_direction;
    float m_radius;

public:
    CSkeletonRing()
    {
        m_center.set(0.0f, 0.0f, 0.0f);
        m_direction.set(0.0f, 0.0f, 1.0f);
        m_radius = 1.0f;
    }
    CSkeletonRing(vec3f center)
    {
        m_center = center;
        m_direction.set(0.0f, 1.0f, 0.0f);
        m_radius = 1.0f;
    }

    CSkeletonRing(vec3f center, vec3f dir)
    {
        m_center = center;
        m_direction = dir;
        m_radius = 1.0f;
    }

    CSkeletonRing(vec3f center, vec3f dir, float radius)
    {
        m_center = center;
        m_direction = dir;
        m_radius = radius;
    }

    void setParamFrom(CSkeleton* input)
    {
        CSkeletonRing* ringN = dynamic_cast<CSkeletonRing*>(input);
        this->m_center = ringN->m_center;
        this->m_direction = ringN->m_direction;
        this->m_radius    = ringN->m_radius;
    }

    vec3f getPosition() const { return m_center;}
    void setPosition(vec3f pos) { m_center = pos;}

    vec3f getDirection() const { return m_direction;}
    void setDirection(vec3f dir) { m_direction = dir;}

    float getRadius() const { return m_radius;}
    void setRadius(float radius) { m_radius = radius;}

    float distance(vec3f p)
    {
        return sqrt(squareDistance(p));
    }

    float squareDistance(vec3f p)
    {
        //dir = Q-C = p - c - (N.(p-c))N
        vec3f n = m_direction;
        vec3f c = m_center;
        vec3f dir = p - c - (n.dot(p - c))*n;

        //Check if Q lies on center or p is just above center
        if(dir.isZero())
        {
            //r^2 + |p-c|^2
            return (m_radius*m_radius + (p - m_center).length2());
        }
        else
        {
            dir.normalize();
            vec3f x = c + m_radius * dir;
            return (x - p).length2();
        }
    }

    vec3f normal(vec3f p)
    {
        vec3f n = p - m_center;
        n.normalize();
        return n;
    }

    float getDistanceAndNormal(vec3f p, vec3f& normal)
    {
        normal = this->normal(p);
        normal.normalize();
        return distance(p);
    }

    string getName()
    {
        return "RING";
    }

    BBOX bound() const
    {
        vec3f dirComp = vec3f(1.0f, 1.0f, 1.0f) - m_direction;
        float radius = m_radius + ISO_VALUE;
        vec3f expand = radius * dirComp + ISO_VALUE * m_direction;
        BBOX box(m_center - expand, m_center + expand);
        return box;
    }

    vec3f getPolySeedPoint()
    {
        return m_center;
    }

    void translate(vec3f d)
    {
        m_center += d;
    }

    BlobNodeType getType()		{return bntPrimRing;}

    bool saveScript(CSketchConfig *lpSketchScript, int id)
    {
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        lpSketchScript->writeVec3f(strNodeName, "position", this->getPosition());
        lpSketchScript->writeVec3f(strNodeName, "direction", this->getDirection());
        lpSketchScript->writeFloat(strNodeName, "radius", this->getRadius());
        return true;
    }

    bool loadScript(CSketchConfig *lpSketchScript, int id)
    {
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        m_center = lpSketchScript->readVec3f(strNodeName, "position");
        m_direction = lpSketchScript->readVec3f(strNodeName, "direction");
        m_radius = lpSketchScript->readFloat(strNodeName, "radius");
        return true;
    }

};

}
}

#endif
