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

#ifndef CSPLINECATMULLROM_H
#define CSPLINECATMULLROM_H

#include "DSystem/include/DContainers.h"
#include "PS_FrameWork/include/PS_Octree.h"
#include "PS_Vector.h"

using namespace PS;
using namespace PS::MATH;

typedef unsigned int UINT;

struct ARCLENGTHPARAM{
	float u;//Parameter
	float g;//ArcLength Accumulated
	vec3f pt;
};

class CSplineCatmullRom
{

public:
	CSplineCatmullRom();
	CSplineCatmullRom(const CSplineCatmullRom& cspline);
	~CSplineCatmullRom();

	bool isValid() const { return ((vCtrlPoints.size() >= 4) && (vArcTable.size() > 0)); }
	DVec<vec3>& getControlPoints() {return vCtrlPoints;}
	DVec<ARCLENGTHPARAM>& getArcTable() {return vArcTable;}
	void getArcPoints(DVec<vec3f>& lstPoints) const;
	vec3f  getPoint(size_t i);
	COctree getOctree() const {return m_octree;}

	void setPoint(int index, vec3f& pt);
	void addPoint(vec3f& p);
	void removePoint(size_t i);
	void removeAll();

	void setSegmentsCount(size_t nSegments);
	size_t getSegmentsCount() const { return m_nSegments; }

	float getArcLength() const;
	//void selectCtrlPoint(int index, bool bSelect);
	//void deselectAll();

	//Get Position Tangent and Acceleration
	vec3f position(float t);
	vec3f position(float t, vec3f& p0, vec3f& p1, vec3f& p2, vec3f& p3);
	vec3f tangent(float t);
	vec3f tangent(float t, vec3f& p0, vec3f& p1, vec3f& p2, vec3f& p3);
	vec3f acceleration(float t);
	vec3f acceleration(float t, vec3f& p0, vec3f& p1, vec3f& p2, vec3f& p3);

	//Arc Length Parameterization
	bool populateTable();
	bool populateTableAdaptive();
	float arcLengthViaTable(float u);
	float parameterViaTable(float arcLength);
	bool isTableEmpty();

	ARCLENGTHPARAM GetArcTableEntry(size_t i);
	int GetArcTableCount();

	void drawCtrlLine(unsigned int gl_draw_mode = 0);
	void drawCurve(unsigned int gl_draw_mode = 3);

	//Serialize Control Points
	friend std::ostream& operator <<(std::ostream& outs, const CSplineCatmullRom& curve);
	friend std::istream& operator >>(std::istream& ins, const CSplineCatmullRom& curve);
private:	
	void computeOctree();
	bool getLocalSpline(float t, float &local_t, int *indices);


private:	
	COctree	m_octree;
	size_t  m_nSegments;	
public:
	DVec<vec3f> vCtrlPoints;
	DVec<ARCLENGTHPARAM> vArcTable;
};


#endif
