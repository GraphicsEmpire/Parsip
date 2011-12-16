// Ryan Schmidt   rms@unknownroad.com
// Copyright (c) 2007. All Rights Reserved
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This source code is provided for non-commercial,
// academic use only. It may not be disclosed or distributed,
// in part or in whole, without the express written consent
// of the Copyright Holder (Ryan Schmidt). This copyright notice must
// not be removed from any original or modified source files, and 
// must be included in any source files which contain portions of
// the original source code.


#ifndef _RMSIMPLICIT_FAST_QUADRIC_POINT_SET_H_
#define _RMSIMPLICIT_FAST_QUADRIC_POINT_SET_H_

#include "PS_BlobTree/include/CSkeletonPrimitive.h"
#include "PS_BlobTree/include/CQuadricPoint.h"
#include <vector>

using namespace std;

//#include <WmlVector3.h>
//#include <WmlMatrix3.h>
//#include <Frame.h>


#define FQPS_USE_DOUBLES 0

namespace PS {
namespace BLOBTREE{

class FastQuadricPointSet : public CSkeletonPrimitive
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
        { ptPosition = vPos; fFieldScale = fieldScale; fRadius = radius;
            fRadiusSqr = radius*radius;
            fCoeff1 = fieldScale / (radius*radius*radius*radius);
            fCoeff2 = (-2.0f * fieldScale) / (radius*radius);
            fCoeff3 = fFieldScale;
        }
#endif
    };

    FastQuadricPointSet( bool bUsePerPointColors = false )
        : m_bUsePerPointColors(bUsePerPointColors)
    {
        if (!m_bUsePerPointColors)
        {
            m_vColors.push_back( vec3f(1.0, 0.0, 0.0) );
        }
        m_bValidOctree = false;
    }

    ~FastQuadricPointSet()
    {
        m_vPoints.clear();
        m_vColors.clear();
    }

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

    //void ApplyFrame( const Wml::Frame & frame );
    //void ApplyFrameInverse( const Wml::Frame & frame );

    // must be relative!
    void translate( const vec3f & bTranslate );

    void getPointFieldBox( unsigned int nPoint, COctree& dest );

    virtual void getSeedPoints( std::vector<vec3f> & seedPoints );

    /*
 * ScalarField interface
 */
    float fieldValue(vec3f p);

    int fieldValueAndGradient(vec3f p, float delta, vec3f &outGradient, float &outField);

    COctree computeOctree();

    BlobNodeType getNodeType() {return bntPrimFastQuadraticPointSet;}

    string getName()
    {
        return "FASTQUADRATICPOINTSET";
    }

    bool isOperator()
    {
        return false;
    }

    vec3f getPolySeedPoint()
    {
        vec3f c;
        if(m_vPoints.size() > 0)
            c = m_vPoints[0].ptPosition;
        c = m_transform.applyForwardTransform(c);
        return c;

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
