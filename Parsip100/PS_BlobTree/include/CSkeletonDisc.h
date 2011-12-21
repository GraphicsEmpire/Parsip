#ifndef CSKELETONDISC_H
#define CSKELETONDISC_H


#include "CSkeleton.h"

namespace PS{
namespace BLOBTREE{

//Implementation of Skeleton types like point, line, disk, circle, cylinder, hollowCylinder
class  CSkeletonDisc: public CSkeleton 
{
private:
    vec3f m_center;
    vec3f m_direction;
    float m_radius;
public:
    CSkeletonDisc()
    {
        m_center.set(0.0f, 0.0f, 0.0f);
        m_direction.set(0.0f, 0.0f, 1.0f);
        m_radius = 1.0f;
    }

    CSkeletonDisc(vec3f position, vec3f direction, float radius)
    {
        m_center = position;
        m_direction = direction;
        m_radius = radius;
        m_direction.normalize();
    }

    CSkeletonDisc(CSkeleton* other)
    {
        setParamFrom(other);
    }

    void setParamFrom(CSkeleton* input)
    {
        CSkeletonDisc* discN = dynamic_cast<CSkeletonDisc*>(input);
        this->m_center	  = discN->m_center;
        this->m_direction = discN->m_direction;
        this->m_radius    = discN->m_radius;
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
        if(dir.length() <= m_radius)
        {
            return abs((p - m_center).length2() - dir.length2());
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
        return distance(p);
    }

    string getName()
    {
        return "DISC";
    }

    bool getExtremes(vec3f& lower, vec3f& upper)
    {
        vec3f dir;
        dir.x = cos(m_direction.x * PiOver2);
        dir.y = cos(m_direction.y * PiOver2);
        dir.z = cos(m_direction.z * PiOver2);

        lower = m_center - m_radius * dir;
        upper = m_center + m_radius * dir;
        return true;
    }

    VOL::CVolume* getBoundingVolume(float range)
    {
        m_direction.normalize();
        VOL::CVolumeSphere* s = new VOL::CVolumeSphere(m_center, m_radius + range);
        return s;
    }

    vec3f getPolySeedPoint()
    {
        return m_center;
    }

    void translate(vec3f d)
    {
        m_center += d;
    }

    BlobNodeType getType()		{return bntPrimDisc;}

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
