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


#include "GalinMedusaGenerator.h"
#include "FastQuadricPointSet.h"
#include "PS_FrameWork/include/_parsDebug.h"

using namespace PS;

using namespace PS::BLOBTREE;


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*!

\brief Creates an Hermite cubic polynomial on interval [0,1] given

function values and derivatives.

\param a, b function values for t=0 and t=1.

\param ta, tb function derivative values for t=0 and t=1.

*/

class Polynomial
{
public:
    Polynomial() { memset(m_params,0,sizeof(float)*4); }

    Polynomial( double a, double b, double c, double d ){
        m_params[0] = a;
        m_params[1] = b;
        m_params[2] = c;
        m_params[3] = d;
    }

    static Polynomial Hermite( const double& a,const double& b,const double& ta,const double& tb )
    {
        return Polynomial( tb+ta+2.0*(a-b), -tb-2.0*ta+3.0*(b-a), ta, a );
    }

    double operator () ( double t )
    {
        return t*t*t*m_params[0] + t*t*m_params[1] + t*m_params[2] + m_params[3];
    }

    double m_params[4];
};



class SplineGenerator
{
public:
    enum Type {
        NestedSumBlendsOfQuadricPoints,
        OneSumBlend,
        SumBlendOfFastQuadricPointSets,
        OneFastQuadricPointSet
    };


    SplineGenerator( Type eType )
    {
        m_eType = eType;

        m_pTopLevelPointSet = NULL;
        m_pTopLevelBlend = new CBlend();

        if (m_eType == OneFastQuadricPointSet)
        {
            m_pTopLevelPointSet = new FastQuadricPointSet();
            m_pTopLevelBlend->addChild(m_pTopLevelPointSet);
        }

        m_nPointPrimitives = 0;
    }

    ~SplineGenerator()
    {
        //SAFE_DELETE(m_pTopLevelBlend);
        //SAFE_DELETE(m_pTopLevelPointSet);
    };

    void addSpline( vec3d a, vec3d na, double ra, double fa,
                    vec3d b, vec3d nb, double rb, double fb,
                    double dFieldScale, int n )
    {
        Polynomial px = Polynomial::Hermite(a[0],b[0],na[0],nb[0]);
        Polynomial py = Polynomial::Hermite(a[1],b[1],na[1],nb[1]);
        Polynomial pz = Polynomial::Hermite(a[2],b[2],na[2],nb[2]);
        Polynomial r = Polynomial::Hermite(ra,rb,fa,fb);

        CBlend * pNestedBlend = (m_eType == NestedSumBlendsOfQuadricPoints) ? new CBlend() : NULL;
        FastQuadricPointSet * pPointSet = (m_eType == SumBlendOfFastQuadricPointSets) ? new FastQuadricPointSet() : NULL;

        for (int i = 0; i < n; ++i)
        {
            double t = (double)i / (double)(n-1);

            CQuadricPoint * pPoint = new CQuadricPoint(vec3f( (float)px(t), (float)py(t), (float)pz(t)), (float)r(t), (float)dFieldScale );
            ++m_nPointPrimitives;

            if (m_eType == NestedSumBlendsOfQuadricPoints)
            {
                pNestedBlend->addChild(pPoint);
            }
            else if ( m_eType == OneSumBlend )
            {
                m_pTopLevelBlend->addChild(pPoint);

            }
            else if (m_eType == SumBlendOfFastQuadricPointSets)
            {
                pPointSet->addPoint( *pPoint, vec3f(1.0f, 0.0f, 0.0f));
                SAFE_DELETE(pPoint);
            }
            else if (m_eType == OneFastQuadricPointSet)
            {
                m_pTopLevelPointSet->addPoint( *pPoint, vec3f(1.0f, 0.0f, 0.0f) );
                SAFE_DELETE(pPoint);
            }
            else
                abort();
        }

        if ( m_eType == NestedSumBlendsOfQuadricPoints)
            m_pTopLevelBlend->addChild( pNestedBlend );
        else if (m_eType == SumBlendOfFastQuadricPointSets)
            m_pTopLevelBlend->addChild( pPointSet );
    }

    CBlend * getField() { return m_pTopLevelBlend; }

    unsigned int getNumPointPrimitives() { return m_nPointPrimitives; }

protected:
    Type m_eType;

    CBlend * m_pTopLevelBlend;
    FastQuadricPointSet * m_pTopLevelPointSet;
    unsigned int m_nPointPrimitives;

};



/*!
\brief Creates the neck and shoulders.
*/
CBlend * GalinMedusaGenerator::Medusa_Neck()
{
    SplineGenerator g( SplineGenerator::SumBlendOfFastQuadricPointSets );
    //SplineGenerator g( SplineGenerator::OneSumBlend );

    g.addSpline(vec3d(0,0.0,2.9), vec3d(0,0,1),   0.3, -0.20,
                vec3d(0,-0.05,3.7), vec3d(0,0.1,1), 0.24, 0.0,
                0.75, 20 );

    parsDebugInfo("Mudusa Neck: %d primitives\n", g.getNumPointPrimitives() );

    return g.getField();
}



/*!
\brief Creates the lower hips and the tail. Lower hips merge two spline curves,
whereas the tail itself consists in three cubic splines.
*/

CBlend * GalinMedusaGenerator::Medusa_Tail()
{
    SplineGenerator g( SplineGenerator::SumBlendOfFastQuadricPointSets );

    // Hips
    g.addSpline(
                vec3d(-0.17,0,1.1),vec3d(-1,0,1),0.5,0.0,
                vec3d(-0.13,0,1.95),vec3d(0,0,1.5),0.35,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.17,0,1.1),vec3d(1,0,1),0.5,0.0,
                vec3d(0.13,0,1.95),vec3d(0,0,1.5),0.35,0.0,
                0.75,30);

    // fesses
    g.addSpline(
                vec3d(-0.23,-0.1,1.19),vec3d(0,1,0.5),0.55,0.0,
                vec3d(-0.22,0.0,1.38),vec3d(0,-0.5,1.0),0.35,0.0,
                0.75,10);
    g.addSpline(
                vec3d(0.23,-0.1,1.19),vec3d(0,1,0.5),0.55,0.0,
                vec3d(0.22,0.0,1.38),vec3d(0,-0.5,1.0),0.35,0.0,
                0.75,10);

    // Lower belly
    g.addSpline(
                vec3d(-0.28,0.0,1.15),vec3d(0,0,0),0.5,0.075,
                vec3d(-0.15,-0.4,0.25),vec3d(0,0,0),0.4,0.05,
                0.75,20);
    g.addSpline(
                vec3d(0.28,0.0,1.15),vec3d(0,0,0),0.5,0.075,
                vec3d(0.15,-0.4,0.25),vec3d(0,0,0),0.4,0.05,
                0.75,20);

    // Tail
    g.addSpline(
                vec3d(0.0,-0.4,0.1),vec3d(0,0,0),0.45,0.075,
                vec3d(0.0,0.4,0.15),vec3d(1,1,0),0.5,0.05,
                0.75,200);
    g.addSpline(
                vec3d(0.0,0.4,0.15),vec3d(1,1,0),0.55,0.075,
                vec3d(1.5,0.0,0.3),vec3d(1,-3,2.0),0.5,0.05,
                0.75,20);
    g.addSpline(
                vec3d(1.5,0.0,0.3),vec3d(1,-3.0,2.0),0.5,0.075,
                vec3d(-1.5,0.0,1.5),vec3d(0,4.0,-0.5),0.4,0.075,
                0.75,30);
    g.addSpline(
                vec3d(-1.5,0.01,1.5),vec3d(0,4.0,-0.5),0.36,0.075,
                vec3d(0.7,0.8,2.5),vec3d(2,-1.7,1.0),0.237,0.075,
                0.75,400);
    g.addSpline(
                vec3d(0.7,0.8,2.5),vec3d(2,-1.7,1.0),0.223,0.075,
                vec3d(0.4,-0.7,2.3),vec3d(-2,0,-1.0),0.1,0.075,
                0.75,40);

    parsDebugInfo("Mudusa Tail: %d primitives\n", g.getNumPointPrimitives() );

    return g.getField();
}




