//-------------------------------------------------------------------------------------------
//  University of Victoria Computer Science Department 
//	Assignment04
//  Course title: Computer Animation CSC578B
//	Please report any comments or feedback to: pouryash@uvic.ca
//
//	Author: Pourya Shirazian 
//  Student# V00681446
//	Date  : January 2009
//-------------------------------------------------------------------------------------------

#include "PS_SplineCatmullRom.h"
//#include "GL/glew.h"
#include <iostream>
#include <string>
#include <sstream>
#include <istream>
#include "mathHelper.h"

using namespace std;

namespace PS{

CSplineCatmullRom::CSplineCatmullRom(const CSplineCatmullRom& cspline)
{
    set(cspline);
}


CSplineCatmullRom::CSplineCatmullRom():m_vCtrlPoints(), m_nSegments(DEFAULT_NUM_POINTS_PER_SEGMENT)
{}

void CSplineCatmullRom::set(const CSplineCatmullRom& rhs)
{
    removeAll();
    for (unsigned int i=0; i < rhs.m_vCtrlPoints.size(); i++)
        m_vCtrlPoints.push_back(rhs.m_vCtrlPoints[i]);
    populateTable();
}

CSplineCatmullRom::~CSplineCatmullRom()
{
    removeAll();
}


void CSplineCatmullRom::setSegmentsCount(U32 nSegments)
{
	if(nSegments > 0)		
		m_nSegments = nSegments;
}

int CSplineCatmullRom::getArcTableCount()
{
	return m_vArcTable.size();
}

CSplineCatmullRom::ARCLENGTHPARAM CSplineCatmullRom::getArcTableEntry(size_t i)
{
	ARCLENGTHPARAM dummyEntry;
	if((i >=0)&&(i < m_vArcTable.size()))
		return m_vArcTable[i];
	else
		return dummyEntry;
}

void CSplineCatmullRom::getArcPoints(std::vector<vec3f>& lstPoints) const
{
	if(m_vArcTable.size() < 2) return;
	lstPoints.resize(m_vArcTable.size());	
	for(size_t i=0; i<m_vArcTable.size(); i++)
	{
		lstPoints[i] = m_vArcTable[i].pt;
	}
}

COctree CSplineCatmullRom::getOctree() const
{
	COctree oct;
	oct.set(m_vCtrlPoints);
	return oct;
}

bool CSplineCatmullRom::populateTable()
{
	int nCtrlPoints = m_vCtrlPoints.size();
	if(nCtrlPoints < 4)
		return false;


	int n = m_nSegments;
	float u = (float)(1.0f /(float)(n-1));

	//Erase Table
	m_vArcTable.resize(n);

	for(int i=0; i < n; i++)
	{
		float cu = (float)(u * i);

		ARCLENGTHPARAM arcLenParam;
		arcLenParam.pt = position(cu);
		arcLenParam.u = cu;
		arcLenParam.g = 0.0f;
		if(i > 0)
		{
			arcLenParam.g = m_vArcTable[i - 1].g + m_vArcTable[i-1].pt.distance(arcLenParam.pt);
		}
		m_vArcTable[i] = arcLenParam;
	}

	//Normalize ArcLength Parameter
	float TotalArcLen = m_vArcTable[n-1].g;
	for(int i=0; i < n; i++)
	{
		m_vArcTable[i].g = m_vArcTable[i].g / TotalArcLen;
	}

	return (m_vArcTable.size() > 0);
}

bool CSplineCatmullRom::populateTableAdaptive()
{
	int nCtrlPoints = m_vCtrlPoints.size();
	if(nCtrlPoints < 4)
		return false;

	//Erase Table
	m_vArcTable.clear();

	float delta = 0.001f;
	float cu = 0.0f;
	vec3f prevTan = tangent(0.0f);
	prevTan.normalize();
	vec3f curTan;
	int i = 0;

	do
	{
		ARCLENGTHPARAM arcLenParam;
				
		arcLenParam.pt = position(cu);
		arcLenParam.u = cu;
		arcLenParam.g = 0.0f;
		if(i > 0)
		{
			arcLenParam.g = m_vArcTable[i - 1].g + m_vArcTable[i - 1].pt.distance(arcLenParam.pt);
		}
		m_vArcTable.push_back(arcLenParam);
		i++;
		
		//update Steps
		curTan = tangent(cu + delta);
		curTan.normalize();
		
		
		float angle = curTan.getAngleDeg(prevTan);
		if(FLOAT_EQ(angle, 0.0f, 0.5f))
			delta *= 2.0f;
		else		
			delta *= 0.5f; 

		cu += delta;
		prevTan = curTan;
	}while(cu  < 1.0f);

	if(i>0)
	{
		if(m_vArcTable[i-1].u < 1.0f)
		{
			ARCLENGTHPARAM arcLenParam;
			arcLenParam.pt = position(1.0f);
			arcLenParam.u = 1.0f;
			arcLenParam.g = m_vArcTable[i - 1].g + m_vArcTable[i - 1].pt.distance(arcLenParam.pt);
			m_vArcTable.push_back(arcLenParam);
		}
	}

	//Normalize ArcLength Parameter
	int n = m_vArcTable.size();
	float TotalArcLen = m_vArcTable[n - 1].g;
	for(i=0; i < n; i++)
	{
		m_vArcTable[i].g = m_vArcTable[i].g / TotalArcLen;
	}

	return (n > 0);
}

bool CSplineCatmullRom::isTableEmpty()
{
	return (m_vArcTable.size() == 0);
}

float CSplineCatmullRom::getArcLength() const
{
	size_t n = m_vArcTable.size();
	if(n > 0)
		return m_vArcTable[n-1].g;
	else
		return 0.0f;
}

float CSplineCatmullRom::arcLengthViaTable(float u)
{
	if(isTableEmpty())
		return 0.0f;

	if(u < 0.0f)
		u = 0.0f;
	if(u > 1.0f)
		u = 1.0f;

	int n = m_vArcTable.size();
	float d = (float)(1.0f /(float)n);

	int i = static_cast<int>(floor(u / d));

	float arcLen;
	if((i >=0)&&(i < n - 1))
		arcLen = m_vArcTable[i].g + float((u - m_vArcTable[i].u)/(m_vArcTable[i + 1].u - m_vArcTable[i].u)) * (m_vArcTable[i+1].g - m_vArcTable[i].g);
	else 
		//	if(i == n - 1)
		//	arcLen = varcTable[i].g + float((u - varcTable[i].u)/(1.00f - varcTable[i].u)) * (1.00f - varcTable[i].g);
		//	else
		arcLen = m_vArcTable[n-1].g;

	return arcLen;
}

float CSplineCatmullRom::parameterViaTable(float arcLength)
{
	//if((arcLength < 0)||(arcLength > 1.0f))
	if(arcLength < 0)
		return 0.0f;
	if(isTableEmpty())
		return 0.0f;

	int n = m_vArcTable.size();
	int i;

	for (i = n - 1; i >= 0; i--)
	{
		if(arcLength > m_vArcTable[i].g)
			break;
	}

	float u,s;
	s = arcLength;

	if((i >=0)&&(i < n - 1))
		u = m_vArcTable[i].u + (s - m_vArcTable[i].g)/(m_vArcTable[i+1].g - m_vArcTable[i].g)*(m_vArcTable[i+1].u - m_vArcTable[i].u);
	else if(i == n - 1)
		u = m_vArcTable[i].u + (s - m_vArcTable[i].g)/(1.00f - m_vArcTable[i].g)*(1.00f - m_vArcTable[i].u);
	else
		u = 1.00f;


	return u;
}

void CSplineCatmullRom::addPoint(const vec3f& p)
{
	m_vCtrlPoints.push_back(p);
}

void CSplineCatmullRom::removePoint(size_t i)
{
	if(isPointIndexCorrect(i))
		m_vCtrlPoints.erase(m_vCtrlPoints.begin() + i);
}

void CSplineCatmullRom::removeAll()
{
	m_vCtrlPoints.resize(0);
	m_vArcTable.resize(0);
}

vec3f CSplineCatmullRom::getPoint(size_t i)
{
	return m_vCtrlPoints[i];
}

void CSplineCatmullRom::setPoint(int index, const vec3f& pt)
{
	if(!isPointIndexCorrect(index))
		return;

	float x = m_vCtrlPoints[index].x;
	float y = m_vCtrlPoints[index].y;
	float z = m_vCtrlPoints[index].z;

	m_vCtrlPoints[index] = pt;

	x = m_vCtrlPoints[index].x;
	y = m_vCtrlPoints[index].y;
	z = m_vCtrlPoints[index].z;
}

bool CSplineCatmullRom::isPointIndexCorrect(size_t i)
{
	return (i >= 0)&&(i < m_vCtrlPoints.size());
}

bool CSplineCatmullRom::getLocalSpline(float t, float &local_t, int *indices)
{
	int n = m_vCtrlPoints.size();
	if(n < 4)	
		return false;

    Clampf(t, 0.0f, 1.0f);

	int nSlices = n - 3;
	int p = static_cast<int>(ceil(nSlices * t));
	if(p == 0) p++;

	indices[0] = p-1;
	indices[1] = p;
	indices[2] = p+1;
	indices[3] = p+2;


	float delta = (1.0f / static_cast<float>(nSlices));
	local_t = (t - static_cast<float>(p - 1) * delta) / delta;

	return true;
}


vec3f CSplineCatmullRom::tangent(float t)
{
	vec3f v;
	int indices[4];
	float local;

	if(!getLocalSpline(t, local, indices))	
	{
		return v;
	}

	// Interpolate
	return tangent(local, m_vCtrlPoints[indices[0]], m_vCtrlPoints[indices[1]], m_vCtrlPoints[indices[2]], m_vCtrlPoints[indices[3]]);
}

vec3f CSplineCatmullRom::tangent(float t, vec3f& p0, vec3f& p1, vec3f& p2, vec3f& p3)
{
	//p(u) = U^T * M * B
	//U^ = [3u^2 2u 1 0]

	//M Matrix =0.5 * [-1 3 -3  1]
	//				  [2 -5  4 -1]
	//				  [-1 0  1  0]
	//                [0  2  0  0]
	//B = [Pi-1 pi pi+1 pi+2]
	float t2 = t * t;

	float b0 = float(0.5 * (-3*t2 + 4*t - 1));
	float b1 = float(0.5 * (9*t2 -10*t));
	float b2 = float(0.5 * (-9*t2 + 8*t + 1));
	float b3 = float(0.5 * (3*t2 - 2*t));

	return (p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3);
}

vec3f CSplineCatmullRom::acceleration(float t)
{
	vec3f v;
	int indices[4];
	float local;

	if(!getLocalSpline(t, local, indices))	
	{
		return v;
	}

	// Interpolate
	return acceleration(local, m_vCtrlPoints[indices[0]], m_vCtrlPoints[indices[1]], m_vCtrlPoints[indices[2]], m_vCtrlPoints[indices[3]]);
}

vec3f CSplineCatmullRom::acceleration(float t, vec3f& p0, vec3f& p1, vec3f& p2, vec3f& p3)
{
	//p(u) = U^T * M * B
	//U^^ = [6^u 2 0 0]

	//M Matrix =0.5 * [-1 3 -3  1]
	//                [2 -5  4 -1]
	//		  [-1 0  1  0]
	//                [0  2  0  0]
	//B = [Pi-1 pi pi+1 pi+2]

	float b0 = float(0.5 * (-6*t + 4));
	float b1 = float(0.5 * (18*t - 10));
	float b2 = float(0.5 * (-18*t + 8));
	float b3 = float(0.5 * (6*t - 2));

	return (p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3);
}


vec3f CSplineCatmullRom::position(float t)
{	
	vec3f v;
	int indices[4];
	float local;

	if(!getLocalSpline(t, local, indices))	
	{
		return v;
	}

	// Interpolate
	return position(local, m_vCtrlPoints[indices[0]], m_vCtrlPoints[indices[1]], m_vCtrlPoints[indices[2]], m_vCtrlPoints[indices[3]]);
}


vec3f CSplineCatmullRom::position(float t, vec3f& p0, vec3f& p1, vec3f& p2, vec3f& p3)
{
	//p(u) = U^T * M * B
	//U = [u^3 u^2 u^1 1]

	//M Matrix =0.5 * [-1 3 -3  1]
	//                [2 -5  4 -1]
	//				  [-1 0  1  0]
	//                [0  2  0  0]
	//B = [Pi-1 pi pi+1 pi+2]
	float t2 = t * t;
	float t3 = t2 * t;

	float b0 = float(0.5 * (-t3 + 2*t2 - t));
	float b1 = float(0.5 * (3*t3 -5*t2 + 2));
	float b2 = float(0.5 * (-3*t3 + 4*t2 + t));
	float b3 = float(0.5 * (t3 - t2));

	return (p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3);
}

void CSplineCatmullRom::drawCtrlLine(unsigned int gl_draw_mode)
{
	/*
	vec3f v;
	glPushAttrib(GL_ALL_ATTRIB_BITS);
		glBegin(gl_draw_mode);
		for(size_t i=0; i<vCtrlPoints.size(); i++)
		{
			v = vCtrlPoints[i];
			glVertex3f(v.x, v.y, v.z);
		}
		glEnd();
	glPopAttrib();
	*/
}


void CSplineCatmullRom::drawCurve(unsigned int gl_draw_mode)
{
	/*
	glPushAttrib(GL_ALL_ATTRIB_BITS);
		glBegin(gl_draw_mode);
		for(size_t i=0; i<vArcTable.size(); i++)
		{
			glVertex3fv(vArcTable[i].pt.ptr());
		}
		glEnd();
	glPopAttrib();
	*/
}


std::istream& operator >>( istream& ins, CSplineCatmullRom& curve )
{
	int ctCtrlPoints = 0;	
	ins >> ctCtrlPoints;

	std::vector<vec3> ctrlPoints = curve.getControlPoints();
	if(ctCtrlPoints > 0)
	{
		ctrlPoints.resize(ctCtrlPoints);
		
		float x, y, z;		
		for(int i=0; i<ctCtrlPoints; i++)
		{
			ins >> x >> y >> z;
			ctrlPoints[i] = vec3f(x, y, z);
		}
	}
	return ins;
}

std::ostream& operator <<( std::ostream& outs, const CSplineCatmullRom& curve )
{
	int ctCtrlPoints = curve.m_vCtrlPoints.size();

	if(ctCtrlPoints > 0)
	{
		outs << ctCtrlPoints;

		vec3f p;
		for(int i=0; i<ctCtrlPoints; i++)
		{
			p = curve.m_vCtrlPoints[i];
			outs << p.x << p.y << p.z;	
		}
	}

	return outs;
}

}
