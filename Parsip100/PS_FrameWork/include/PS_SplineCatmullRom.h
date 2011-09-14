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

#include <vector>
#include <istream>
#include "PS_Vector.h"
#include "PS_Octree.h"

#define DEFAULT_NUM_POINTS_PER_SEGMENT 400

using namespace PS::MATH;

namespace PS{

class CSplineCatmullRom
{
public:
	struct ARCLENGTHPARAM{
		float u;//Parameter
		float g;//ArcLength Accumulated
		vec3f pt;
	};


public:
	CSplineCatmullRom();
	CSplineCatmullRom(const CSplineCatmullRom& cspline);
	~CSplineCatmullRom();

	void set(const CSplineCatmullRom& rhs);

	bool isValid() const { return ((m_vCtrlPoints.size() >= 4) && (m_vArcTable.size() > 0)); }
	bool isCtrlPointIndex(U32 index) const {return (index >=0 && index < m_vCtrlPoints.size());}
	std::vector<vec3>& getControlPoints() {return m_vCtrlPoints;}
	std::vector<ARCLENGTHPARAM>& getArcTable() {return m_vArcTable;}
	void getArcPoints(std::vector<vec3f>& lstPoints) const;
	COctree getOctree() const;


	size_t getCtrlPointsCount() const {return m_vCtrlPoints.size();}
	vec3f  getPoint(size_t i);

	void setPoint(int index, const vec3f& pt);
	void addPoint(const vec3f& p);
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

	ARCLENGTHPARAM getArcTableEntry(size_t i);
	int getArcTableCount();

	void drawCtrlLine(unsigned int gl_draw_mode = 0);
	void drawCurve(unsigned int gl_draw_mode = 3);

	//Serialize Control Points
	friend std::ostream& operator <<(std::ostream& outs, const CSplineCatmullRom& curve);
	friend std::istream& operator >>(std::istream& ins, const CSplineCatmullRom& curve);
private:	
	bool isPointIndexCorrect(size_t i);    
	bool getLocalSpline(float t, float &local_t, int *indices);


private:	
	size_t  m_nSegments;	
	std::vector<vec3f> m_vCtrlPoints;
	std::vector<ARCLENGTHPARAM> m_vArcTable;
};

}

#endif