/*
\brief Create the left Hand.
*/

CBlend * GalinMedusaGenerator::Medusa_LeftHand()
{
    SplineGenerator g( SplineGenerator::SumBlendOfFastQuadricPointSets );

    // dessus

    g.addSpline(
                vec3d(0.70 -0.2,-0.92 +0.5,2.29 -0.55),vec3d(0,0,0.05),0.09 ,0.0,
                vec3d(0.57 -0.2,-0.94 +0.5,2.44 -0.55),vec3d(0,0,-0.05),0.047,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.70 -0.2,-0.94 +0.5,2.31 -0.55),vec3d(0,-0.1,0.1),0.078 ,0.0,
                vec3d(0.53 -0.2,-1.13 +0.5,2.40 -0.55),vec3d(0,0.1,-0.05),0.047,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.72 -0.2,-0.96 +0.5,2.29 -0.55),vec3d(0,-0.1,0),0.06 ,0.0,
                vec3d(0.52 -0.2,-1.11 +0.5,2.33 -0.55),vec3d(0,0.1,0),0.047,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.72 -0.2,-0.96 +0.5,2.26 -0.55),vec3d(0,-0.1,0),0.06 ,0.0,
                vec3d(0.53 -0.2,-1.09 +0.5,2.25 -0.55),vec3d(0,0.1,0),0.047,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.68 -0.2,-0.95 +0.5,2.23 -0.55),vec3d(0,-0.1,-0.1),0.078 ,0.0,
                vec3d(0.54 -0.2,-1.07 +0.5,2.19 -0.55),vec3d(0,0.1,0.05),0.047,0.0,
                0.75,30);

    // 1eres phalanges

    g.addSpline(
                vec3d(0.57 -0.2,-0.94 +0.5,2.44 -0.55),vec3d(0,0,0),0.047 ,0.0,
                vec3d(0.48 -0.2,-0.94 +0.5,2.44 -0.55),vec3d(0,0,0),0.047,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.53 -0.2,-1.13 +0.5,2.40 -0.55),vec3d(0,-0.05,0),0.047 ,0.0,
                vec3d(0.42 -0.2,-1.05 +0.5,2.38 -0.55),vec3d(0,0.05,0),0.046,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.52 -0.2,-1.11 +0.5,2.33 -0.55),vec3d(0,-0.05,0),0.047 ,0.0,
                vec3d(0.41 -0.2,-1.04 +0.5,2.32 -0.55),vec3d(0,0.05,0),0.046,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.53 -0.2,-1.09 +0.5,2.25 -0.55),vec3d(0,-0.05,0),0.047 ,0.0,
                vec3d(0.42 -0.2,-1.03 +0.5,2.26 -0.55),vec3d(0,0.05,0),0.046,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.54 -0.2,-1.07 +0.5,2.19 -0.55),vec3d(0,-0.05,0.0),0.047 ,0.0,
                vec3d(0.45 -0.2,-1.01 +0.5,2.21 -0.55),vec3d(0,0.05,0.0),0.045,0.0,
                0.75,30);
    // 2ndes phalanges

    g.addSpline(
                vec3d(0.48 -0.2,-0.94 +0.5,2.44 -0.55),vec3d(0,0,0),0.047 ,0.0,
                vec3d(0.44 -0.2,-0.96 +0.5,2.38 -0.55),vec3d(0,0,0),0.044,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.42 -0.2,-1.05 +0.5,2.38 -0.55),vec3d(0,0,0),0.046 ,0.0,
                vec3d(0.42 -0.2,-0.97 +0.5,2.35 -0.55),vec3d(0,0,0),0.044,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.41 -0.2,-1.04 +0.5,2.32 -0.55),vec3d(0,0,0),0.046 ,0.0,
                vec3d(0.43 -0.2,-0.95 +0.5,2.30 -0.55),vec3d(0,0,0),0.044,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.42 -0.2,-1.03 +0.5,2.26 -0.55),vec3d(0,0,0),0.046 ,0.0,
                vec3d(0.44 -0.2,-0.94 +0.5,2.24 -0.55),vec3d(0,0,0),0.044,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.45 -0.2,-1.01 +0.5,2.21 -0.55),vec3d(0,0,0.0),0.045 ,0.0,
                vec3d(0.45 -0.2,-0.95 +0.5,2.20 -0.55),vec3d(0,0,0.0),0.043,0.0,
                0.75,30);

    // 3emes phalanges
    g.addSpline(
                vec3d(0.42 -0.2,-0.97 +0.5,2.35 -0.55),vec3d(0,0,0),0.044 ,0.0,
                vec3d(0.44 -0.2,-0.93 +0.5,2.335 -0.55),vec3d(0,0,0),0.0425,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.43 -0.2,-0.95 +0.5,2.30 -0.55),vec3d(0,0,0),0.044 ,0.0,
                vec3d(0.46 -0.2,-0.90 +0.5,2.285 -0.55),vec3d(0,0,0),0.043,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.44 -0.2,-0.94 +0.5,2.24 -0.55),vec3d(0,0,0),0.044 ,0.0,
                vec3d(0.46 -0.2,-0.91 +0.5,2.225 -0.55),vec3d(0,0,0),0.043,0.0,
                0.75,30);
    g.addSpline(
                vec3d(0.45 -0.2,-0.95 +0.5,2.20 -0.55),vec3d(0,0,0.0),0.043 ,0.0,
                vec3d(0.47 -0.2,-0.93 +0.5,2.22 -0.55),vec3d(0,0,0.0),0.0415,0.0,
                0.75,30);


    parsDebugInfo("Mudusa Left Hand: %d primitives\n", g.getNumPointPrimitives() );

    return g.getField();
}






