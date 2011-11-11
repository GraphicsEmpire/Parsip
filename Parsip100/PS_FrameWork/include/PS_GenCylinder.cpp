
#include <assert.h>
#include "mathHelper.h"
#include "_dataTypes.h"
#include "PS_GenCylinder.h"


namespace PS{
namespace MODELING{

    CGenCylinderCrossSection::CGenCylinderCrossSection()
    {
        m_ctSectors = DEFAULT_SECTORS;
        m_radius = DEFAULT_RADIUS;
        m_position = vec3f(0.0f, 0.0f, 0.0f);
        m_normal = vec3f(0.0f, 0.0f, 0.0f);
        calc();
    }

    CGenCylinderCrossSection::CGenCylinderCrossSection(int nSectors, float radius)
    {
        m_ctSectors = nSectors;
        m_radius = radius;
        m_position = vec3f(0.0f, 0.0f, 0.0f);
        m_normal = vec3f(0.0f, 0.0f, 0.0f);
        calc();
    }

    CGenCylinderCrossSection::CGenCylinderCrossSection(const CGenCylinderCrossSection& rhs)
    {
        this->set(rhs);
    }

    void CGenCylinderCrossSection::set(const CGenCylinderCrossSection& rhs)
    {
        removeAllPoints();
        m_ctSectors = rhs.m_ctSectors;
        m_radius    = rhs.m_radius;
        m_position  = rhs.m_position;
        m_tangent   = rhs.m_tangent;
        m_normal    = rhs.m_normal;
        m_binormal  = rhs.m_binormal;

        for(size_t i=0; i < rhs.m_lstPoints.size(); i++)
            m_lstPoints.push_back(rhs.m_lstPoints[i]);
        for(size_t i=0; i < rhs.m_lstNormals.size(); i++)
            m_lstNormals.push_back(rhs.m_lstNormals[i]);

    }

    CGenCylinderCrossSection::~CGenCylinderCrossSection() {
        removeAllPoints();
    }

    void CGenCylinderCrossSection::removeAllPoints()
    {
        m_lstNormals.resize(0);
        m_lstPoints.resize(0);
    }


    void CGenCylinderCrossSection::calc()
    {
        float angle = (float)(TwoPi /(float)m_ctSectors);

        m_lstPoints.resize(0);
        m_lstNormals.resize(0);

        vec3f C, pt, n;
        for(int i=0; i < m_ctSectors; i++)
        {
            //1.Point on 2D cross section
            C.x = m_radius * cos(i * angle);
            C.y = m_radius * sin(i * angle);
            C.z = 1.0f;

            //2.Point on cylinder
            pt.x = m_position.x + C.x * m_normal.x + C.y * m_binormal.x;
            pt.y = m_position.y + C.x * m_normal.y + C.y * m_binormal.y;
            pt.z = m_position.z + C.x * m_normal.z + C.y * m_binormal.z;

            //Compute Normal
            n = pt - m_position;
            n.normalize();

            m_lstPoints.push_back(pt);
            m_lstNormals.push_back(n);
        }
    }

    void CGenCylinderCrossSection::getFrenetFrame(vec3f& T, vec3f& N, vec3f& B) const
    {
        T = m_tangent;
        N = m_normal;
        B = m_binormal;
    }

    void CGenCylinderCrossSection::setFrenetFrame(vec3f T, vec3f N, vec3f B)
    {
        m_tangent = T;
        m_normal = N;
        m_binormal = B;
    }

    vec3f CGenCylinderCrossSection::getPoint(size_t i) const
    {
        assert(i >=0 && i<m_lstPoints.size());
        return m_lstPoints[i];
    }

    vec3f CGenCylinderCrossSection::getNormal(size_t i) const
    {
        assert(i >=0 && i<m_lstNormals.size());
        return m_lstNormals[i];
    }

    /*
    void CGenCylinderCrossSection::draw(int glArg)
    {
        glPushMatrix();
        glBegin(glArg);
        for(size_t i=0; i< m_lstPoints.size(); i++)
            glVertex3f(m_lstPoints[i].x, m_lstPoints[i].y, m_lstPoints[i].z);
        glEnd();
        glPopMatrix();
    }
    */
    ////////////////////////////////////////////////////////////////////////////////////////
    // CGenCylinder
    CGenCylinder::CGenCylinder()
    {
        m_radiusStart = DEFAULT_RADIUS;
        m_radiusEnd   = DEFAULT_RADIUS;
        m_ctSectors = DEFAULT_SECTORS;
        m_ctStacks = DEFAULT_STACKS;
    }

