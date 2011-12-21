#ifndef CSKELETONCYLINDER_H
#define CSKELETONCYLINDER_H

#include "CSkeleton.h"

namespace PS{
namespace BLOBTREE{

//Implementation of Skeleton types like point, line, disk, circle, cylinder, hollowCylinder
class  CSkeletonCylinder: public CSkeleton
{
private:
    vec3f m_position;
    vec3f m_direction;
    float m_radius;
    float m_height;
public:
    CSkeletonCylinder()
    {
        m_position.set(-1.0f, 0.0f, 0.0f);
        m_direction.set(1.0f, 0.0f, 0.0f);
        m_radius = 0.01f;
        m_height = 4.0f;
    }

    CSkeletonCylinder(vec3f position, vec3f direction, float radius, float height)
    {
        m_position = position;
        m_direction = direction;
        m_radius = radius;
        m_height = height;

        m_direction.normalize();
    }

    void setParamFrom(CSkeleton* input)
    {
        CSkeletonCylinder* cylN = dynamic_cast<CSkeletonCylinder*>(input);
        this->m_position = cylN->m_position;
        this->m_direction = cylN->m_direction;
        this->m_radius    = cylN->m_radius;
        this->m_height    = cylN->m_height;
    }

    vec3f getPosition() const { return m_position;}
    void setPosition(vec3f pos) { m_position = pos;}

    vec3f getDirection() const { return m_direction;}
    void setDirection(vec3f dir) { m_direction = dir;}

    float getHeight() const { return m_height;}
    void setHeight(float height) { m_height = height;}

    float getRadius() const { return m_radius;}
    void setRadius(float radius) { m_radius = radius;}

    float distance(vec3f p)
    {
        return sqrt(squareDistance(p));
    }

    float squareDistance(vec3f p)
    {
        vec3f pos = p - m_position;
        float y = pos.dot(m_direction);
        float x = maxf(0.0f, sqrt(pos.length2() - y*y) - m_radius);

        //Make y 0.0 if it is positive and less than height
        // For Hemispherical caps
        if(y > 0)
            y = maxf(0.0f, y - m_height);

        return x*x + y*y;
    }

    vec3f normal(vec3f p)
    {
        vec3f n = p - m_position;
        n.normalize();
        return n;
    }

    float getDistanceAndNormal(vec3f p, vec3f& normal)
    {
        vec3f pos = p - m_position;
        normal = pos;
        normal.normalize();

        float y = pos.dot(m_direction);
        float x = maxf(0.0f, sqrt(pos.length2() - y*y) - m_radius);
        if(y > 0)
            y = maxf(0.0f, y - m_height);

        return sqrt(x*x + y*y);

    }

    string getName()
    {
        return "CYLINDER";
    }

    bool getExtremes(vec3f& lower, vec3f& upper)
    {
        lower = m_position - m_radius;
        upper = m_position + (m_height + m_radius) * m_direction;
        return true;
    }

    VOL::CVolume* getBoundingVolume(float range)
    {
        m_direction.normalize();
        float r = sqrtf(m_radius * m_radius + 0.25f * m_height * m_height);
        VOL::CVolumeSphere * s = new VOL::CVolumeSphere(vec3f(), r + range);

        vec3f d  = (0.5f * m_height) * m_direction;
        vec3f da = d.vectorAbs();
        vec3f bv = da + vec3f(cos(m_direction.x) + range, cos(m_direction.y) + range, cos(m_direction.z) + range).vectorAbs();

        vec3f p = m_position + da;
        VOL::CVolumeBox * b = new VOL::CVolumeBox(p - bv, p + bv);
        if (s->size() > b->size())
        {
            s->setCenter(m_position + d);
            delete b;
            b = NULL;

            return s;
        }
        else
        {
            delete s;
            s = NULL;
            return b;
        }

    }

    vec3f getPolySeedPoint()
    {
        //Obtain a perpendicular vector to the direction vector
        vec3f perp = m_direction.findArbitaryNormal();
        return m_position + (m_radius + 0.001f)*perp;
    }

    void translate(vec3f d)
    {
        m_position += d;
    }

    BlobNodeType getType()		{return bntPrimCylinder;}

    bool saveScript(CSketchConfig *lpSketchScript, int id)
    {
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        lpSketchScript->writeVec3f(strNodeName, "position", this->getPosition());
        lpSketchScript->writeVec3f(strNodeName, "direction", this->getDirection());
        lpSketchScript->writeFloat(strNodeName, "radius", this->getRadius());
        lpSketchScript->writeFloat(strNodeName, "height", this->getHeight());
        return true;
    }

    bool loadScript(CSketchConfig *lpSketchScript, int id)
    {
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        m_position = lpSketchScript->readVec3f(strNodeName, "position");
        m_direction = lpSketchScript->readVec3f(strNodeName, "direction");
        m_radius = lpSketchScript->readFloat(strNodeName, "radius");
        m_height = lpSketchScript->readFloat(strNodeName, "height");
        return true;
    }

};

}
}

#endif
