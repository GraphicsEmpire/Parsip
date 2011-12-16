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

    CSkeletonRing(CSkeleton* other)
    {
        setParamFrom(other);
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

    bool getExtremes(vec3f& lower, vec3f& upper)
    {
        vec3f dir = vec3f(cos(m_direction.x * PiOver2),
                          cos(m_direction.y * PiOver2),
                          cos(m_direction.z * PiOver2));
        lower = m_center - m_radius * dir;
        upper = m_center + m_radius * dir;
        return true;
    }

    Vol::CVolume* getBoundingVolume(float range)
    {
        m_direction.normalize();
        Vol::CVolumeSphere* s = new Vol::CVolumeSphere(m_center, m_radius + range);
        vec3f bv(cos(m_direction.x) * m_radius + range,
                 cos(m_direction.y) * m_radius + range,
                 cos(m_direction.z) * m_radius + range);

        Vol::CVolumeBox* b = new Vol::CVolumeBox(m_center - bv, m_center +  bv);
        if (s->size() > b->size())
        {
            delete b; b = NULL;
            return s;
        }
        else
        {
            delete s; s = NULL;
            return b;
        }
    }

    vec3f getPolySeedPoint()
    {
        //vec3f perp = m_direction.findArbitaryNormal();
        //return m_center + m_radius*perp;
        return m_center;
    }

    void translate(vec3f d)
    {
        m_center += d;
    }

    SkeletonType getType()		{return sktRing;}

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
