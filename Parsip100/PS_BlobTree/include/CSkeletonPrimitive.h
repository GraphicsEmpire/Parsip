#ifndef CSKELETONPRIMITIVE_H
#define CSKELETONPRIMITIVE_H

#include <math.h>

#include "CSkeletonCylinder.h"
#include "CSkeletonDisc.h"
#include "CSkeletonLine.h"
#include "CSkeletonPoint.h"
#include "CSkeletonRing.h"
#include "CSkeletonCube.h"
#include "CSkeletonTriangle.h"

#include "CBlobTree.h"
#include "CFieldFunction.h"
#include "CRootFinder.h"
#include "PS_FrameWork/include/_dataTypes.h"
#include "PS_FrameWork/include/PS_Octree.h"


namespace PS{
namespace BLOBTREE{

class  CSkeletonPrimitive : public CBlobNode
{
private:
    //Type of field function to evaluate field with
    CFieldFunction* m_fieldFunction;

    //Pointer to skeleton structure of the shape
    CSkeleton* m_skeleton;

    //Range of influence
    float m_range;

    //iso-distance to skeleton
    float m_isoDistance;

    //Scale params
    float m_scale;
    float m_scale1;
    float m_scale2;

    //Octree Set?
    bool m_bOctreeValid;
public:
    CSkeletonPrimitive()
    {
        m_skeleton = NULL;
        m_fieldFunction = new CFunctionWyvill();
        init(1.0f);
    }

    CSkeletonPrimitive(const CSkeleton * skelet)
    {
        m_skeleton = const_cast<CSkeleton*>(skelet);
        m_fieldFunction = new CFunctionWyvill();
        init(1.0f);
    }

    CSkeletonPrimitive(const CSkeleton * skelet, FieldFunctionType fieldFunctionType, float scale = 1.0f)
    {
        m_skeleton = const_cast<CSkeleton*>(skelet);
        switch(fieldFunctionType)
        {
        case(fftWyvill):
            m_fieldFunction = new CFunctionWyvill();
            break;
        case(fftSoftObjects):
            m_fieldFunction = new CFunctionSoftObject();
            break;
        default:
            m_fieldFunction = new CFunctionWyvill();
            break;
        }

        init(scale);
    }

    ~CSkeletonPrimitive()
    {
        SAFE_DELETE(m_fieldFunction);
        SAFE_DELETE(m_skeleton);
    }

    float getRange() const {return m_range;}
    float getIsoDistance() const {return m_isoDistance;}
    float getScale() const {return m_scale;}
    float getScaleInv1() const {return m_scale1;}
    float getScaleInv2() const {return m_scale2;}


    //Update internal parameters
    void init(float scale)
    {
        //Set scaling Params
        m_scale = scale;
        m_scale1 = 1 / scale;
        m_scale2 = m_scale1 * m_scale1;

        //Set Range and IsoDistance
        m_isoDistance = m_fieldFunction->inverse(ISO_VALUE) * m_scale;
        m_range = m_fieldFunction->getRange() * m_scale;
        m_bOctreeValid = false;
    }
    //////////////////////////////////////////////////////////////////////////
    void setFieldFunction(const CFieldFunction * field)
    {
        m_fieldFunction = const_cast<CFieldFunction*>(field);
    }

    void setRange(float range)
    {
        m_range = range;
    }

    void setIsoDistance(float dist)
    {
        m_isoDistance = dist;
    }

    void setFieldScale(float scale)
    {
        m_scale = scale;
    }

    CSkeleton* getSkeleton()
    {
        return m_skeleton;
    }

    CFieldFunction* getFieldFunction() const { return m_fieldFunction;}


    bool isOperator() { return false;}

    float fieldValue(vec3f p)
    {
        p = m_transform.applyBackwardTransform(p);
        float dist = m_skeleton->squareDistance(p) * m_scale2;
        return ComputeWyvillFieldValueSquare(dist);
    }

    float curvature(vec3f p)
    {
        //Just to use p and remove warnings
        p.normalize();
        return 1.0f*p.length() / m_isoDistance;
    }

    CMaterial getMaterial()
    {
        return m_material;
    }

    string getName()
    {
        if(m_skeleton != NULL)
            return m_skeleton->getName();
        else
            return "PRIMITIVE";

    }

    //Finds a seed point with field-value greater than the specified iso-value if asked for hot
    //Using the gradient of the field
    bool findSeedPoint(bool bFindHot, float iso_value, vec3f& p, float& fp);

    //Finds a seed point with field-value greater than the specified iso-value if asked for hot
    //By searching
    bool findSeedPoint(bool bFindHot, float iso_value, float search_step, vec3f search_dir, vec3f& p, float& fp);

    COctree computeOctree();

    //This function will compute a very tight bounding volume for the primitive to be used
    //In polygonization algorithm

    BlobNodeType getNodeType()
    {
        if(m_skeleton)
            return m_skeleton->getType();
        else
            return bntPrimSkeleton;
    }

private:
    vec3f getPolySeedPoint()
    {
        vec3f c = m_skeleton->getPolySeedPoint();
        c = m_transform.applyForwardTransform(c);
        return c;
    }    
};

}
}

#endif