    CGenCylinder::CGenCylinder(int nSectors, int nStacks, float startRadius, float endRadius)
    {
        m_radiusStart = startRadius;
        m_radiusEnd = endRadius;
        m_ctSectors = nSectors;
        m_ctStacks  = nStacks;
    }

    CGenCylinder::CGenCylinder(const CGenCylinder& rhs)
    {
        m_ctStacks  = rhs.getStacksCount();
        m_ctSectors = rhs.getSectorsCount();

        m_radiusStart = rhs.getRadiusStart();
        m_radiusEnd   = rhs.getRadiusEnd();
        m_profileCurve.set(rhs.m_profileCurve);


        for(size_t i=0; i<rhs.m_lstStacks.size(); i++)
        {
            CGenCylinderCrossSection *lpCrossSection = new CGenCylinderCrossSection(*rhs.m_lstStacks[i]);
            m_lstStacks.push_back(lpCrossSection);
        }
    }


    CGenCylinder::~CGenCylinder()
    {
        removeAll();
    }

    void CGenCylinder::removeAll()
    {
        for(size_t i=0; i< m_lstStacks.size(); i++)
            SAFE_DELETE(m_lstStacks[i]);
        m_lstStacks.resize(0);
        m_profileCurve.removeAll();
    }

    CGenCylinderCrossSection* CGenCylinder::getStack(int idx)
    {
    	assert(idx >= 0 && idx < (int)m_lstStacks.size());
    	return m_lstStacks[idx];
    }


    bool CGenCylinder::calc()
    {
        if(m_profileCurve.getCtrlPointsCount() < 4)
            return false;
        m_profileCurve.populateTable();
        if(m_lstStacks.size() > 0)
        {
            for(size_t i=0; i< m_lstStacks.size(); i++)
                SAFE_DELETE(m_lstStacks[i]);
            m_lstStacks.resize(0);
        }

        vec3f xAxis(1.0f, 0.0f, 0.0f);
        vec3f yAxis(0.0f, 1.0f, 0.0f);
        vec3f zAxis(0.0f, 0.0f, 1.0f);

        float t = 0.0f;
        //Current Position + Frenet Frame
        vec3f T, N, B, P;

        //Previous Frenet Frame
        vec3f prevN, prevB;

        vec3f A;

        float deltaRadius = (m_radiusEnd - m_radiusStart) / static_cast<float>(m_ctStacks);
        float radius = m_radiusStart;

        for (int i=0; i < m_ctStacks; i++)
        {
            radius = m_radiusStart + i * deltaRadius;

            t = static_cast<float>(i)/ static_cast<float>(m_ctStacks - 1);

            P = m_profileCurve.position(t);

            T = m_profileCurve.tangent(t);
            T.normalize();

            if(i == 0)
            {
                A = m_profileCurve.acceleration(t);

                if(A.length() < EPSILON)
                    A = T.cross(yAxis);
                A.normalize();

                //N = normalized(VxQxV)
                N = T.cross(A.cross(T)); //v = u*w
                N.normalize();

                //B = TxN
                B = T.cross(N);
                B.normalize();
            }
            else
            {
                N = prevB.cross(T);
                B = T.cross(N);
            }

            //Save for Next Iteration
            prevN = N;
            prevB = B;

            //At this point we have the frenet frame
            CGenCylinderCrossSection* lpBase = new CGenCylinderCrossSection();
            lpBase->setRadius(radius);
            lpBase->setSectors(m_ctSectors);
            lpBase->setPosition(P);
            lpBase->setFrenetFrame(T, N, B);
            lpBase->calc();

            m_lstStacks.push_back(lpBase);
        }

        return true;
    }

