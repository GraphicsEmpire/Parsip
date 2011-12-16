#ifndef CVOLUMESPHERE_H
#define CVOLUMESPHERE_H

#include "CVolume.h"
#include "CVolumeBox.h"

namespace PS{
namespace BLOBTREE{
namespace Vol{
class  CVolumeSphere: public CVolume
{
private:
    float m_radius;
    vec3f m_center;

public:
    CVolumeSphere() {;}
    CVolumeSphere(CVolume* vol);
    CVolumeSphere(vec3f center, float radius);

    void set(vec3f center, float radius);

    float radius() const {return m_radius;}
    float radius2() const {return m_radius * m_radius;}
    void setRadius(float radius);

    static CVolume* emptyVolume();


    vec3f center() const {return m_center;}
    void setCenter(vec3f center);

    float size();

    bool isBox();

    bool isEmpty();

    float distance(vec3f v);

    bool isInside(vec3f pt);

    bool intersect(CVolume* bv);


    void tranform(const CMatrix& m);

    CVolume* scale(vec3f s);

    CVolume* translate(vec3f trans);

    CVolume* rotate(quat rot);

    void scaleSelf(vec3f scaleVector);
    void translateSelf(vec3f translateVector);



    CVolume* UnionSphere(const CVolume* sphere);
    CVolume* UnionBox(const CVolume* box);
    CVolume* Union(const CVolume* bv);

    CVolume* intersectionSphere(const CVolume* sphere);
    CVolume* intersectionBox(const CVolume* box);
    CVolume* intersection(const CVolume* bv);

    void operator =(const CVolume* other);

};
}
}
}

#endif
