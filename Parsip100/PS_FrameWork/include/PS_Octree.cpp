#include "PS_Octree.h"

//#include <gl/GL.h>
//#include <gl/GLU.h>



namespace PS{


COctree::COctree()
{
    for(int i=0;i<8; i++)
        children[i] = NULL;
    m_bHasChildren = false;
    lower.zero();
    upper.zero();
}

COctree::COctree(vec3f lbnCorner, vec3f rtfCorner)
{
    for(int i=0;i<8; i++)
        children[i] = NULL;
    m_bHasChildren = false;
    lower = lbnCorner;
    upper = rtfCorner;
}

COctree::COctree(float l, float b, float n, float r, float t, float f)
{
    for(int i=0;i<8; i++)
        children[i] = NULL;
    m_bHasChildren = false;
    lower = vec3f(l,b,n);
    upper = vec3f(r,t,f);
}

COctree::COctree(vec3f lstPoints[], int ctPoints)
{	
    m_bHasChildren = false;
    set(lstPoints, ctPoints);
}

COctree::~COctree()
{	
    removeAllChildren();
}

void COctree::set(const std::vector<vec3f>& lstPoints)
{
    removeAllChildren();

    vec3f vMax, vMin, p;
    vMin = lstPoints[0];
    vMax = lstPoints[0];

    for(int i=1; i<lstPoints.size(); i++)
    {
        p = lstPoints[i];
        if(p.x > vMax.x ) vMax.x = p.x;
        if(p.y > vMax.y ) vMax.y = p.y;
        if(p.z > vMax.z ) vMax.z = p.z;
        if(p.x < vMin.x ) vMin.x = p.x;
        if(p.y < vMin.y ) vMin.y = p.y;
        if(p.z < vMin.z ) vMin.z = p.z;
    }
    lower = vMin;
    upper = vMax;
}

void COctree::set(vec3f lstPoints[], int ctPoints)
{
    removeAllChildren();

    vec3f vMax, vMin, p;
    vMin = lstPoints[0];
    vMax = lstPoints[0];

    for(int i=1; i<ctPoints; i++)
    {
        p = lstPoints[i];
        if(p.x > vMax.x ) vMax.x = p.x;
        if(p.y > vMax.y ) vMax.y = p.y;
        if(p.z > vMax.z ) vMax.z = p.z;
        if(p.x < vMin.x ) vMin.x = p.x;
        if(p.y < vMin.y ) vMin.y = p.y;
        if(p.z < vMin.z ) vMin.z = p.z;
    }
    lower = vMin;
    upper = vMax;
}

void COctree::set( vec3f lo, vec3f hi )
{
    this->lower = lo;
    this->upper = hi;
}

void COctree::removeAllChildren()
{
    for (int i=0; i<8; i++)
        SAFE_DELETE(children[i]);
    m_bHasChildren = false;
}


bool COctree::hasChildren()
{
    return m_bHasChildren;
}

vec3f COctree::center()
{
    vec3f c = (lower + upper) * 0.5f;
    return c;
}

bool COctree::setChild(int index, COctree* child)
{
    if(!isIndex(index)) return false;

    //Free the slot holding current child
    if(this->children[index])
    {
        delete this->children[index];
        this->children[index] = NULL;
    }

    this->children[index] = child;

    return true;
}

COctree* COctree::getChild(int index)
{
    if(index >=0 && index < 8)
        return children[index];
    else return NULL;
}

float COctree::getMaxSideSize()
{
    float xs = getSideSize(xSide);
    float ys = getSideSize(ySide);
    float zs = getSideSize(zSide);
    float mx = xs;
    if(ys > mx)
        mx = ys;
    if(zs > mx)
        mx = zs;
    return mx;
}

float COctree::getMinSideSize()
{
    float xs = getSideSize(xSide);
    float ys = getSideSize(ySide);
    float zs = getSideSize(zSide);
    float mi = xs;
    if(ys < mi)
        mi = ys;
    if(zs < mi)
        mi = zs;
    return mi;
}

vec3f COctree::getSidesSize()
{
    return upper-lower;
}

float COctree::getSideSize(SIDE which)
{	
    float side = 0.0f;
    switch (which)
    {
    case xSide:
        side = this->upper.x - this->lower.x;
        break;
    case ySide:
        side = this->upper.y - this->lower.y;
        break;
    case zSide:
        side = this->upper.z - this->lower.z;
        break;
    }
    return side;
}

void COctree::subDivide()
{	
    if (hasChildren())	removeAllChildren();

    vec3f c = center();
    float l = lower.x; float r = upper.x;
    float b = lower.y; float t = upper.y;
    float n = lower.z; float f = upper.z;
    float lrov2 = (l+r)/2.0f;
    float btov2 = (b+t)/2.0f;
    float nfov2 = (n+f)/2.0f;

    children[LBN] = new COctree(vec3f(l, b, n), vec3f(lrov2, btov2, nfov2));
    children[LBF] = new COctree(vec3f(l, b, nfov2), vec3f(lrov2, btov2, f));
    children[LTN] = new COctree(vec3f(l, btov2, n), vec3f(lrov2, t, nfov2));
    children[LTF] = new COctree(vec3f(l, btov2, nfov2), vec3f(lrov2, t, f));

    children[RBN] = new COctree(vec3f(lrov2, b, n), vec3f(r, btov2, nfov2));
    children[RBF] = new COctree(vec3f(lrov2, b, nfov2), vec3f(r, btov2, f));
    children[RTN] = new COctree(vec3f(lrov2, btov2, n), vec3f(r, t, nfov2));
    children[RTF] = new COctree(vec3f(lrov2, btov2, nfov2), vec3f(r, t, f));
    m_bHasChildren = true;
}

void COctree::correct()
{
    float temp;
    if(lower.x > upper.x)
    {
        temp = lower.x;
        lower.x = upper.x;
        upper.x = temp;
    }

    if(lower.y > upper.y)
    {
        temp = lower.y;
        lower.y = upper.y;
        upper.y = temp;
    }

    if(lower.z > upper.z)
    {
        temp = lower.z;
        lower.z = upper.z;
        upper.z = temp;
    }
}
/*
void COctree::drawPolygon()
{
 glPushAttrib(GL_ALL_ATTRIB_BITS);

 const float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
 glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
 //glFrontFace(GL_CCW);
 glBegin(GL_QUADS);


 glEnd();
 glPopAttrib();

}
*/

bool COctree::isInside(vec3f pt)
{
    if((pt.x < lower.x)||(pt.x > upper.x))
        return false;
    if((pt.y < lower.y)||(pt.y > upper.y))
        return false;
    if((pt.z < lower.z)||(pt.z > upper.z))
        return false;
    return true;
}

bool COctree::intersect(vec3f lo, vec3f hi) const
{
    if ((lower.x >= hi.x) || (upper.x <= lo.x))
        return false;
    if ((lower.y >= hi.y) || (upper.y <= lo.y))
        return false;
    if ((lower.z >= hi.z) || (upper.z <= lo.z))
        return false;

    return true;
}

bool COctree::intersect(COctree* other) const
{
    if ((lower.x >= other->upper.x) || (upper.x <= other->lower.x))
        return false;
    if ((lower.y >= other->upper.y) || (upper.y <= other->lower.y))
        return false;
    if ((lower.z >= other->upper.z) || (upper.z <= other->lower.z))
        return false;

    return true;
}

bool COctree::intersect( const CRay& ray, float t0, float t1 ) const
{
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin = (bounds(ray.sign[0]).x - ray.start.x) * ray.inv_direction.x;
    tmax = (bounds(1-ray.sign[0]).x - ray.start.x) * ray.inv_direction.x;
    tymin = (bounds(ray.sign[1]).y - ray.start.y) * ray.inv_direction.y;
    tymax = (bounds(1-ray.sign[1]).y - ray.start.y) * ray.inv_direction.y;
    if ( (tmin > tymax) || (tymin > tmax) )
        return false;
    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;
    tzmin = (bounds(ray.sign[2]).z - ray.start.z) * ray.inv_direction.z;
    tzmax = (bounds(1-ray.sign[2]).z - ray.start.z) * ray.inv_direction.z;
    if ( (tmin > tzmax) || (tzmin > tmax) )
        return false;
    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;
    return ( (tmin < t1) && (tmax > t0) );

}
//Draw Octree
void COctree::draw()
{	
    /*
 vec3f c = center();
 float l = lower.x; float r = upper.x;
 float b = lower.y; float t = upper.y;
 float n = lower.z; float f = upper.z;
 float lrov2 = (l+r)/2.0f;
 float btov2 = (b+t)/2.0f;
 float nfov2 = (n+f)/2.0f;

 GLfloat vertices [][3] = {{l, b, f}, {l, t, f}, {r, t, f},
         {r, b, f}, {l, b, n}, {l, t, n},
         {r, t, n}, {r, b, n}};

 //{{-1.0, -1.0, 1.0}, {-1.0, 1.0, 1.0}, {1.0, 1.0, 1.0},
 //{1.0, -1.0, 1.0}, {-1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0},
 //{1.0, 1.0, -1.0}, {1.0, -1.0, -1.0}};


 glPushAttrib(GL_ALL_ATTRIB_BITS);

 const float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
 glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);


 glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 //glFrontFace(GL_CCW);
 glBegin(GL_QUADS);
  glVertex3fv(vertices[0]); glVertex3fv(vertices[3]); glVertex3fv(vertices[2]); glVertex3fv(vertices[1]);
  glVertex3fv(vertices[4]); glVertex3fv(vertices[5]); glVertex3fv(vertices[6]); glVertex3fv(vertices[7]);
  glVertex3fv(vertices[3]); glVertex3fv(vertices[0]); glVertex3fv(vertices[4]); glVertex3fv(vertices[7]);
  glVertex3fv(vertices[1]); glVertex3fv(vertices[2]); glVertex3fv(vertices[6]); glVertex3fv(vertices[5]);
  glVertex3fv(vertices[2]); glVertex3fv(vertices[3]); glVertex3fv(vertices[7]); glVertex3fv(vertices[6]);
  glVertex3fv(vertices[5]); glVertex3fv(vertices[4]); glVertex3fv(vertices[0]); glVertex3fv(vertices[1]);
 glEnd();
 glPopAttrib();

 if(hasChildren())
 {
  for (int i=0; i<8; i++)
  {
   COctree* child = children[i];
   if(child)
   {
    child->draw();
   }
  }
 }
 */
}

vec3f COctree::getCorner(int index)
{
    vec3f mask(static_cast<float>((index & 4) >> 2),
               static_cast<float>((index & 2) >> 1),
               static_cast<float>(index & 1));
    return lower + upper*mask;

    /*
 float l = lower.x; float r = upper.x;
 float b = lower.y; float t = upper.y;
 float n = lower.z; float f = upper.z;

 vec3f corner;
 switch(index)
 {
 case LBN: corner = vec3f(l,b,n);
  break;
 case LBF: corner = vec3f(l,b,f);
  break;
 case LTN: corner = vec3f(l,t,n);
  break;
 case LTF: corner = vec3f(l,t,f);
  break;
 case RBN: corner = vec3f(r,b,n);
  break;
 case RBF: corner = vec3f(r,b,f);
  break;
 case RTN: corner = vec3f(r,t,n);
  break;
 case RTF: corner = vec3f(r,t,f);
  break;
 }
 return corner;
 */
}

void COctree::expand( vec3f v )
{
    upper += v;
    lower -= v;
}

void COctree::expand( vec3f lstPoints[], int ctPoints )
{
    for(int i=0;i<8; i++)
        children[i] = NULL;

    vec3f* lpArrPoints = new vec3f[ctPoints+2];
    for(int i=0; i<ctPoints; i++)
        lpArrPoints[i] = lstPoints[i];
    lpArrPoints[ctPoints] = lower;
    lpArrPoints[ctPoints+1] = upper;

    set(lpArrPoints, ctPoints+2);
    SAFE_DELETE_ARRAY(lpArrPoints);
}

void COctree::expand( float offset )
{
    upper += vec3f(offset, offset, offset);
    lower -= vec3f(offset, offset, offset);
}

void COctree::translate( vec3f displacement )
{
    lower += displacement;
    upper += displacement;
}

void COctree::scale( vec3f displacement )
{
    lower *= displacement;
    upper *= displacement;
}

void COctree::rotate(quat q)
{
    lower = q.transform(lower);
    upper = q.transform(upper);
}

void COctree::transform(const CMatrix& m)
{
    vec3f loT = m.transform(lower);
    vec3f hiT = m.transform(upper);
    this->lower = loT.vectorMin(hiT);
    this->upper = loT.vectorMax(hiT);
}

void COctree::csgUnion( const COctree& rhs )
{
    lower = lower.vectorMin(rhs.lower);
    upper = upper.vectorMax(rhs.upper);
}

void COctree::csgUnion( vec3f rhsLo, vec3f rhsHi )
{
    lower = lower.vectorMin(rhsLo);
    upper = upper.vectorMax(rhsHi);
}

void COctree::csgIntersection( const COctree& rhs )
{
    if((lower.x <= rhs.upper.x)&&(upper.x >= rhs.lower.x))
    {
        lower.x = MATHMAX(lower.x, rhs.lower.x);
        upper.x = MATHMIN(upper.x, rhs.upper.x);
    }

    if((lower.y <= rhs.upper.y)&&(upper.y >= rhs.lower.y))
    {
        lower.y = MATHMAX(lower.y, rhs.lower.y);
        upper.y = MATHMIN(upper.y, rhs.upper.y);
    }

    if((lower.z <= rhs.upper.z)&&(upper.z >= rhs.lower.z))
    {
        lower.z = MATHMAX(lower.z, rhs.lower.z);
        upper.z = MATHMIN(upper.z, rhs.upper.z);
    }
}

}
