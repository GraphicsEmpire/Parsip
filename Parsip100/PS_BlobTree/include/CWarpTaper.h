#ifndef CTAPER_H
#define CTAPER_H

#include <math.h>
#include "CBlobTree.h"


namespace PS{
namespace BLOBTREE{

//***********************************************************
//Taper Warping
class  CWarpTaper : public CBlobNode
{
private:
    float m_warpFactor;
    MajorAxices m_axisAlong;
    MajorAxices m_axisTaper;

public:
    CWarpTaper()
    {
        m_warpFactor = DEFAULT_WARP_FACTOR;
        m_axisTaper = xAxis;
        m_axisAlong = zAxis;
    }

    CWarpTaper(CBlobNode * child)
    {
        addChild(child);
        m_warpFactor = DEFAULT_WARP_FACTOR;
        m_axisTaper = xAxis;
        m_axisAlong = zAxis;
    }

    CWarpTaper(CBlobNode * child, float factor)
    {
        addChild(child);
        m_warpFactor = factor;
        m_axisTaper = xAxis;
        m_axisAlong = zAxis;
    }

    CWarpTaper(CBlobNode * child, float factor, MajorAxices taper)
    {
        addChild(child);
        m_warpFactor = factor;
        m_axisTaper = taper;
        m_axisAlong = zAxis;
    }

    CWarpTaper(CBlobNode * child, float factor, MajorAxices taper, MajorAxices along)
    {
        addChild(child);
        m_warpFactor = factor;
        m_axisTaper = taper;
        m_axisAlong = along;
    }

    CWarpTaper(float factor, MajorAxices taper, MajorAxices along)
    {
        m_warpFactor = factor;
        m_axisTaper = taper;
        m_axisAlong = along;
    }

    void setParamFrom(CBlobNode* input)
    {
        CWarpTaper * taperN = dynamic_cast<CWarpTaper*>(input);
        this->m_warpFactor = taperN->m_warpFactor;
        this->m_axisTaper = taperN->m_axisTaper;
        this->m_axisAlong = taperN->m_axisAlong;
    }

    MajorAxices getAxisAlong() const
    {
        return m_axisAlong;
    }

    void setAxisAlong(MajorAxices axis)
    {
        m_axisAlong = axis;
    }

    MajorAxices getAxisTaper() const
    {
        return m_axisTaper;
    }

    void setAxisTaper(MajorAxices axis)
    {
        m_axisTaper = axis;
    }


    float getWarpFactor() {return m_warpFactor;}
    void setWarpFactor(float factor) { m_warpFactor = factor;}

    vec3f taperAlongX(vec3f p)
    {
        vec3f result;
        switch(m_axisTaper)
        {
        case(yAxis):
            result = vec3f(p.x, p.y * (1 + p.x * m_warpFactor), p.z);
            break;
        case(zAxis):
            result = vec3f(p.x, p.y, p.z * (1 + p.x * m_warpFactor));
            break;
        default:
            result = vec3f(p.x, p.y * (1 + p.x * m_warpFactor), p.z);
            break;
        }

        return result;
    }

    vec3f taperAlongY(vec3f p)
    {
        vec3f result;
        switch(m_axisTaper)
        {
        case(xAxis):
            result = vec3f(p.x * (1 + p.y * m_warpFactor), p.y, p.z);
            break;
        case(zAxis):
            result = vec3f(p.x, p.y, p.z * (1 + p.y * m_warpFactor));
            break;
        default:
            result = vec3f(p.x * (1 + p.y * m_warpFactor), p.y, p.z);
            break;
        }

        return result;
    }

    vec3f taperAlongZ(vec3f p)
    {
        vec3f result;
        switch(m_axisTaper)
        {
        case(xAxis):
            result = vec3f(p.x * (1 + p.z * m_warpFactor), p.y, p.z);
            break;
        case(zAxis):
            result = vec3f(p.x, p.y * (1 + p.z * m_warpFactor), p.z);
            break;
        default:
            result = vec3f(p.x * (1 + p.z * m_warpFactor), p.y, p.z);
            break;
        }

        return result;
    }

    vec3f warp(vec3f p)
    {
        vec3f result;
        switch(m_axisAlong)
        {
        case(xAxis):
            result = taperAlongX(p);
            break;
        case(yAxis):
            result = taperAlongY(p);
            break;
        case(zAxis):
            result = taperAlongZ(p);
        }
        return result;
    }

    float fieldValue(vec3f p)
    {
        if(m_children[0]->isOperator())
        {
            //Goto Warp Space Directly!
            p = warp(p);
            return m_children[0]->fieldValue(p);
        }
        else
        {
            CSkeletonPrimitive* sprim = reinterpret_cast<CSkeletonPrimitive*>(m_children[0]);
            //Goto Normal Space
            p = sprim->getTransform().applyBackwardTransform(p);

            //Goto Warp Space
            p = warp(p);
            float dd = sprim->getSkeleton()->squareDistance(p) * sprim->getScaleInv2();
            return sprim->getFieldFunction()->fieldValueSquare(dd);
        }
    }

    float curvature(vec3f p)
    {
        p = warp(p);
        float result = m_children[0]->curvature(p);
        return result;
    }

    string getName()
    {
        return "TAPER";
    }

    COctree computeOctree()
    {
        m_octree = m_children[0]->getOctree();
        m_octree.expand(BOUNDING_OCTREE_EXPANSION_FACTOR);
        return m_octree;
    }

    bool isOperator() { return true;}
    BlobNodeType getNodeType() {return bntOpWarpTaper;}

    bool saveScript(CSketchConfig* lpSketchScript)
    {
        bool bres = saveGenericInfoScript(lpSketchScript);

        //Write parameters for RicciBlend
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", this->getID());
        lpSketchScript->writeFloat(strNodeName, "factor", this->getWarpFactor());
        lpSketchScript->writeInt(strNodeName, "base axis", static_cast<int>(this->getAxisAlong()));
        lpSketchScript->writeInt(strNodeName, "taper axis", static_cast<int>(this->getAxisTaper()));
        return bres;
    }


};

}
}
#endif