/*!
\brief Creates the body of the medusa. The body is defined
by two vertical splines that create the overall shape effect.
Another spline screates a small round belly.
*/
CBlend * GalinMedusaGenerator::Medusa_Body()
{
    SplineGenerator g( SplineGenerator::SumBlendOfFastQuadricPointSets );


    // Torso
    g.addSpline( vec3d(-0.15,0.05,1.5),vec3d(0.4,-1,1.5),0.39,0.0,
                 vec3d(-0.37,0,3.0),vec3d(-1,0,2),0.4,0.0,
                 0.75,30);
    g.addSpline(vec3d(0.15,0.05,1.5),vec3d(-0.4,-1,1.5),0.39,0.0,
                vec3d(0.37,0,3.0),vec3d(1,0,2),0.4,0.0,
                0.75,30);

    // Epaules

    g.addSpline(vec3d(-0.21,0.0,3.0),vec3d(0.0,0.0,0.0),0.3,0.0,
                vec3d(-0.1,0.05,3.3),vec3d(0.0,0,0.0),0.1,0.0,
                0.75,30);
    g.addSpline(vec3d(0.21,0.0,3.0),vec3d(0.0,0.0,0.0),0.3,0.0,
                vec3d(0.1,0.05,3.3),vec3d(0.0,0,0.0),0.1,0.0,
                0.75,30);

    // arms

    // bras droit

    g.addSpline(vec3d(-0.5,-0.01,3.0),vec3d(0,0,0),/*0.27*/0.25 ,0.0,
                vec3d(-1.1,-0.3,2.5),vec3d(0,0,0),0.19,0.0,
                0.75,30);
    g.addSpline(vec3d(-1.1,-0.3,2.5),vec3d(0,0,0),0.19 ,0.0,
                vec3d(-0.7,-0.9,2.7),vec3d(0,0,0),0.12,0.0,
                0.75,30);

    // main droite

    // dessus

    g.addSpline(vec3d(-0.70,-0.92,2.29 +0.45),vec3d(0,0,0.05),0.09 ,0.0,
                vec3d(-0.57,-0.94,2.44 +0.45),vec3d(0,0,-0.05),0.047,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.70,-0.94,2.31 +0.45),vec3d(0,-0.1,0.1),0.078 ,0.0,
                vec3d(-0.53,-1.13,2.40 +0.45),vec3d(0,0.1,-0.05),0.047,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.72,-0.96,2.29 +0.45),vec3d(0,-0.1,0),0.06 ,0.0,
                vec3d(-0.52,-1.11,2.33 +0.45),vec3d(0,0.1,0),0.047,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.72,-0.96,2.26 +0.45),vec3d(0,-0.1,0),0.06 ,0.0,
                vec3d(-0.53,-1.09,2.25 +0.45),vec3d(0,0.1,0),0.047,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.68,-0.95,2.23 +0.45),vec3d(0,-0.1,-0.1),0.078 ,0.0,
                vec3d(-0.54,-1.07,2.19 +0.45),vec3d(0,0.1,0.05),0.047,0.0,
                0.75,30);

    // 1eres phalanges

    g.addSpline(vec3d(-0.57,-0.94,2.44 +0.45),vec3d(0,0,0),0.047 ,0.0,
                vec3d(-0.48,-0.94,2.44 +0.45),vec3d(0,0,0),0.047,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.53,-1.13,2.40 +0.45),vec3d(0,-0.05,0),0.047 ,0.0,
                vec3d(-0.42,-1.05,2.38 +0.45),vec3d(0,0.05,0),0.046,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.52,-1.11,2.33 +0.45),vec3d(0,-0.05,0),0.047 ,0.0,
                vec3d(-0.41,-1.04,2.32 +0.45),vec3d(0,0.05,0),0.046,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.53,-1.09,2.25 +0.45),vec3d(0,-0.05,0),0.047 ,0.0,
                vec3d(-0.42,-1.03,2.26 +0.45),vec3d(0,0.05,0),0.046,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.54,-1.07,2.19 +0.45),vec3d(0,-0.05,0.0),0.047 ,0.0,
                vec3d(-0.45,-1.01,2.21 +0.45),vec3d(0,0.05,0.0),0.045,0.0,
                0.75,30);

    // 2ndes phalanges
    g.addSpline(vec3d(-0.48,-0.94,2.44 +0.45),vec3d(0,0,0),0.047 ,0.0,
                vec3d(-0.44,-0.96,2.38 +0.45),vec3d(0,0,0),0.044,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.42,-1.05,2.38 +0.45),vec3d(0,0,0),0.046 ,0.0,
                vec3d(-0.42,-0.97,2.35 +0.45),vec3d(0,0,0),0.044,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.41,-1.04,2.32 +0.45),vec3d(0,0,0),0.046 ,0.0,
                vec3d(-0.43,-0.95,2.30 +0.45),vec3d(0,0,0),0.044,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.42,-1.03,2.26 +0.45),vec3d(0,0,0),0.046 ,0.0,
                vec3d(-0.44,-0.94,2.24 +0.45),vec3d(0,0,0),0.044,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.45,-1.01,2.21 +0.45),vec3d(0,0,0.0),0.045 ,0.0,
                vec3d(-0.45,-0.95,2.20 +0.45),vec3d(0,0,0.0),0.043,0.0,
                0.75,30);

    // 3emes phalanges
    g.addSpline(vec3d(-0.42,-0.97,2.35 +0.45),vec3d(0,0,0),0.044 ,0.0,
                vec3d(-0.44,-0.93,2.335 +0.45),vec3d(0,0,0),0.0425,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.43,-0.95,2.30 +0.45),vec3d(0,0,0),0.044 ,0.0,
                vec3d(-0.46,-0.90,2.285 +0.45),vec3d(0,0,0),0.043,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.44,-0.94,2.24 +0.45),vec3d(0,0,0),0.044 ,0.0,
                vec3d(-0.46,-0.91,2.225 +0.45),vec3d(0,0,0),0.043,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.45,-0.95,2.20 +0.45),vec3d(0,0,0.0),0.043 ,0.0,
                vec3d(-0.47,-0.93,2.22 +0.45),vec3d(0,0,0.0),0.0415,0.0,
                0.75,30);

    // bras gauche
    g.addSpline(vec3d(0.5,-0.01,3.0),vec3d(0,0,0),0.25 ,0.0,
                vec3d(0.7,0.2,2.2),vec3d(0,0,0),0.19,0.0,
                0.75,30);
    g.addSpline(vec3d(0.7,0.2,2.2),vec3d(0,0,0),0.19 ,0.0,
                vec3d(0.5,-0.4,1.7),vec3d(0,0,0),0.12,0.0,
                0.75,30);

    //	g.AddSpline(blend,Medusa_Lhand();

    // Belly
    g.addSpline(vec3d(0,0.08,1.0),vec3d(0,-1,1),0.41,0.0,
                vec3d(0,0.0,1.75),vec3d(0,0,1.0),0.44,0.0,
                0.75,30);

    parsDebugInfo("Mudusa Body: %d primitives\n", g.getNumPointPrimitives() );

    return g.getField();
}





/*
\brief Create the breasts. Basically create two spheres and
hyper-blend them so as to avoid over-blending. Added nipples
for special effects !
*/
CBlend * GalinMedusaGenerator::Medusa_Breast()
{
    SplineGenerator g( SplineGenerator::SumBlendOfFastQuadricPointSets );

    g.addSpline(vec3d(-0.21,/*-0.16*/-0.14,2.66),vec3d(0,-0.24,-0.4),0.36,0.0,
                vec3d(-0.23,/*-0.52*/-0.5,2.661),vec3d(0,0,0),0.05,0.0,
                0.75,20);
    g.addSpline(vec3d(0.21,/*-0.16*/-0.14,2.66),vec3d(0,-0.24,-0.4),0.34,0.0,
                vec3d(0.23,/*-0.52*/-0.5,2.661),vec3d(0,0,0),0.05,0.0,
                0.75,20);

    parsDebugInfo("Mudusa Chest: %d primitives\n", g.getNumPointPrimitives() );

    return g.getField();
}






