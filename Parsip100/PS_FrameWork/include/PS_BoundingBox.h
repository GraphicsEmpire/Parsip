#pragma once
#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "PS_Vector.h"
#include "mathHelper.h"

using namespace PS::MATH;

namespace PS{
class BBOX
{
public:
    enum BOXSIDE {sideX, sideY, sideZ};

    BBOX() { init();}
    BBOX(const vec3f& lo, const vec3f& up):lower(lo), upper(up)
    { }

    BBOX(const BBOX& rhs):lower(rhs.lower), upper(rhs.upper)
    { }

    void expand(const vec3f& e)
    {
        lower -= e;
        upper += e;
    }

    float getMinSide() const
    {
        vec3f sides = upper - lower;
        return MATHMIN(MATHMIN(sides.x, sides.y), sides.z);
    }

    float getMaxSide() const
    {
        vec3f sides = upper - lower;
        return MATHMAX(MATHMAX(sides.x, sides.y), sides.z);
    }

    vec3f middle() const
    {
        return (lower + upper) * 0.5f;
    }

    void init()
    {
        lower = vec3f(FLT_MAX, FLT_MAX, FLT_MAX);
        upper = vec3f(FLT_MIN, FLT_MIN, FLT_MIN);
    }

    bool isValid() const
    {
        if((lower.x <= upper.x)&&
           (lower.y <= upper.y)&&
           (lower.z <= upper.z))
            return true;
        return false;
    }

    BBOX operator=(const BBOX& other)
    {
        this->lower = other.lower;
        this->upper = other.upper;
        return *this;
    }

public:
    vec3f lower;
    vec3f upper;

};

}

#endif
