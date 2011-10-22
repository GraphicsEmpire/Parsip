#ifndef COCTREE_H
#define COCTREE_H

#include "PS_Vector.h"
#include "PS_Quaternion.h"
#include "PS_Ray.h"
#include "_dataTypes.h"

using namespace std;

//This class will manage an octree
//If a box is divided it will produce 8 cubes
//As children

//Directions
namespace PS{

using namespace PS::MATH;

class COctree
{
private:
	bool m_bHasChildren;
public:
	COctree();
	COctree(vec3 lbnCorner, vec3 rtfCorner);
	COctree(float l, float b, float n, float r, float t, float f);
	COctree(vec3 lstPoints[], int ctPoints);

	~COctree();

public:
        enum SIDE {xSide, ySide, zSide};
        enum CORNERS {LBN=0, LBF=1, LTN=2, LTF=3, RBN=4, RBF=5, RTN=6, RTF=7};
        enum DIRECTION {L=0, R=1, B=2, T=3, N=4, F=5};

	vec3 lower; 
	vec3 upper;	
	
	COctree* children[8];
		
	void set(const std::vector<vec3f>& lstPoints);
	void set(vec3 lstPoints[], int ctPoints);
	void set(vec3f lo, vec3f hi);

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
	bool isInside(vec3 pt);
	bool intersect(vec3f lo, vec3f hi) const;
	bool intersect(COctree* other) const;
	bool intersect(const CRay& ray, float t0, float t1) const;

	//Expand and Compress
	void expand(float offset);
	void expand(vec3f v);
	void expand(vec3 lstPoints[], int ctPoints);

	__inline vec3 bounds(int idx) const { return (idx == 0)?lower:upper;}
	vec3 getCorner(int index);
	vec3 center();
	void subDivide();
	void correct();
	
	//Affine
	void translate(vec3f displacement);
	void scale(vec3f displacement);
	void rotate(quat q);
	void transform(const CMatrix& m);

	//CSG
	void csgUnion(vec3f rhsLo, vec3f rhsHi);
	void csgUnion( const COctree& rhs );
	void csgIntersection( const COctree& rhs );
	
	bool hasChildren(); 
	virtual void draw();

	COctree& operator=(const COctree& rhs)
	{
		lower = rhs.lower;
		upper = rhs.upper;
		return(*this);
	}
};

}
#endif