CBlend * GalinMedusaGenerator::Medusa_Hair()
{
    SplineGenerator g( SplineGenerator::SumBlendOfFastQuadricPointSets );

    //meche avant
    g.addSpline(vec3d(-0.07,-0.535,4.25 +0.35),vec3d(-0.1,-0.1,0.0),0.04,0.05,
                vec3d(-0.12,-0.57,4.12 +0.35),vec3d(0.1,0.0,0.0),0.03,0.05,
                0.75,35);
    g.addSpline(vec3d(-0.03,-0.535,4.25 +0.35),vec3d(-0.1,-0.1,0.0),0.04,0.05,
                vec3d(-0.06,-0.57,4.12 +0.35),vec3d(0.1,0.0,0.0),0.02,0.05,
                0.75,35);
    //premier rang droit
    g.addSpline(vec3d(-0.02,-0.24,4.35 +0.35),vec3d(0.0,-0.1,0.3),0.02,0.05,
                vec3d(-0.02,-0.57,4.26 +0.35),vec3d(0.0,-0.1,-0.2),0.02,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.06,-0.26,4.35 +0.35),vec3d(0.0,-0.1,0.3),0.03,0.05,
                vec3d(-0.03,-0.57,4.26 +0.35),vec3d(0.0,-0.1,-0.2),0.02,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.09,-0.28,4.35 +0.35),vec3d(0.0,-0.1,0.3),0.03,0.05,
                vec3d(-0.05,-0.57,4.26 +0.35),vec3d(0.0,-0.1,-0.2),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.12,-0.30,4.34 +0.35),vec3d(0.0,-0.1,0.3),0.03,0.05,
                vec3d(-0.07,-0.56,4.26 +0.35),vec3d(0.0,-0.1,-0.2),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.15,-0.32,4.32 +0.35),vec3d(-0.05,-0.1,0.3),0.03,0.05,
                vec3d(-0.10,-0.56,4.25 +0.35),vec3d(0.05,-0.1,-0.2),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.18,-0.34,4.30 +0.35),vec3d(-0.1,-0.1,0.3),0.03,0.05,
                vec3d(-0.13,-0.56,4.25 +0.35),vec3d(0.1,-0.1,-0.2),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.22,-0.36,4.28 +0.35),vec3d(-0.1,-0.1,0.2),0.04,0.05,
                vec3d(-0.15,-0.565,4.25 +0.35),vec3d(0.1,-0.1,-0.1),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.26,-0.34,4.26 +0.35),vec3d(-0.1,-0.1,0.1),0.04,0.05,
                vec3d(-0.155,-0.55,4.235 +0.35),vec3d(0.1,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.29,-0.34,4.22 +0.35),vec3d(-0.2,-0.1,0.0),0.04,0.05,
                vec3d(-0.12,-0.56,4.24 +0.35),vec3d(0.2,0.1,0.3),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.33,-0.30,4.18 +0.35),vec3d(-0.2,-0.1,-0.1),0.04,0.05,
                vec3d(-0.16,-0.50,4.20 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.35,-0.24,4.14 +0.35),vec3d(-0.2,-0.1,-0.1),0.04,0.05,
                vec3d(-0.19,-0.44,4.16 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.365,-0.19,4.10 +0.35),vec3d(-0.2,-0.1,-0.1),0.04,0.05,
                vec3d(-0.21,-0.39,4.12 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.37,-0.14,4.06 +0.35),vec3d(-0.2,-0.1,-0.1),0.04,0.05,
                vec3d(-0.23,-0.34,4.08 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.375,-0.10,4.02 +0.35),vec3d(-0.2,-0.1,-0.1),0.03,0.05,
                vec3d(-0.24,-0.30,4.02 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.36,-0.09,4.00 +0.35),vec3d(-0.2,-0.1,-0.1),0.03,0.05,
                vec3d(-0.25,-0.27,3.98 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.36,-0.08,3.98 +0.35),vec3d(-0.2,-0.1,-0.1),0.03,0.05,
                vec3d(-0.25,-0.24,3.94 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.36,-0.07,3.96 +0.35),vec3d(-0.2,-0.1,-0.1),0.03,0.05,
                vec3d(-0.25,-0.23,3.90 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.35,-0.06,3.94 +0.35),vec3d(-0.2,-0.1,-0.1),0.03,0.05,
                vec3d(-0.24,-0.21,3.86 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.34,-0.05,3.92 +0.35),vec3d(-0.2,-0.1,-0.1),0.03,0.05,
                vec3d(-0.23,-0.18,3.80 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.33,-0.04,3.90 +0.35),vec3d(-0.2,-0.1,-0.1),0.03,0.05,
                vec3d(-0.22,-0.15,3.76 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.32,-0.03,3.88 +0.35),vec3d(-0.2,-0.1,-0.1),0.03,0.05,
                vec3d(-0.21,-0.10,3.72 +0.35),vec3d(0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.31,0.00,3.86 +0.35),vec3d(-0.2,-0.1,-0.1),0.03,0.05,
                vec3d(-0.20,-0.06,3.70 +0.35),vec3d(0.1,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.30,0.03,3.84 +0.35),vec3d(-0.2,-0.05,-0.1),0.03,0.05,
                vec3d(-0.19,-0.02,3.68 +0.35),vec3d(0.2,0.05,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.29,0.06,3.82 +0.35),vec3d(-0.2,0.0,-0.1),0.03,0.05,
                vec3d(-0.18,0.02,3.66 +0.35),vec3d(0.2,0.0,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.28,0.10,3.80 +0.35),vec3d(-0.2,0.05,-0.1),0.03,0.05,
                vec3d(-0.17,0.06,3.64 +0.35),vec3d(0.1,-0.05,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.26,0.13,3.78 +0.35),vec3d(-0.2,0.1,-0.1),0.03,0.05,
                vec3d(-0.16,0.10,3.62 +0.35),vec3d(0.2,-0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.20,0.16,3.76 +0.35),vec3d(-0.2,0.1,-0.1),0.03,0.05,
                vec3d(-0.13,0.14,3.60 +0.35),vec3d(0.2,-0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.16,0.18,3.75 +0.35),vec3d(-0.2,0.1,-0.1),0.03,0.05,
                vec3d(-0.10,0.18,3.59 +0.35),vec3d(0.2,-0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.12,0.20,3.74 +0.35),vec3d(-0.2,0.1,-0.1),0.03,0.05,
                vec3d(-0.07,0.22,3.58 +0.35),vec3d(0.2,-0.1,0.3),0.04,0.05,
                0.75,40);
    // Second rang droit
    g.addSpline(vec3d(-0.02,-0.20,4.40 +0.35),vec3d(0.0,0.0,0.1),0.04,0.05,
                vec3d(-0.02,-0.36,4.36 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.06,-0.20,4.40 +0.35),vec3d(0.0,0.0,0.1),0.04,0.05,
                vec3d(-0.07,-0.36,4.35 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.10,-0.20,4.40 +0.35),vec3d(0.0,0.0,0.1),0.04,0.05,
                vec3d(-0.12,-0.36,4.34 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.14,-0.20,4.40 +0.35),vec3d(0.0,0.0,0.1),0.04,0.05,
                vec3d(-0.15,-0.36,4.31 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.17,-0.20,4.38 +0.35),vec3d(0.0,0.0,0.1),0.04,0.05,
                vec3d(-0.22,-0.36,4.28 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.20,-0.16,4.36 +0.35),vec3d(-0.1,-0.1,0.1),0.04,0.05,
                vec3d(-0.26,-0.3,4.26 +0.35),vec3d(0.1,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.22,-0.13,4.34 +0.35),vec3d(-0.2,-0.1,0.1),0.03,0.05,
                vec3d(-0.29,-0.28,4.22 +0.35),vec3d(0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.23,-0.10,4.32 +0.35),vec3d(-0.2,-0.1,0.1),0.03,0.05,
                vec3d(-0.33,-0.26,4.18 +0.35),vec3d(0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.25,-0.07,4.30 +0.35),vec3d(-0.2,-0.1,0.1),0.03,0.05,
                vec3d(-0.35,-0.24,4.14 +0.35),vec3d(0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.27,-0.04,4.28 +0.35),vec3d(-0.2,-0.1,0.1),0.03,0.05,
                vec3d(-0.365,-0.19,4.10 +0.35),vec3d(0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.265,-0.01,4.26 +0.35),vec3d(-0.2,-0.1,0.1),0.03,0.05,
                vec3d(-0.37,-0.14,4.06 +0.35),vec3d(0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.265,0.02,4.25 +0.35),vec3d(-0.2,-0.1,0.1),0.03,0.05,
                vec3d(-0.375,-0.10,4.02 +0.35),vec3d(0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.26,0.05,4.24 +0.35),vec3d(-0.3,-0.1,-0.1),0.03,0.05,
                vec3d(-0.35,-0.09,4.00 +0.35),vec3d(0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.25,0.08,4.22 +0.35),vec3d(-0.3,-0.1,-0.1),0.03,0.05,
                vec3d(-0.35,-0.07,3.96 +0.35),vec3d(0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.24,0.11,4.20 +0.35),vec3d(-0.3,-0.1,-0.1),0.03,0.05,
                vec3d(-0.31,-0.03,3.88 +0.35),vec3d(0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.22,0.14,4.18 +0.35),vec3d(-0.3,-0.1,-0.1),0.03,0.05,
                vec3d(-0.29,0.03,3.84 +0.35),vec3d(0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.19,0.17,4.16 +0.35),vec3d(-0.3,-0.1,-0.1),0.03,0.05,
                vec3d(-0.28,0.06,3.82 +0.35),vec3d(0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.16,0.19,4.14 +0.35),vec3d(-0.3,-0.1,-0.1),0.03,0.05,
                vec3d(-0.25,0.10,3.80 +0.35),vec3d(0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.13,0.21,4.12 +0.35),vec3d(-0.3,-0.1,-0.1),0.03,0.05,
                vec3d(-0.22,0.13,3.78 +0.35),vec3d(0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.10,0.22,4.12 +0.35),vec3d(-0.3,-0.1,-0.1),0.03,0.05,
                vec3d(-0.19,0.16,3.78 +0.35),vec3d(0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.07,0.23,4.12 +0.35),vec3d(-0.3,-0.1,-0.1),0.03,0.05,
                vec3d(-0.15,0.18,3.78 +0.35),vec3d(0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.04,0.24,4.12 +0.35),vec3d(-0.3,-0.1,-0.1),0.03,0.05,
                vec3d(-0.11,0.20,3.78 +0.35),vec3d(0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    // Troisieme rang droit
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.02,-0.20,4.40 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.06,-0.20,4.40 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.10,-0.20,4.40 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.14,-0.20,4.40 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.17,-0.20,4.38 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.20,-0.16,4.36 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.22,-0.13,4.34 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.23,-0.10,4.32 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.25,-0.07,4.30 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.27,-0.04,4.28 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.265,-0.01,4.26 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.265,0.02,4.25 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.26,0.05,4.24 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.25,0.08,4.22 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.24,0.11,4.20 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.22,0.14,4.18 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.19,0.17,4.16 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.16,0.19,4.14 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.13,0.21,4.12 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.10,0.22,4.12 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.07,0.23,4.12 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(-0.04,0.24,4.12 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);

    //premier rang gauche
    g.addSpline(vec3d(0.00,-0.24,4.35 +0.35),vec3d(0.0,-0.1,0.3),0.02,0.05,
                vec3d(0.00,-0.57,4.26 +0.35),vec3d(0.0,-0.1,-0.2),0.02,0.05,
                0.75,40);
    g.addSpline(vec3d(0.02,-0.24,4.35 +0.35),vec3d(0.0,-0.1,0.3),0.02,0.05,
                vec3d(0.02,-0.57,4.26 +0.35),vec3d(0.0,-0.1,-0.2),0.02,0.05,
                0.75,40);
    g.addSpline(vec3d(0.06,-0.26,4.35 +0.35),vec3d(0.0,-0.1,0.3),0.03,0.05,
                vec3d(0.03,-0.57,4.26 +0.35),vec3d(0.0,-0.1,-0.2),0.02,0.05,
                0.75,40);
    g.addSpline(vec3d(0.09,-0.28,4.35 +0.35),vec3d(0.0,-0.1,0.3),0.03,0.05,
                vec3d(0.05,-0.57,4.26 +0.35),vec3d(0.0,-0.1,-0.2),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(0.12,-0.30,4.34 +0.35),vec3d(0.0,-0.1,0.3),0.03,0.05,
                vec3d(0.07,-0.56,4.26 +0.35),vec3d(0.0,-0.1,-0.2),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(0.15,-0.32,4.32 +0.35),vec3d(0.05,-0.1,0.3),0.03,0.05,
                vec3d(0.10,-0.56,4.25 +0.35),vec3d(-0.05,-0.1,-0.2),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(0.18,-0.34,4.30 +0.35),vec3d(0.1,-0.1,0.3),0.03,0.05,
                vec3d(0.13,-0.56,4.25 +0.35),vec3d(-0.1,-0.1,-0.2),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(0.22,-0.36,4.28 +0.35),vec3d(0.1,-0.1,0.2),0.04,0.05,
                vec3d(0.15,-0.565,4.25 +0.35),vec3d(-0.1,-0.1,-0.1),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(0.26,-0.34,4.26 +0.35),vec3d(0.1,-0.1,0.1),0.04,0.05,
                vec3d(0.155,-0.55,4.235 +0.35),vec3d(-0.1,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.29,-0.34,4.22 +0.35),vec3d(0.2,-0.1,0.0),0.04,0.05,
                vec3d(0.12,-0.56,4.24 +0.35),vec3d(-0.2,0.1,0.3),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(0.33,-0.30,4.18 +0.35),vec3d(0.2,-0.1,-0.1),0.04,0.05,
                vec3d(0.16,-0.50,4.20 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.35,-0.24,4.14 +0.35),vec3d(0.2,-0.1,-0.1),0.04,0.05,
                vec3d(0.19,-0.44,4.16 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.365,-0.19,4.10 +0.35),vec3d(0.2,-0.1,-0.1),0.04,0.05,
                vec3d(0.21,-0.39,4.12 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.37,-0.14,4.06 +0.35),vec3d(0.2,-0.1,-0.1),0.04,0.05,
                vec3d(0.23,-0.34,4.08 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.375,-0.10,4.02 +0.35),vec3d(0.2,-0.1,-0.1),0.03,0.05,
                vec3d(0.24,-0.30,4.02 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.36,-0.09,4.00 +0.35),vec3d(0.2,-0.1,-0.1),0.03,0.05,
                vec3d(0.25,-0.27,3.98 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.36,-0.08,3.98 +0.35),vec3d(0.2,-0.1,-0.1),0.03,0.05,
                vec3d(0.25,-0.24,3.94 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.36,-0.07,3.96 +0.35),vec3d(0.2,-0.1,-0.1),0.03,0.05,
                vec3d(0.25,-0.23,3.90 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.35,-0.06,3.94 +0.35),vec3d(0.2,-0.1,-0.1),0.03,0.05,
                vec3d(0.24,-0.21,3.86 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.34,-0.05,3.92 +0.35),vec3d(0.2,-0.1,-0.1),0.03,0.05,
                vec3d(0.23,-0.18,3.80 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.33,-0.04,3.90 +0.35),vec3d(0.2,-0.1,-0.1),0.03,0.05,
                vec3d(0.22,-0.15,3.76 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.32,-0.03,3.88 +0.35),vec3d(0.2,-0.1,-0.1),0.03,0.05,
                vec3d(0.21,-0.10,3.72 +0.35),vec3d(-0.2,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.31,0.00,3.86 +0.35),vec3d(0.2,-0.1,-0.1),0.03,0.05,
                vec3d(0.20,-0.06,3.70 +0.35),vec3d(-0.1,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.30,0.03,3.84 +0.35),vec3d(0.2,-0.05,-0.1),0.03,0.05,
                vec3d(0.19,-0.02,3.68 +0.35),vec3d(-0.2,0.05,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.29,0.06,3.82 +0.35),vec3d(0.2,0.0,-0.1),0.03,0.05,
                vec3d(0.18,0.02,3.66 +0.35),vec3d(-0.2,0.0,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.28,0.10,3.80 +0.35),vec3d(0.2,0.05,-0.1),0.03,0.05,
                vec3d(0.17,0.06,3.64 +0.35),vec3d(-0.1,-0.05,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.26,0.13,3.78 +0.35),vec3d(0.2,0.1,-0.1),0.03,0.05,
                vec3d(0.16,0.10,3.62 +0.35),vec3d(-0.2,-0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.20,0.16,3.76 +0.35),vec3d(0.2,0.1,-0.1),0.03,0.05,
                vec3d(0.13,0.14,3.60 +0.35),vec3d(-0.2,-0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.16,0.18,3.75 +0.35),vec3d(0.2,0.1,-0.1),0.03,0.05,
                vec3d(0.10,0.18,3.59 +0.35),vec3d(-0.2,-0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.12,0.20,3.74 +0.35),vec3d(0.2,0.1,-0.1),0.03,0.05,
                vec3d(0.07,0.22,3.58 +0.35),vec3d(-0.2,-0.1,0.3),0.04,0.05,
                0.75,40);
    // Second rang gauche
    g.addSpline(vec3d(0.02,-0.20,4.40 +0.35),vec3d(0.0,0.0,0.1),0.04,0.05,
                vec3d(0.02,-0.36,4.36 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.06,-0.20,4.40 +0.35),vec3d(0.0,0.0,0.1),0.04,0.05,
                vec3d(0.07,-0.36,4.35 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.10,-0.20,4.40 +0.35),vec3d(0.0,0.0,0.1),0.04,0.05,
                vec3d(0.12,-0.36,4.34 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.14,-0.20,4.40 +0.35),vec3d(0.0,0.0,0.1),0.04,0.05,
                vec3d(0.15,-0.36,4.31 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.17,-0.20,4.38 +0.35),vec3d(0.0,0.0,0.1),0.04,0.05,
                vec3d(0.22,-0.36,4.28 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.20,-0.16,4.36 +0.35),vec3d(0.1,-0.1,0.1),0.04,0.05,
                vec3d(0.26,-0.3,4.26 +0.35),vec3d(-0.1,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.22,-0.13,4.34 +0.35),vec3d(0.2,-0.1,0.1),0.03,0.05,
                vec3d(0.29,-0.28,4.22 +0.35),vec3d(-0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.23,-0.10,4.32 +0.35),vec3d(0.2,-0.1,0.1),0.03,0.05,
                vec3d(0.33,-0.26,4.18 +0.35),vec3d(-0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.25,-0.07,4.30 +0.35),vec3d(0.2,-0.1,0.1),0.03,0.05,
                vec3d(0.35,-0.24,4.14 +0.35),vec3d(-0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.27,-0.04,4.28 +0.35),vec3d(0.2,-0.1,0.1),0.03,0.05,
                vec3d(0.365,-0.19,4.10 +0.35),vec3d(-0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.265,-0.01,4.26 +0.35),vec3d(0.2,-0.1,0.1),0.03,0.05,
                vec3d(0.37,-0.14,4.06 +0.35),vec3d(-0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.265,0.02,4.25 +0.35),vec3d(0.2,-0.1,0.1),0.03,0.05,
                vec3d(0.375,-0.10,4.02 +0.35),vec3d(-0.2,0.1,0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.26,0.05,4.24 +0.35),vec3d(0.3,-0.1,-0.1),0.03,0.05,
                vec3d(0.35,-0.09,4.00 +0.35),vec3d(-0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.25,0.08,4.22 +0.35),vec3d(0.3,-0.1,-0.1),0.03,0.05,
                vec3d(0.35,-0.07,3.96 +0.35),vec3d(-0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.24,0.11,4.20 +0.35),vec3d(0.3,-0.1,-0.1),0.03,0.05,
                vec3d(0.31,-0.03,3.88 +0.35),vec3d(-0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.22,0.14,4.18 +0.35),vec3d(0.3,-0.1,-0.1),0.03,0.05,
                vec3d(0.29,0.03,3.84 +0.35),vec3d(-0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.19,0.17,4.16 +0.35),vec3d(0.3,-0.1,-0.1),0.03,0.05,
                vec3d(0.28,0.06,3.82 +0.35),vec3d(-0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.16,0.19,4.14 +0.35),vec3d(0.3,-0.1,-0.1),0.03,0.05,
                vec3d(0.25,0.10,3.80 +0.35),vec3d(-0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.13,0.21,4.12 +0.35),vec3d(0.3,-0.1,-0.1),0.03,0.05,
                vec3d(0.22,0.13,3.78 +0.35),vec3d(-0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.10,0.22,4.12 +0.35),vec3d(0.3,-0.1,-0.1),0.03,0.05,
                vec3d(0.19,0.16,3.78 +0.35),vec3d(-0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.07,0.23,4.12 +0.35),vec3d(0.3,-0.1,-0.1),0.03,0.05,
                vec3d(0.15,0.18,3.78 +0.35),vec3d(-0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.04,0.24,4.12 +0.35),vec3d(0.3,-0.1,-0.1),0.03,0.05,
                vec3d(0.11,0.20,3.78 +0.35),vec3d(-0.3,0.1,0.3),0.04,0.05,
                0.75,40);
    // Troisieme rang gauche
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.02,-0.20,4.40 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.06,-0.20,4.40 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.10,-0.20,4.40 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.14,-0.20,4.40 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.17,-0.20,4.38 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.20,-0.16,4.36 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.22,-0.13,4.34 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.23,-0.10,4.32 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.25,-0.07,4.30 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.27,-0.04,4.28 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.265,-0.01,4.26 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.265,0.02,4.25 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.26,0.05,4.24 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.25,0.08,4.22 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.24,0.11,4.20 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.22,0.14,4.18 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.19,0.17,4.16 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.16,0.19,4.14 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.13,0.21,4.12 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.10,0.22,4.12 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.07,0.23,4.12 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.04,0.24,4.12 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.10,4.37 +0.35),vec3d(0.0,0.0,0.1),0.005,0.05,
                vec3d(0.0,0.24,4.12 +0.35),vec3d(0.0,0.0,-0.1),0.04,0.05,
                0.75,40);
    // Derriere
    g.addSpline(vec3d(-0.2,0.14,3.59 +0.35),vec3d(-0.2,0.3,0.0),0.05,0.05,
                vec3d(0.11,0.19,4.17 +0.35),vec3d(0.6,-0.3,0.0),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.16,0.19,3.58 +0.35),vec3d(-0.2,0.3,0.0),0.05,0.05,
                vec3d(0.09,0.21,4.18 +0.35),vec3d(0.5,-0.3,0.0),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.12,0.24,3.57 +0.35),vec3d(-0.2,0.3,0.0),0.05,0.05,
                vec3d(0.07,0.23,4.19 +0.35),vec3d(0.4,-0.3,0.0),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.08,0.29,3.56 +0.35),vec3d(-0.2,0.3,0.0),0.05,0.05,
                vec3d(0.05,0.25,4.20 +0.35),vec3d(0.3,-0.3,0.0),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.04,0.34,3.55 +0.35),vec3d(-0.2,0.3,0.0),0.05,0.05,
                vec3d(0.03,0.27,4.21 +0.35),vec3d(0.2,-0.3,0.0),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(-0.0075,0.35,3.55 +0.35),vec3d(-0.1,0.3,0.0),0.04,0.05,
                vec3d(0.0,0.27,4.21 +0.35),vec3d(0.1,-0.3,0.0),0.02,0.05,
                0.75,40);
    g.addSpline(vec3d(0.2,0.14,3.59 +0.35),vec3d(0.2,0.3,0.0),0.05,0.05,
                vec3d(-0.11,0.19,4.17 +0.35),vec3d(-0.6,-0.3,0.0),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(0.16,0.19,3.58 +0.35),vec3d(0.2,0.3,0.0),0.05,0.05,
                vec3d(-0.09,0.21,4.18 +0.35),vec3d(-0.5,-0.3,0.0),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(0.12,0.24,3.57 +0.35),vec3d(0.2,0.3,0.0),0.05,0.05,
                vec3d(-0.07,0.23,4.19 +0.35),vec3d(-0.4,-0.3,0.0),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(0.08,0.29,3.56 +0.35),vec3d(0.2,0.3,0.0),0.05,0.05,
                vec3d(-0.05,0.25,4.20 +0.35),vec3d(-0.3,-0.3,0.0),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(0.04,0.34,3.55 +0.35),vec3d(0.2,0.3,0.0),0.05,0.05,
                vec3d(-0.03,0.27,4.21 +0.35),vec3d(-0.2,-0.3,0.0),0.03,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0075,0.35,3.55 +0.35),vec3d(0.1,0.3,0.0),0.04,0.05,
                vec3d(-0.0,0.27,4.21 +0.35),vec3d(-0.1,-0.3,0.0),0.02,0.05,
                0.75,40);
    g.addSpline(vec3d(0.0,0.35,3.55 +0.35),vec3d(0.0,0.3,0.0),0.04,0.05,
                vec3d(-0.0,0.27,4.21 +0.35),vec3d(0.0,-0.3,0.0),0.02,0.05,
                0.75,40);

    parsDebugInfo("Mudusa Hair: %d primitives\n", g.getNumPointPrimitives() );

    return g.getField();
}



