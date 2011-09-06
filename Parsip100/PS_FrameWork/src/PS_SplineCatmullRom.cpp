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
#include "GL/glew.h"
#include <iostream>
#include <string>
#include <sstream>
#include <istream>

#define DEFAULT_NUM_POINTS_PER_SEGMENT 400

using namespace std;

CSplineCatmullRom::CSplineCatmullRom(const CSplineCatmullRom& cspline)
{
	for (unsigned int i=0; i < cspline.vCtrlPoints.size(); i++)
	{
		vCtrlPoints.push_back(cspline.vCtrlPoints[i]);
	}	
}

CSplineCatmullRom::CSplineCatmullRom():vCtrlPoints(), m_nSegments(DEFAULT_NUM_POINTS_PER_SEGMENT)
{}

CSplineCatmullRom::~CSplineCatmullRom()
{
	vCtrlPoints.clear();
	vArcTable.clear();
}

void CSplineCatmullRom::setSegmentsCount(UINT nSegments)
{
	if(nSegments > 0)		
		m_nSegments = nSegments;
}

int CSplineCatmullRom::GetArcTableCount()
{
	return vArcTable.size();
}

ARCLENGTHPARAM CSplineCatmullRom::GetArcTableEntry(size_t i)
{
	ARCLENGTHPARAM dummyEntry;
	if((i >=0)&&(i < vArcTable.size()))
		return vArcTable[i];
	else
		return dummyEntry;
}

void CSplineCatmullRom::getArcPoints(DVec<vec3f>& lstPoints) const
{
	if(vArcTable.size() < 2) return;
	lstPoints.resize(vArcTable.size());	
	for(size_t i=0; i<vArcTable.size(); i++)
	{
		lstPoints[i] = vArcTable[i].pt;
	}
}

bool CSplineCatmullRom::populateTable()
{
	int nCtrlPoints = vCtrlPoints.size();
	if(nCtrlPoints < 4)
		return false;

	computeOctree();

	int n = m_nSegments;
	float u = (float)(1.0f /(float)(n-1));

	//Erase Table
	vArcTable.resize(n);

	for(int i=0; i < n; i++)
	{
		float cu = (float)(u * i);

		ARCLENGTHPARAM arcLenParam;
		arcLenParam.pt = position(cu);
		arcLenParam.u = cu;
		arcLenParam.g = 0.0f;
		if(i > 0)
		{
			arcLenParam.g = vArcTable[i - 1].g + vArcTable[i-1].pt.distance(arcLenParam.pt);
		}
		vArcTable[i] = arcLenParam;
	}

	//Normalize ArcLength Parameter
	float TotalArcLen = vArcTable[n-1].g;
	for(int i=0; i < n; i++)
	{
		vArcTable[i].g = vArcTable[i].g / TotalArcLen;
	}

	return (vArcTable.size() > 0);
}

bool CSplineCatmullRom::populateTableAdaptive()
{
	int nCtrlPoints = vCtrlPoints.size();
	if(nCtrlPoints < 4)
		return false;

	computeOctree();
	
	//Erase Table
	vArcTable.clear();

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
			arcLenParam.g = vArcTable[i - 1].g + vArcTable[i - 1].pt.distance(arcLenParam.pt);
		}
		vArcTable.push_back(arcLenParam);
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
		if(vArcTable[i-1].u < 1.0f)
		{
			ARCLENGTHPARAM arcLenParam;
			arcLenParam.pt = position(1.0f);
			arcLenParam.u = 1.0f;
			arcLenParam.g = vArcTable[i - 1].g + vArcTable[i - 1].pt.distance(arcLenParam.pt);
			vArcTable.push_back(arcLenParam);
		}
	}

	//Normalize ArcLength Parameter
	int n = vArcTable.size();
	float TotalArcLen = vArcTable[n - 1].g;
	for(i=0; i < n; i++)
	{
		vArcTable[i].g = vArcTable[i].g / TotalArcLen;
	}

	return (n > 0);
}

bool CSplineCatmullRom::isTableEmpty()
{
	return (vArcTable.size() == 0);
}

