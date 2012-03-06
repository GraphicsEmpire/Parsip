#ifndef COCTREE_H
#define COCTREE_H

#include "PS_Vector.h"
#include "PS_Quaternion.h"
#include "PS_Ray.h"
#include "_dataTypes.h"

using namespace std;
using namespace PS::MATH;
//This class will manage an octree
//If a box is divided it will produce 8 cubes
//As children

//Directions
namespace PS{


class COctree
{
private:
    bool m_bHasChildren;
public:
    COctree();
    COctree(const vec3f& lo, const vec3f& hi);

    virtual ~COctree();

public:
    enum SIDE {xSide, ySide, zSide};
    enum CORNERS {LBN=0, LBF=1, LTN=2, LTF=3, RBN=4, RBF=5, RTN=6, RTF=7};
    enum DIRECTION {L=0, R=1, B=2, T=3, N=4, F=5};

    vec3f lower;
    vec3f upper;
    COctree* children[8];

    void init();
    void set(const std::vector<vec3f>& lstPoints);
    void set(vec3f lstPoints[], int ctPoints);
    void set(const vec3f& lo, const vec3f& hi);

    bool isValid() const { return ((lower.x < upper.x)&&(lower.y < upper.y)&&(lower.z < upper.z));}

    //Children Management
    bool isIndex(int index) const {return ((index>=0)&&(index < 8));}
    COctree * getChild(int index);
    bool setChild(int index, COctree* child);
    void removeAllChildren();

    //Dimensions
    vec3f getSidesSize();
    float getSideSize(SIDE which = xSide);
    float getMaxSideSize();
    float getMinSideSize();

    //Collision Detection
    bool isInside(const vec3f& pt) const;
    bool intersect(const vec3f& lo, const vec3f& hi) const;
    bool intersect(COctree* other) const;
    bool intersect(const CRay& ray, float t0, float t1) const;

    //Expand and Compress
    void expand(float offset);
    void expand(const vec3f& v);
    void expand(vec3f lstPoints[], int ctPoints);

    inline vec3f bounds(int idx) const { return (idx == 0)?lower:upper;}
    vec3f getCorner(int index) const;
    vec3f center() const;
    void subDivide();
    void correct();

    //Affine
    void scale(const vec3f& s);
    void rotate(const quat& r);
    void translate(const vec3f& t);
    void transform(const CMatrix& m);

    //CSG
    void csgUnion(const vec3f& rhsLo, const vec3f& rhsHi);
    void csgUnion( const COctree& rhs );
    void csgIntersection( const COctree& rhs );

    bool hasChildren() const;

    COctree& operator=(const COctree& rhs)
    {
        lower = rhs.lower;
        upper = rhs.upper;
        return(*this);
    }
};

}
#endif
