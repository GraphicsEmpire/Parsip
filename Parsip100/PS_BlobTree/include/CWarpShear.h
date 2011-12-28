#ifndef CSHEAR_H
#define CSHEAR_H

#include <math.h>
#include "CBlobTree.h"

#define DEFAULT_SHEAR_FACTOR 1.2f

namespace PS{
namespace BLOBTREE{

//***********************************************************
//Taper Warping
class  CWarpShear : public CBlobNode
{
private:	
    MajorAxices m_axisAlong;
    MajorAxices m_axisDependant;
    float m_warpFactor;

public:
    CWarpShear() {;}
    CWarpShear(CBlobNode * child)
    {
        addChild(child);
        m_warpFactor = DEFAULT_SHEAR_FACTOR;
        m_axisAlong = xAxis;
        m_axisDependant = zAxis;
    }

    CWarpShear(CBlobNode * child, float factor)
    {
        addChild(child);
        m_warpFactor = factor;
        m_axisAlong = xAxis;
        m_axisDependant = zAxis;
    }

    CWarpShear(CBlobNode * child, float factor, MajorAxices axisAlong, MajorAxices axisDependant)
    {
        addChild(child);
        m_warpFactor = factor;
        m_axisAlong = axisAlong;
        m_axisDependant = axisDependant;
    }

    CWarpShear(float factor, MajorAxices axisAlong, MajorAxices axisDependant)
    {
        m_warpFactor = factor;
        m_axisAlong = axisAlong;
        m_axisDependant = axisDependant;
    }

    void setParamFrom(CBlobNode* input)
    {
        CWarpShear* shearN = dynamic_cast<CWarpShear*>(input);
        this->m_axisAlong = shearN->m_axisAlong;
        this->m_axisDependant = shearN->m_axisDependant;
        this->m_warpFactor = shearN->m_warpFactor;
    }

    MajorAxices getAxisAlong()
    {
        return m_axisAlong;
    }

    MajorAxices getAxisDependent()
    {
        return m_axisDependant;
    }

    float getWarpFactor() {return m_warpFactor;}
    void setWarpFactor(float factor) { m_warpFactor = factor;}

    vec3f warpAlongX(vec3f p)
    {
        vec3f result;
        switch(m_axisDependant)
        {
        case(yAxis):
            return result = vec3f(p.x + m_warpFactor * p.y, p.y, p.z);
            break;
        case(zAxis):
            return result = vec3f(p.x + m_warpFactor * p.z, p.y, p.z);
            break;
        default:
            return result = vec3f(p.x + m_warpFactor * p.y, p.y, p.z);
            break;
        }
        return result;
    }

    vec3f warpAlongY(vec3f p)
    {
        vec3f result;
        switch(m_axisDependant)
        {
        case(xAxis):
            return result = vec3f(p.x, p.y + m_warpFactor * p.x, p.z);
            break;
        case(zAxis):
            return result = vec3f(p.x, p.y + m_warpFactor * p.z, p.z);
            break;
        default:
            return result = vec3f(p.x, p.y + m_warpFactor * p.x, p.z);
            break;
        }
        return result;
    }

    vec3f warpAlongZ(vec3f p)
    {
        vec3f result;
        switch(m_axisDependant)
        {
        case(xAxis):
            return result = vec3f(p.x, p.y, p.z + m_warpFactor * p.x);
            break;
        case(yAxis):
            return result = vec3f(p.x, p.y, p.z + m_warpFactor * p.y);
            break;
        default:
            return result = vec3f(p.x, p.y, p.z + m_warpFactor * p.x);
            break;
        }
        return result;
    }

    vec3f warp(vec3f p)
    {
        switch(m_axisAlong)
        {
        case(xAxis):
            return warpAlongX(p);
        case(yAxis):
            return warpAlongY(p);
        case(zAxis):
            return warpAlongZ(p);
        default:
            return warpAlongX(p);
        }
    }

    float fieldValue(vec3f p)
    {
        p = warp(p);
        float result = m_children[0]->fieldValue(p);
        return result;
    }

    float curvature(vec3f p)
    {
        p = warp(p);
        return m_children[0]->curvature(p);
    }

    string getName()
    {
        return "SHEAR";
    }

    COctree computeOctree()
    {
        m_octree = m_children[0]->getOctree();
        m_octree.expand(BOUNDING_OCTREE_EXPANSION_FACTOR);
        return m_octree;
    }

    bool isOperator() { return true;}
    BlobNodeType getNodeType() {return bntOpWarpShear;}

    bool saveScript(CSketchConfig* lpSketchScript)
    {
        bool bres = saveGenericInfoScript(lpSketchScript);

        //Write parameters for RicciBlend
        DAnsiStr strNodeName = printToAStr("BLOBNODE %d", this->getID());
        lpSketchScript->writeFloat(strNodeName, "factor", this->getWarpFactor());
        lpSketchScript->writeInt(strNodeName, "base axis", static_cast<int>(this->getAxisAlong()));
        lpSketchScript->writeInt(strNodeName, "shear axis", static_cast<int>(this->getAxisDependent()));
        return bres;
    }
};

}
}
#endif

