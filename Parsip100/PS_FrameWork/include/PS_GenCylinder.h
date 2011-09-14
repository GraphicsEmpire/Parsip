#ifndef CGenCylinder_H
#define CGenCylinder_H

#include <vector>
#include "PS_SplineCatmullRom.h"

#define DEFAULT_STACKS 10
#define DEFAULT_SECTORS 3
#define DEFAULT_RADIUS 0.25f

using namespace PS::MATH;

namespace PS{
namespace MODELING{

/*!
 *	Cross Section for a generalized cylinder
 *
 */
class CGenCylinderCrossSection
{

public:
    CGenCylinderCrossSection();

    CGenCylinderCrossSection(int nSectors, float radius);

    CGenCylinderCrossSection(const CGenCylinderCrossSection& rhs);

    ~CGenCylinderCrossSection();

    /*!
     *
     */
    void set(const CGenCylinderCrossSection& rhs);

    void removeAllPoints();

    //
    void calc();

    //Access
    int getSectorsCount() const {return m_ctSectors;}
    void setSectors(int sectors) { m_ctSectors = sectors;}

    float getRadius() const {return m_radius;}
    void setRadius(const float radius){ m_radius = radius;}

    vec3f getPosition() const {return m_position;}
    void setPosition(vec3f v) { m_position = v;}

    void getFrenetFrame(vec3f& T, vec3f& N, vec3f& B) const;
    void setFrenetFrame(vec3f T, vec3f N, vec3f B);

    //
    vec3f getNormal(size_t i) const;
    vec3f getPoint(size_t i) const;
    size_t getPointsCount() const { return m_lstPoints.size();}

    //
    //void draw(int glArg = GL_POLYGON);

private:
    int m_ctSectors;
    float m_radius;
    vec3f m_position;

    vec3f m_tangent;
    vec3f m_normal;
    vec3f m_binormal;

    std::vector<vec3f> m_lstPoints;
    std::vector<vec3f> m_lstNormals;
};

//////////////////////////////////////////////////////////////////////////////////
/*!
 * Generalized Cylinder
 */
class CGenCylinder
{
public:
    CGenCylinder();
    CGenCylinder(int nSectors, int nStacks, float startRadius, float endRadius);
    CGenCylinder(const CGenCylinder& rhs);
    ~CGenCylinder();

    void removeAll();

    //StacksRadius
    void setStacks(int stacks) { m_ctStacks = stacks;}
    int getStacksCount() const {return m_ctStacks;}

    //Sectors
    void setSectors(int sectors) { m_ctSectors = sectors; }
    int getSectorsCount() const {return m_ctSectors;}

    //Start Radius
    void setRadiusStart(const float r1)    {  m_radiusStart = r1;  }
    float getRadiusStart() const { return m_radiusStart;}

    //End Radius
    void setRadiusEnd(const float r2)    {  m_radiusEnd = r2; }
    float getRadiusEnd() const { return m_radiusEnd;}

    //GetCurve
    CSplineCatmullRom& getCurve() {return m_profileCurve;}

    CGenCylinderCrossSection* getStack(int idx);

    bool calc();

    /*
    void drawCurveControlPoints(bool bSelectMode = false);

    void drawCurve();

    void drawCrossSections();

    void drawWireFrame();

    void drawNormals();

    void drawSurface();

*/
private:
    int m_ctStacks;
    int m_ctSectors;
    float m_radiusStart;
    float m_radiusEnd;

    CSplineCatmullRom m_profileCurve;
    std::vector<CGenCylinderCrossSection*> m_lstStacks;
};

}
}

#endif