CBlend * GalinMedusaGenerator::Medusa_Tete()
{
    SplineGenerator g( SplineGenerator::SumBlendOfFastQuadricPointSets );

    // Crane

    // [ RMS NOTE TODO: this first spline is scaled by (0.85, 1,1 ) in eric's code...

    g.addSpline(vec3d(0.0,0.0,4.05 +0.35),vec3d(0,0,0),0.5,0.075,
                vec3d(0.0,-0.2,3.55 +0.35),vec3d(0,0,0),0.17,0.05,
                0.75,20);



    // Joues
    g.addSpline(vec3d(-0.09,-0.13,3.74 +0.35),vec3d(0.2,0.0,-0.1),0.25,0.01,
                vec3d(-0.02,-0.4,3.68 +0.35),vec3d(0.0,0.0,0.7),0.05,0.07,
                0.75,20);
    g.addSpline(vec3d(0.09,-0.1,3.74 +0.35),vec3d(-0.2,0.0,-0.1),0.25,0.01,
                vec3d(0.02,-0.4,3.68 +0.35),vec3d(0.0,0.0,0.7),0.05,0.07,
                0.75,20);
    // Mouth
    //non moustache
    g.addSpline(vec3d(0.025,-0.325,3.62 +0.35),vec3d(-0.02,-0.05,0.0),0.09,0.0,
                vec3d(0.02,-0.35,3.71 +0.35),vec3d(0.02,0.05,0.0),0.06,0.0,
                0.75,30);
    g.addSpline(vec3d(-0.025,-0.325,3.62 +0.35),vec3d(0.02,-0.05,0.0),0.09,0.0,
                vec3d(-0.02,-0.35,3.71 +0.35),vec3d(-0.02,0.05,0.0),0.06,0.0,
                0.75,30);
    // bouche boudeuse
    /* g.AddSpline(vec3d(-0.08,-0.339,3.54 +0.35),vec3d(-0.1,-0.2,0.2),0.02,0.08, */
    /* vec3d(-0.0,-0.415,3.62 +0.35),vec3d(0.05,0.0,0.05),0.015,0.01, */
    /* -0.75,30); */
    /* g.AddSpline(vec3d(0.08,-0.339,3.54 +0.35),vec3d(0.1,-0.2,0.2),0.02,0.08, */
    /* vec3d(0.0,-0.415,3.62 +0.35),vec3d(-0.05,0.0,0.05),0.015,0.01, */
    /* -0.75,30); */
    // autre
    g.addSpline(vec3d(-0.08,-0.345,3.61 +0.35),vec3d(-0.1,-0.2,-0.05),0.013,0.08,
                vec3d(-0.0,-0.415,3.61 +0.35),vec3d(0.05,0.0,0.0),0.017,0.01,
                -0.75,30);
    g.addSpline(vec3d(0.08,-0.345,3.61 +0.35),vec3d(0.1,-0.2,-0.05),0.013,0.08,
                vec3d(0.0,-0.415,3.61 +0.35),vec3d(-0.05,0.0,0.0),0.017,0.01,
                -0.75,30);
    // Machoires
    g.addSpline(vec3d(-0.165,-0.23,3.8 +0.35),vec3d(0.2,-0.1,-0.1),0.15,0.075,
                vec3d(-0.02,-0.34,3.48 +0.35),vec3d(0.1,-0.4,0.2),0.08,0.05,
                0.75,20);
    g.addSpline(vec3d(0.165,-0.23,3.8 +0.35),vec3d(-0.2,-0.1,-0.1),0.15,0.075,
                vec3d(0.02,-0.34,3.48 +0.35),vec3d(-0.1,-0.4,0.2),0.08,0.05,
                0.75,20);
    // Double menton
    g.addSpline(vec3d(0.0,-0.1,3.6 +0.35),vec3d(0.0,0.0,0.0),0.2,0.05,
                vec3d(0.0,-0.3,3.475 +0.35),vec3d(0.0,0.0,0.0),0.05,0.05,
                0.75,20);
    // Orbites
    g.addSpline(vec3d(-0.15,-0.4,3.908 +0.35),vec3d(0.0,0.0,0.0),0.132,0.01,
                vec3d(-0.1,-0.395,3.52 +0.35),vec3d(0.0,0.0,0.0),0.01,0.05,
                -0.3,30);
    g.addSpline(vec3d(0.15,-0.4,3.908 +0.35),vec3d(0.0,0.0,0.0),0.132,0.01,
                vec3d(0.1,-0.395,3.52 +0.35),vec3d(0.0,0.0,0.0),0.01,0.05,
                -0.3,30);


    // Paumettes
    g.addSpline(vec3d(-0.1,-0.25,3.9 +0.35),vec3d(0.0,-0.05,-0.9),0.1,0.05,
                vec3d(-0.1,-0.16,3.55 +0.35),vec3d(0.0,0.3,-0.1),0.06,0.05,
                0.9,20);
    g.addSpline(vec3d(0.1,-0.25,3.9 +0.35),vec3d(0.0,-0.05,-0.9),0.14,0.05,
                vec3d(0.1,-0.16,3.55 +0.35),vec3d(0.0,0.3,-0.1),0.06,0.05,
                0.9,20);

    // Eyes
    g.addSpline(vec3d(-0.155,-0.3,3.9 +0.35),vec3d(0.0,0.0,0.0),0.085,0.01,
                vec3d(-0.1,-0.295,3.82 +0.35),vec3d(0.0,0.0,0.0),0.05,0.05,
                0.75,30);
    g.addSpline(vec3d(0.155,-0.3,3.9 +0.35),vec3d(0.0,0.0,0.0),0.085,0.01,
                vec3d(-0.1,-0.295,3.82 +0.35),vec3d(0.0,0.0,0.0),0.05,0.05,
                0.75,30);


    // pupilles
    //g.AddSpline(vec3d(-0.145,-0.355,3.9 +0.35),vec3d(0.0,0.0,0.0),0.035,0.0,
    //	vec3d(-0.155,-0.3,3.89 +0.35),vec3d(0.0,0.0,0.0),0.01,0.0,
    //	-0.75,30);
    //g.AddSpline(vec3d(0.145,-0.355,3.9 +0.35),vec3d(0.0,0.0,0.0),0.035,0.0,
    //	vec3d(0.135,-0.3,3.89 +0.35),vec3d(0.0,0.0,0.0),0.01,0.0,
    //	-0.75,30);

    //Tour de yeux haut
    g.addSpline(vec3d(-0.22,-0.32,3.92 +0.35),vec3d(0.2,-0.1,0.1),0.025,0.05,
                vec3d(-0.07,-0.345,3.92 +0.35),vec3d(0.2,0.05,-0.1),0.023,0.05,
                0.75,30);
    g.addSpline(vec3d(0.22,-0.32,3.92 +0.35),vec3d(-0.2,-0.1,0.1),0.025,0.05,
                vec3d(0.07,-0.345,3.92 +0.35),vec3d(-0.2,0.05,-0.1),0.023,0.05,
                0.75,30);
    //Tour de yeux bas
    g.addSpline(vec3d(-0.2,-0.33,3.88 +0.35),vec3d(0.2,-0.07,-0.1),0.022,0.05,
                vec3d(-0.09,-0.345,3.88 +0.35),vec3d(0.2,0.05,0.1),0.022,0.05,
                0.75,30);
    g.addSpline(vec3d(0.2,-0.33,3.88 +0.35),vec3d(-0.2,-0.07,-0.1),0.022,0.05,
                vec3d(0.09,-0.345,3.88 +0.35),vec3d(-0.2,0.05,0.1),0.022,0.05,
                0.75,30);
    // Nez
    g.addSpline(vec3d(0.0,-0.355,3.93 +0.35),vec3d(0.0,0.0,0.0),0.035,0.05,
                vec3d(0.0,-0.41,3.73 +0.35),vec3d(0.0,0.0,0.0),0.06,0.05,
                0.75,30);
    g.addSpline(vec3d(0.0,-0.375,3.93 +0.35),vec3d(0.0,0.0,0.0),0.025,0.05,
                vec3d(0.0,-0.445,3.73 +0.35),vec3d(0.0,0.0,0.0),0.045,0.05,
                0.75,30);
    g.addSpline(vec3d(-0.025,-0.36,3.73 +0.35),vec3d(0.0,0.0,0.0),0.05,0.05,
                vec3d(-0.01,-0.44,3.72 +0.35),vec3d(0.0,0.0,0.0),0.03,0.05,
                0.75,30);
    g.addSpline(vec3d(0.025,-0.36,3.73 +0.35),vec3d(0.0,0.0,0.0),0.05,0.05,
                vec3d(0.01,-0.44,3.72 +0.35),vec3d(0.0,0.0,0.0),0.03,0.05,
                0.75,30);

    // Ears
    /* g.AddSpline(vec3d(-0.43,0.0,3.90 +0.35),vec3d(0,0.2,0.1),0.05,0.075, */
    /* vec3d(-0.37,0.0,3.71 +0.35),vec3d(0,-0.2,-0.1),0.04,0.05, */
    /* 0.75,20); */
    /* g.AddSpline(vec3d(0.43,0.0,3.90 +0.35),vec3d(0,0.2,0.1),0.05,0.075, */
    /* vec3d(0.37,0.0,3.71 +0.35),vec3d(0,-0.2,-0.1),0.04,0.05, */
    /* 0.75,20); */

    parsDebugInfo("Mudusa Head: %d primitives\n", g.getNumPointPrimitives() );

    return g.getField();
}





