#ifndef CTWIST_H
#define CTWIST_H

#include <math.h>
#include "CBlobTree.h"

#define DEFAULT_WARP_FACTOR 1.0f

namespace PS{
namespace BLOBTREE{

//***********************************************************
//Twist 
class  CWarpTwist : public CBlobTree
{
private:
	float m_warpFactor;
	MajorAxices m_axis;

public:
	CWarpTwist() 
	{
		m_warpFactor = DEFAULT_WARP_FACTOR;
		m_axis = zAxis;		
	}

	CWarpTwist(CBlobTree * child)
	{
		addChild(child);
		m_warpFactor = DEFAULT_WARP_FACTOR;
		m_axis = zAxis;
	}

	CWarpTwist(CBlobTree * child, float warpFactor)
	{
		addChild(child);
		m_warpFactor = warpFactor;
		m_axis = zAxis;
		
	}

	CWarpTwist(CBlobTree * child, float warpFactor, MajorAxices axis)
	{
		addChild(child);
		m_warpFactor = warpFactor;
		m_axis = axis;		
	}

	CWarpTwist(float warpFactor, MajorAxices axis)
	{
		m_warpFactor = warpFactor;
		m_axis = axis;		
	}

	void setParamFrom(CBlobTree* input)
	{
		CWarpTwist* twistN = dynamic_cast<CWarpTwist*>(input);
		this->m_warpFactor = twistN->m_warpFactor;
		this->m_axis = twistN->m_axis;
	}

	float getWarpFactor() {return m_warpFactor;}
	void setWarpFactor(float factor) { m_warpFactor = factor;}

	MajorAxices getMajorAxis() {return m_axis;}
	void setMajorAxis(MajorAxices axis) { m_axis = axis;}

	__inline vec3f warp(vec3f p)
	{
		float theta = 0.0f;
		vec3f pw;

		switch(m_axis)
		{
		case(xAxis):		
			theta = p.x * m_warpFactor;
			pw.x = p.x;
			pw.y = p.y*cos(theta) - p.z*sin(theta);
			pw.z = p.y*sin(theta) + p.z*cos(theta);
			break;

		case(yAxis):
			theta = p.y * m_warpFactor;
			pw.x = p.x*cos(theta) - p.z*sin(theta);
			pw.y = p.y;
			pw.z = p.x*sin(theta) + p.z*cos(theta);
			break;

		case(zAxis):
			theta = p.z * m_warpFactor;
			pw.x = p.x*cos(theta) - p.y*sin(theta);
			pw.y = p.x*sin(theta) + p.y*cos(theta);
			pw.z = p.z;
			break;
		}

		return pw;
	}

	float fieldValue(vec3f p)
	{		
		if(m_children[0]->isOperator())
		{
			//Goto Warp Space Directly!
			p = warp(p);
			return m_children[0]->fieldValue(p);			
		}
		else
		{
			CSkeletonPrimitive* sprim = reinterpret_cast<CSkeletonPrimitive*>(m_children[0]);
			//Goto Normal Space
			p = sprim->getTransform().applyBackwardTransform(p);

			//Goto Warp Space
			p = warp(p);
			float dd = sprim->getSkeleton()->squareDistance(p) * sprim->getScaleInv2();
			return sprim->getFieldFunction()->fieldValueSquare(dd);
		}
	}

	float curvature(vec3f p)
	{
		p = warp(p);
		return m_children[0]->curvature(p);
	}

	vec4f baseColor(vec3f p)
	{		
		return m_children[0]->baseColor(warp(p));
	}

	CMaterial baseMaterial(vec3f p)
	{
		p = warp(p);
		return m_children[0]->baseMaterial(p);
	}


	void getName(char * chrName)
	{
		strcpy_s(chrName, MAX_NAME_LEN, "TWIST");		
	}


	COctree computeOctree()
	{
		m_octree = m_children[0]->getOctree();			
		m_octree.expand(BOUNDING_OCTREE_EXPANSION_FACTOR);
		return m_octree;
	}


	bool isOperator() { return true;}
	BlobNodeType getNodeType() {return bntOpWarpTwist;}
};
}
}
#endif