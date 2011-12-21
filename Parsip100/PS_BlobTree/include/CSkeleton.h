#pragma once
#ifndef CSKELETON_H
#define CSKELETON_H

#include <math.h>

#include "PS_FrameWork/include/mathHelper.h"
#include "PS_FrameWork/include/PS_Vector.h"
#include "PS_FrameWork/include/PS_GeometryFuncs.h"
#include "PS_FrameWork/include/PS_Octree.h"
#include "PS_FrameWork/include/PS_String.h"
#include "PS_FrameWork/include/PS_SketchConfig.h"

#include "BlobTreeScripting.h"
#include "CVolumeSphere.h"
#include "_constSettings.h"

using namespace PS;

namespace PS{
namespace BLOBTREE{

using namespace PS::MATH;

//Implementation of Skeleton types like point, line, disk, circle, cylinder, hollowCylinder
class  CSkeleton
{
public:
    CSkeleton() {}

    virtual VOL::CVolume* getBoundingVolume(float range) = 0;
    virtual bool getExtremes(vec3f& lower, vec3f& upper) = 0;

    //=========================================================
    vec3f gradient(vec3f p, float delta)
    {
        float f = distance(p);

        float fx = distance(vec3f(p.x - delta, p.y, p.z));
        float fy = distance(vec3f(p.x, p.y - delta, p.z));
        float fz = distance(vec3f(p.x, p.y, p.z - delta));

        vec3f n;
        n.set(f-fx, f-fy, f-fz);
        return n;
    }

    virtual float distance(vec3f p) = 0;

    virtual float squareDistance(vec3f p) = 0;

    virtual vec3f normal(vec3f p) = 0;
    virtual float getDistanceAndNormal(vec3f p, vec3f& normal) = 0;
    //=========================================================
    virtual string getName() = 0;

    //Computes Centroid useful for computation of bounding volume
    virtual vec3f getPolySeedPoint() = 0;
    virtual void translate(vec3f d) = 0;
    virtual BlobNodeType getType() = 0;

    virtual bool saveScript(CSketchConfig* lpSketchScript, int id) = 0;
    virtual bool loadScript(CSketchConfig* lpSketchScript, int id) = 0;
};

}
}
#endif