    /*
    void CGenCylinder::drawCurveControlPoints(bool bSelectMode)
    {
        vec3f p;
        glBegin(GL_POINTS);
        for(size_t i = 0; i< m_profileCurve.getCtrlPointsCount(); i++)
        {
            p = m_profileCurve.getPoint(i);

            if(bSelectMode)
                glPushName(i+1);

            glVertex3f(p.x, p.y, p.z);
            if(bSelectMode)
                glPopName();
        }
        glEnd();
    }

    void CGenCylinder::drawCurve()
    {
        //m_axisCurve.draw(GL_LINE_STRIP, 100);
        std::vector<ARCLENGTHPARAM> arc = m_profileCurve.getArcTable();
        if(arc.size() == 0)
            return;
        glBegin(GL_LINE_STRIP);
            for(size_t i=0;i<arc.size(); i++)
                glVertex3fv(arc[i].pt.ptr());
        glEnd();
    }

    void CGenCylinder::drawCrossSections()
    {
        size_t n = m_lstStacks.size();
        if(n == 0) return;
        for (size_t i=0; i< n; i++)
        {
            //m_lstStacks[i].calc();
            m_lstStacks[i].draw(GL_LINE_LOOP);
        }
    }

    void CGenCylinder::drawWireFrame()
    {
        if(m_lstStacks.size() < 2) return;

        size_t nPoints = m_lstStacks[0].getPointsCount();
        size_t nStacks = m_lstStacks.size();

        vec3f v1, v2;

        for (size_t i=0; i < nStacks - 1; i++)
        {
            glBegin(GL_LINE_STRIP);
            for(size_t j=0; j < nPoints; j++)
            {
                v1 = m_lstStacks[i].getPoint(j);
                v2 = m_lstStacks[i + 1].getPoint(j);
                glVertex3f(v1.x, v1.y, v1.z);
                glVertex3f(v2.x, v2.y, v2.z);
            }
            v1 = m_lstStacks[i].getPoint(0);
            v2 = m_lstStacks[i + 1].getPoint(0);
            glVertex3f(v1.x, v1.y, v1.z);
            glVertex3f(v2.x, v2.y, v2.z);

            glEnd();
        }
    }

    void CGenCylinder::drawNormals()
    {
        if(m_lstStacks.size() < 2) return;

        size_t nPoints = m_lstStacks[0].getPointsCount();
        size_t nStacks = m_lstStacks.size();

        vec3f v1, v2;
        vec3f n1, n2;
        vec3f t;

        for (size_t i=0; i < nStacks - 1; i++)
        {
            //m_lstStacks[i].calc();
            //m_lstStacks[i + 1].calc();

            glBegin(GL_LINES);
            for(size_t j=0; j < nPoints; j++)
            {
                v1 = m_lstStacks[i].getPoint(j);
                n1 = m_lstStacks[i].getNormal(j);

                v2 = m_lstStacks[i + 1].getPoint(j);
                n2 = m_lstStacks[i + 1].getNormal(j);
                //glNormal3fv(n1.ptr());
                glVertex3fv(v1.ptr());
                t = v1 + 0.2f * n1;
                glVertex3fv(t.ptr());

                //glNormal3fv(n2.ptr());
                glVertex3fv(v2.ptr());
                t = v2 + 0.2f * n2;
                glVertex3fv(t.ptr());

            }
            v1 = m_lstStacks[i].getPoint(0);
            n1 = m_lstStacks[i].getNormal(0);
            v2 = m_lstStacks[i + 1].getPoint(0);
            n2 = m_lstStacks[i + 1].getNormal(0);

            //glNormal3fv(n1.ptr());
            glVertex3fv(v1.ptr());
            t = v1 + 0.2f * n1;
            glVertex3fv(t.ptr());


            //glNormal3fv(n2.ptr());
            glVertex3fv(v2.ptr());
            t = v2 + 0.2f * n2;
            glVertex3fv(t.ptr());


            glEnd();
        }

    }

    void CGenCylinder::drawSurface()
    {
        if(m_lstStacks.size() < 2) return;

        size_t nPoints = m_lstStacks[0].getPointsCount();
        size_t nStacks = m_lstStacks.size();

        vec3f v1, v2;
        vec3f n1, n2;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        for (size_t i=0; i < nStacks - 1; i++)
        {
            //m_lstStacks[i].calc();
            //m_lstStacks[i + 1].calc();

            glBegin(GL_TRIANGLE_STRIP);
            for(size_t j=0; j < nPoints; j++)
            {
                v1 = m_lstStacks[i].getPoint(j);
                n1 = m_lstStacks[i].getNormal(j);

                v2 = m_lstStacks[i + 1].getPoint(j);
                n2 = m_lstStacks[i + 1].getNormal(j);
                glNormal3fv(n1.ptr());
                glVertex3fv(v1.ptr());

                glNormal3fv(n2.ptr());
                glVertex3fv(v2.ptr());
            }
            v1 = m_lstStacks[i].getPoint(0);
            n1 = m_lstStacks[i].getNormal(0);
            v2 = m_lstStacks[i + 1].getPoint(0);
            n2 = m_lstStacks[i + 1].getNormal(0);

            glNormal3fv(n1.ptr());
            glVertex3fv(v1.ptr());

            glNormal3fv(n2.ptr());
            glVertex3fv(v2.ptr());

            glEnd();
        }


    }

    */


}
}
