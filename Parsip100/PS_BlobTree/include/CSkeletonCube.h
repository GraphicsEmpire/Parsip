#ifndef CSKELETONCUBE_H
#define CSKELETONCUBE_H

#include "CSkeleton.h"

namespace PS{
namespace BLOBTREE{

//Implementation of Skeleton types like point, line, disk, circle, cylinder, hollowCylinder
class  CSkeletonCube: public CSkeleton
{
private:
    vec3f m_center;
    float m_side;
public:
    CSkeletonCube()
    {
        m_center.set(0.0f, 0.0f, 0.0f);
        m_side = 1.0f;
    }

    CSkeletonCube(vec3f position, float side)
    {
        m_center = position;
        m_side = side;
    }

    CSkeletonCube(CSkeleton* other)
    {
        setParamFrom(other);
    }

    void setParamFrom(CSkeleton* input)
    {
        CSkeletonCube* cubeN = dynamic_cast<CSkeletonCube*>(input);
        this->m_center = cubeN->m_center;
        this->m_side = cubeN->m_side;
    }

    vec3f getPosition() const { return m_center;}
    void setPosition(vec3f pos) { m_center = pos;}

    float getSide() const {return m_side;}
    void setSide(float side) {m_side = side;}


    float distance(vec3f p)
    {
        return sqrt(squareDistance(p));
    }

    float squareDistance(vec3f p)
    {
        vec3f dif = p - m_center;
        float dist2 = 0.0f;
        float delta;

        float projected;

        //Along X
        projected = dif.dot(vec3f(1.0f, 0.0f, 0.0f));
        if(projected < -1.0f * m_side)
        {
            delta = projected + m_side;
            dist2 += delta*delta;
        }
        else if (projected > m_side)
        {
            delta = projected - m_side;
            dist2 += delta*delta;
        }

        //Along Y
        projected = dif.dot(vec3f(0.0f, 1.0f, 0.0f));
        if(projected < -1.0f * m_side)
        {
            delta = projected + m_side;
            dist2 += delta*delta;
        }
        else if (projected > m_side)
        {
            delta = projected - m_side;
            dist2 += delta*delta;
        }

        //Along Z
        projected = dif.dot(vec3f(0.0f, 0.0f, 1.0f));
        if(projected  < -1.0f * m_side)
        {
            delta = projected + m_side;
            dist2 += delta*delta;
        }
        else if (projected > m_side)
        {
            delta = projected - m_side;
            dist2 += delta*delta;
        }

        return dist2;
    }

    float squareDistance(vec3f p, vec3f& outClosestCubePoint)
    {
        vec3f dif = p - m_center;
        float dist2 = 0.0f;
        float delta;

        vec3f closest;

        closest.x = dif.dot(vec3f(1.0f, 0.0f, 0.0f));
        if(closest.x  < -1.0f * m_side)
        {
            delta = closest.x + m_side;
            dist2 += delta*delta;
            closest.x = -1.0f*m_side;
        }
        else if (closest.x > m_side)
        {
            delta = closest.x - m_side;
            dist2 += delta*delta;
            closest.x = m_side;
        }

        closest.y = dif.dot(vec3f(0.0f, 1.0f, 0.0f));
        if(closest.y  < -1.0f * m_side)
        {
            delta = closest.y + m_side;
            dist2 += delta*delta;
            closest.y = -1.0f*m_side;
        }
        else if (closest.y > m_side)
        {
            delta = closest.y - m_side;
            dist2 += delta*delta;
            closest.y = m_side;
        }

        closest.z = dif.dot(vec3f(0.0f, 0.0f, 1.0f));
        if(closest.z  < -1.0f * m_side)
        {
            delta = closest.z + m_side;
            dist2 += delta*delta;
            closest.z = -1.0f*m_side;
        }
        else if (closest.z > m_side)
        {
            delta = closest.z - m_side;
            dist2 += delta*delta;
            closest.z = m_side;
        }

        //Closest point on the cube
        outClosestCubePoint = m_center + closest;
        return dist2;
    }

    vec3f normal(vec3f p)
    {
        vec3f onCube;
        squareDistance(p, onCube);
        vec3f n = onCube - m_center;
        n.normalize();
        return n;
    }

    float getDistanceAndNormal(vec3f p, vec3f& normal)
    {
        vec3f onCube;
        float dist2 = squareDistance(p, onCube);
        normal = onCube - m_center;
        normal.normalize();
        return sqrt(dist2);
    }

    string getName()
    {
        return "CUBE";
    }

    bool getExtremes(vec3f& lower, vec3f& upper)
    {
        lower = m_center - m_side;
        upper = m_center + m_side;
        return true;
    }

    Vol::CVolume* getBoundingVolume(float range)
    {
        Vol::CVolumeBox * b = new Vol::CVolumeBox(m_center - m_side - range, m_center + m_side + range);
        return b;
    }

    vec3f getPolySeedPoint()
    {
        return m_center;
    }

    void translate(vec3f d)
    {
        m_center += d;
    }

    SkeletonType getType()		{return sktCube;}

    bool saveScript(CSketchConfig *lpSketchScript, int id)
    {
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        lpSketchScript->writeVec3f(strNodeName, "position", this->getPosition());
        lpSketchScript->writeFloat(strNodeName, "side", this->getSide());
        return true;
    }

    bool loadScript(CSketchConfig *lpSketchScript, int id)
    {
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        m_center = lpSketchScript->readVec3f(strNodeName, "position");
        m_side = lpSketchScript->readFloat(strNodeName, "side");
        return true;
    }

};

}
}

#endif