float CSplineCatmullRom::getArcLength() const
{
	size_t n = vArcTable.size();
	if(n > 0)
		return vArcTable[n-1].g;
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

	int n = vArcTable.size();
	float d = (float)(1.0f /(float)n);

	int i = static_cast<int>(floor(u / d));

	float arcLen;
	if((i >=0)&&(i < n - 1))
		arcLen = vArcTable[i].g + float((u - vArcTable[i].u)/(vArcTable[i + 1].u - vArcTable[i].u)) * (vArcTable[i+1].g - vArcTable[i].g);
	else 
		//	if(i == n - 1)
		//	arcLen = varcTable[i].g + float((u - varcTable[i].u)/(1.00f - varcTable[i].u)) * (1.00f - varcTable[i].g);
		//	else
		arcLen = vArcTable[n-1].g;

	return arcLen;
}

float CSplineCatmullRom::parameterViaTable(float arcLength)
{
	//if((arcLength < 0)||(arcLength > 1.0f))
	if(arcLength <= 0.0f)
		return 0.0f;
	if(isTableEmpty())
		return 0.0f;

	int n = vArcTable.size();
	int i;

	for (i = n - 1; i >= 0; i--)
	{
		if(arcLength > vArcTable[i].g)
			break;
	}

	float u,s;
	s = arcLength;

	if((i >=0)&&(i < n - 1))
		u = vArcTable[i].u + (s - vArcTable[i].g)/(vArcTable[i+1].g - vArcTable[i].g)*(vArcTable[i+1].u - vArcTable[i].u);
	else if(i == n - 1)
		u = vArcTable[i].u + (s - vArcTable[i].g)/(1.00f - vArcTable[i].g)*(1.00f - vArcTable[i].u);
	else
		u = 1.00f;


	return u;
}

void CSplineCatmullRom::addPoint(vec3f& p)
{
	vCtrlPoints.push_back(p);
}

void CSplineCatmullRom::removePoint(size_t i)
{
	if(vCtrlPoints.isItemIndex(i))
		vCtrlPoints.erase(vCtrlPoints.begin() + i);
}

void CSplineCatmullRom::removeAll()
{
	vCtrlPoints.clear();
	vArcTable.clear();
}

vec3f CSplineCatmullRom::getPoint(size_t i)
{
	return vCtrlPoints[i];
}

void CSplineCatmullRom::setPoint(int index, vec3f& pt)
{
	if(!vCtrlPoints.isItemIndex(index))	return;
	vCtrlPoints[index] = pt;
}

bool CSplineCatmullRom::getLocalSpline(float t, float &local_t, int *indices)
{
	int n = vCtrlPoints.size();
	if(n < 4)	
		return false;

	clampf(t, 0.0f, 1.0f);

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
	return tangent(local, vCtrlPoints[indices[0]], vCtrlPoints[indices[1]], vCtrlPoints[indices[2]], vCtrlPoints[indices[3]]);
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
	return acceleration(local, vCtrlPoints[indices[0]], vCtrlPoints[indices[1]], vCtrlPoints[indices[2]], vCtrlPoints[indices[3]]);
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
	return position(local, vCtrlPoints[indices[0]], vCtrlPoints[indices[1]], vCtrlPoints[indices[2]], vCtrlPoints[indices[3]]);
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
}


void CSplineCatmullRom::drawCurve(unsigned int gl_draw_mode)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
		glBegin(gl_draw_mode);
		for(size_t i=0; i<vArcTable.size(); i++)
		{
			glVertex3fv(vArcTable[i].pt.ptr());
		}
		glEnd();
	glPopAttrib();
}

void CSplineCatmullRom::computeOctree()
{
	m_octree.set(&vCtrlPoints[0], (int)vCtrlPoints.size());
}


std::istream& operator >>( istream& ins, CSplineCatmullRom& curve )
{
	int ctCtrlPoints = 0;	
	ins >> ctCtrlPoints;

	DVec<vec3> ctrlPoints = curve.getControlPoints();
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
	int ctCtrlPoints = curve.vCtrlPoints.size();

	if(ctCtrlPoints > 0)
	{
		outs << ctCtrlPoints;

		vec3f p;
		for(int i=0; i<ctCtrlPoints; i++)
		{
			p = curve.vCtrlPoints[i];
			outs << p.x << p.y << p.z;	
		}
	}

	return outs;
}