#if 0

BlobTree* Medusa_Create()

{

    TreeNode *cheveux=Medusa_Cheveux();

    TreeNode *tete=Medusa_Tete();

    TreeNode *tail=Medusa_Tail();

    TreeNode *body=Medusa_Body();

    TreeNode *breast=Medusa_Breast();

    TreeNode *neck=Medusa_Neck();



    cheveux=new TreeScale(new TreeRotate(cheveux,Matrix::Rotation(Vector(0,0,0.3))),Vector(0.9));

    tete=new TreeTranslate(

                new TreeScale(

                    new TreeRotate(tete,Matrix::Rotation(Vector(0,0,0.3))),

                    Vector(0.9)),

                Vector(0,-0.15,0));

    /*

cheveux=cheveux->Rotate(Matrix::Rotation(Vector(0,0,0.3)));

cheveux=cheveux->Scale(Vector(0.9));

tete=tete->Rotate(Matrix::Rotation(Vector(0,0,0.3)));

tete=tete->Scale(Vector(0.9));

tete=tete->Translate(Vector(0,-0.15,0));

*/

    TreeNode* medusa=cheveux;

    medusa=new TreeBlend(medusa, tete);

    medusa=new TreeBlend(medusa, neck);

    medusa=new TreeBlend(medusa, body);

    medusa=new TreeBlend(medusa, breast);

    medusa=new TreeBlend(medusa, tail);

    return new BlobTree(medusa,0.45);

}

BlobTree* BlobTreeSphere()

{

    return new BlobTree(new TreeSphere(Vector(0.0),1.0,new BlendCubic(1.0,1.0)),0.5);

}
#endif

