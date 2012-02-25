#ifndef CFAST_QUADRIC_POINT_SET_H_
#define CFAST_QUADRIC_POINT_SET_H_

#include "PS_BlobTree/include/CSkeletonPrimitive.h"
#include "PS_BlobTree/include/CQuadricPoint.h"
#include <vector>

using namespace std;

#define FQPS_USE_DOUBLES 0

namespace PS {
namespace BLOBTREE{

class CFastQuadricPointSet : public CBlobNode
{
public:
    struct IPoint {
        IPoint() {}
        vec3f ptPosition;
        float fFieldScale;
        float fRadius;

#if FQPS_USE_DOUBLES
        double fRadiusSqr;
        double fCoeff1;
        double fCoeff2;
        double fCoeff3;

        IPoint( const vec3f & vPos, float fieldScale, float radius )
        { ptPosition = vPos; fFieldScale = fieldScale; fRadius = radius;
            fRadiusSqr = (double)radius*(double)radius;
            fCoeff1 = (double)fieldScale / (fRadiusSqr*fRadiusSqr);
            fCoeff2 = (-2.0 * (double)fieldScale) / fRadiusSqr;
            fCoeff3 = (double)fFieldScale;
        }
#else
        float fRadiusSqr;
        float fCoeff1;
        float fCoeff2;
        float fCoeff3;

        IPoint( const vec3f & vPos, float fieldScale, float radius )
        {
            ptPosition = vPos; fFieldScale = fieldScale; fRadius = radius;
            fRadiusSqr = radius*radius;
            fCoeff1 = fieldScale / (radius*radius*radius*radius);
            fCoeff2 = (-2.0f * fieldScale) / (radius*radius);
            fCoeff3 = fFieldScale;
        }
#endif
    };

    CFastQuadricPointSet()
    {
        m_bUsePerPointColors = false;
        m_bValidOctree = false;
    }

    CFastQuadricPointSet(CBlobNode* child)
    {
        this->addChild(child);
    }

    CFastQuadricPointSet( bool bUsePerPointColors)
        : m_bUsePerPointColors(bUsePerPointColors)
    {
        if (!m_bUsePerPointColors)
        {
            m_vColors.push_back( vec3f(1.0, 0.0, 0.0) );
        }
        m_bValidOctree = false;
    }

    ~CFastQuadricPointSet()
    {
        cleanup();
    }

    void cleanup(){
        m_vPoints.resize(0);
        m_vColors.resize(0);
    }

    //Prepares internal data structures for fast computation
    void prepare();

    //Compute FieldValue
    float fieldValue(vec3f p);

    //Compute Field and Gradient
    int fieldValueAndGradient(vec3f p, float delta, vec3f &outGradient, float &outField);

    //Octree
    COctree computeOctree();

    //NodeType
    BlobNodeType getNodeType() {return bntOpFastQuadricPointSet;}

    std::string getName()
    {
        return "FASTQUADRATICPOINTSET";
    }

    bool isOperator()
    {
        return true;
    }

    bool isPerPointColors() const {return m_bUsePerPointColors;}
private:
    bool getPointFieldBox(U32 idxPoint, COctree& dest );

    unsigned int getPointCount() const
    {
        return (unsigned int)m_vPoints.size();
    }

    void addPoint( const IPoint & point,
                   const vec3f & color)
    {
        m_vPoints.push_back(point);
        if (m_bUsePerPointColors)
            m_vColors.push_back(color);
        m_bValidOctree = false;
    }

    void addPoint( CQuadricPoint & point,
                   const vec3f & color)
    {
        IPoint pt(point.getPosition(), point.getFieldScale(), point.getFieldRadius());
        m_vPoints.push_back(pt);
        if (m_bUsePerPointColors)
            m_vColors.push_back(color);
        m_bValidOctree = false;
    }

    void clearPoints()
    {
        m_vPoints.resize(0);
        if (m_bUsePerPointColors)
            m_vColors.resize(0);
        m_bValidOctree = false;
    }

    const IPoint & getPoint( unsigned int nPoint ) const
    {
        //ASSERT(nPoint < GetPointCount());
        return m_vPoints[nPoint];
    }

    IPoint & getPoint( unsigned int nPoint )
    {
        //ASSERT(nPoint < GetPointCount());
        return m_vPoints[nPoint];
    }

    const vec3f & getPointColor( unsigned int nPoint ) const
    {
        //ASSERT(nPoint < GetPointCount());
        if (m_bUsePerPointColors) {
            return m_vColors[nPoint];
        } else {
            return m_vColors[0];
        }
    }

    vec3f & getPointColor( unsigned int nPoint )
    {
        //ASSERT(nPoint < GetPointCount());
        if (m_bUsePerPointColors) {
            return m_vColors[nPoint];
        } else {
            return m_vColors[0];
        }
    }



protected:
    vector<IPoint> m_vPoints;
    vector<vec3f> m_vColors;

    bool m_bUsePerPointColors;
    bool m_bValidOctree;
    //Wml::AxisAlignedBox3f m_fieldBoundsCache;
    //const Wml::AxisAlignedBox3f & GetCachedFieldBounds();
};



}   // namespace PS
}	//namespace BLOBTREE


#endif  //_RMSIMPLICIT_FAST_QUADRIC_POINT_SET_H_
