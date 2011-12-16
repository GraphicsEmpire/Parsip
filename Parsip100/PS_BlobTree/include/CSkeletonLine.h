#ifndef CSKELETONLINE_H
#define CSKELETONLINE_H

#include "CSkeleton.h"

namespace PS{
namespace BLOBTREE{

//Implementation of Skeleton types like point, line, disk, circle, cylinder, hollowCylinder
class  CSkeletonLine: public CSkeleton 
{
private:
    vec3f m_ptStart;
    vec3f m_ptEnd;
public:
    CSkeletonLine()
    {
        m_ptStart.set(-1.0f, 0.0f, 0.0f);
        m_ptEnd.set(1.0f, 0.0f, 0.0f);
    }

    CSkeletonLine(float length)
    {
        m_ptStart.set(0.0f, -0.5f * length, 0.0f);
        m_ptEnd.set(0.0f, 0.5f * length, 0.0f);
    }

    CSkeletonLine(vec3f start, vec3f end)
    {
        m_ptStart = start;
        m_ptEnd = end;
    }

    CSkeletonLine(CSkeleton* other)
    {
        setParamFrom(other);
    }

    void setParamFrom(CSkeleton* input)
    {
        CSkeletonLine* lineN = dynamic_cast<CSkeletonLine*>(input);
        this->m_ptStart = lineN->m_ptStart;
        this->m_ptEnd = lineN->m_ptEnd;
    }


    vec3f getStartPosition() const { return m_ptStart;}
    void setStartPosition(vec3f pos) { m_ptStart = pos;}

    vec3f getEndPosition() const { return m_ptEnd;}
    void setEndPosition(vec3f pos) { m_ptEnd = pos;}

    float distance(vec3f p)
    {
        vec3f nearestPoint = NearestPointInLineSegment(p, m_ptStart, m_ptEnd);
        return nearestPoint.distance(p);
    }

    float squareDistance(vec3f p)
    {
        vec3f nearestPoint = NearestPointInLineSegment(p, m_ptStart, m_ptEnd);
        return nearestPoint.dist2(p);
    }

    vec3f normal(vec3f p)
    {
        vec3f normal = p - NearestPointInLineSegment(p, m_ptStart, m_ptEnd);
        normal.normalize();
        return normal;
    }

    float getDistanceAndNormal(vec3f p, vec3f& normal)
    {
        vec3f nearestPoint = NearestPointInLineSegment(p, m_ptStart, m_ptEnd);
        normal = p - nearestPoint;
        normal.normalize();
        return nearestPoint.distance(p);
    }

    string getName()
    {
        return "LINE";
    }

    bool getExtremes(vec3f& lower, vec3f& upper)
    {
        lower = m_ptStart;
        upper = m_ptEnd;
        return true;
    }

    Vol::CVolume* getBoundingVolume(float range)
    {
        Vol::CVolumeBox * b = new Vol::CVolumeBox(m_ptStart.vectorMin(m_ptEnd).subtract(range),
                                                  m_ptStart.vectorMax(m_ptEnd).add(range));
        return b;
    }

    vec3f getPolySeedPoint()
    {
        return (m_ptStart + m_ptEnd) * 0.5f;
    }

    void translate(vec3f d)
    {
        m_ptStart += d;
        m_ptEnd += d;
    }

    SkeletonType getType()		{return sktLine;}


    bool saveScript(CSketchConfig *lpSketchScript, int id)
    {
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        lpSketchScript->writeVec3f(strNodeName, "start", this->getStartPosition());
        lpSketchScript->writeVec3f(strNodeName, "end", this->getEndPosition());
        return true;
    }

    bool loadScript(CSketchConfig *lpSketchScript, int id)
    {
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", id);
        m_ptStart = lpSketchScript->readVec3f(strNodeName, "start");
        m_ptEnd = lpSketchScript->readVec3f(strNodeName, "end");
        return true;
    }


};
}
}

#endif
