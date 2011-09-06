#ifndef CBEND_H
#define CBEND_H

#include <math.h>
#include "PS_FrameWork/include/PS_Interval.h"
#include "CBlobTree.h"

#define DEFAULT_WARP_FACTOR 1.0f

namespace PS{
namespace BLOBTREE{

using namespace PS::MATH;
//***********************************************************
//Twist 
class  CWarpBend : public CBlobTree
{
private:
	//Radians per unit length is const k
	float m_bendRate;
//	float m_bendAngle;
//	MajorAxices m_bendAxis;
//	MajorAxices m_bendAlong;

	//Value y0 is bend center
	float m_bendCenter;
	CInterval m_bendRegion;

public:
	CWarpBend() 
	{
		set(1.0f, 0.0f, 0.0f, 1.0f);
	}

	CWarpBend(CBlobTree * child)
	{
		addChild(child);
		set(1.0f, 0.0f, 0.0f, 1.0f);
	}

	CWarpBend(CBlobTree * child, float bendRate, float bendCenter, float ymin, float ymax)
	{
		addChild(child);
		set(bendRate, bendCenter, ymin, ymax);		
	}

	CWarpBend(float bendRate, float bendCenter, float ymin, float ymax)
	{		
		set(bendRate, bendCenter, ymin, ymax);		
	}

	void set(float bendRate, float bendCenter, float ymin, float ymax)
	{
		m_bendRate = bendRate;
		m_bendCenter = bendCenter;
		m_bendRegion.set(ymin, ymax);
	}

	float getBendRate() const {	return m_bendRate;}
	void setBendRate(float bendRate) { m_bendRate = bendRate;}

	float getBendCenter() const { return m_bendCenter;}
	void setBendCenter(float bendCenter) { m_bendCenter = bendCenter;}

	CInterval getBendRegion() const { return m_bendRegion;}
	void setBendRegion(CInterval region) { m_bendRegion = region;}
	void setBendRegionLeft(float left) { m_bendRegion.left = left;}
	void setBendRegionRight(float right) { m_bendRegion.right = right;}

	void setParamFrom(CBlobTree* input)
	{
		CWarpBend* bendN = dynamic_cast<CWarpBend*>(input);
		this->m_bendRate = bendN->m_bendRate;
		this->m_bendCenter = bendN->m_bendCenter;
		this->m_bendRegion = bendN->m_bendRegion;
	}

	vec3 warp(vec3 pin)
	{	
		vec3 pout;
		float k = m_bendRate;
		float kDiv = 1.0f/k;
		float y0 = m_bendCenter;
		
		
		//Compute where yhat is:
		float yh = 0.0f;
		if(pin.y <= m_bendRegion.left)
			yh = m_bendRegion.left;
		else if((pin.y > m_bendRegion.left)&&(pin.y < m_bendRegion.right))
			yh = pin.y;
		else if(pin.y >= m_bendRegion.right)
			yh = m_bendRegion.right;
	
		float theta = k*(yh - y0);
		float ct = cos(theta);
		float st = sin(theta);

		pout.x = pin.x;
		if(m_bendRegion.isInside(pin.y))		
			pout.y = -st*(pin.z - kDiv) + y0;		
		else if(pin.y < m_bendRegion.left)		
			pout.y = -st*(pin.z - kDiv) + y0 + ct*(pin.y - m_bendRegion.left);		
		else if(pin.y > m_bendRegion.right)
			pout.y = -st*(pin.z - kDiv) + y0 + ct*(pin.y - m_bendRegion.right);


		if(m_bendRegion.isInside(pin.y))		
			pout.z = ct*(pin.z - kDiv) + kDiv;		
		else if(pin.y < m_bendRegion.left)		
			pout.z = ct*(pin.z - kDiv) + kDiv + st*(pin.y - m_bendRegion.left);		
		else if(pin.y > m_bendRegion.right)
			pout.z = ct*(pin.z - kDiv) + kDiv + st*(pin.y - m_bendRegion.right);

		return pout;
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
		strcpy_s(chrName, MAX_NAME_LEN, "BEND");		
	}

	COctree computeOctree()
	{
		m_octree = m_children[0]->getOctree();			
		m_octree.expand(BOUNDING_OCTREE_EXPANSION_FACTOR);
		return m_octree;
	}


	bool isOperator() { return true;}
	BlobNodeType getNodeType() {return bntOpWarpBend;}
};

}
}
#endif