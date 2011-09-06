#ifndef CBLEND_H
#define CBLEND_H

#include <math.h>
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

class  CBlend : public CBlobTree
{

public:
	CBlend() {;}
	CBlend(CBlobTree * child)
	{
		addChild(child);
	}

	CBlend(CBlobTree * child1, CBlobTree * child2)
	{
		addChild(child1);
		addChild(child2);
	}

	CBlend(CBlobTree * child1, CBlobTree * child2, CBlobTree * child3)
	{
		addChild(child1);
		addChild(child2);
		addChild(child3);
	}

	CBlend(BLOBTREECHILDREN children)
	{
		addChild(children);
	}

	float fieldValue(vec3f p)
	{		
		float result = m_children[0]->fieldValue(p);	

		for(size_t i=1; i < m_children.size(); i++)
		{
			result += m_children[i]->fieldValue(p);			
		}		
		return result;
	}

	float curvature(vec3f p)
	{
		float sum = 0.0f;
		float c = 0.0f;
		float f = 0.0f;
		float curvature;

		for(size_t i=0; i < m_children.size(); i++)
		{
			f = m_children[i]->fieldValue(p);
			if( f > 0.0f)
			{
				curvature = m_children[i]->curvature(p);
				c += curvature * curvature * f;
				sum += f;
			}
		}

		if( sum == 0.0f)
			return m_children[0]->curvature(p);
		else 
			return sqrt(c  / sum);
	}

	vec4f baseColor(vec3f p)
	{		
		vec4f color;
		float f = 0.0f;
		float sum = 0.0f;
		for(size_t i = 0; i < m_children.size(); i++)
		{
			f = m_children[i]->fieldValue(p);
			if(f > 0.0f)
			{
				color += m_children[i]->baseColor(p) * f;
				sum += f;
			}
		}

		if(sum == 0.0f)
			return m_children[0]->baseColor(p);
		else
			return color*static_cast<float>(1.0f /(float)sum);

	}

	CMaterial baseMaterial(vec3f p)
	{
		float sum = 0.0f;
		CMaterial m;
		float f = 0.0f;

		for(size_t i = 0; i < m_children.size(); i++)
		{
			f = m_children[i]->fieldValue(p);
			if(f > 0.0f)
			{
				m += m_children[i]->baseMaterial(p) * f;
				sum += f;
			}
		}

		if(sum == 0.0f)
			return m_children[0]->baseMaterial(p);
		else
			return m*static_cast<float>(1.0f /(float)sum);
	}

	CBlobTree * getChildMax(vec3f p)
	{
		if(countChildren() == 0)
			return NULL;

		float maxField = m_children[0]->fieldValue(p);
		size_t iMax = 0;
		float curField;
		for(size_t i = 1; i < m_children.size(); i++)
		{
			curField = m_children[i]->fieldValue(p);
			if(curField > maxField)
			{
				maxField = curField;
				iMax = i;
			}
		}
		return m_children[iMax];
	}

	CBlobTree* operator +(CBlobTree* other)
	{		
		CBlend * blend = new CBlend(this, other);
		return blend;
	}

	void getName(char * chrName)
	{
		strcpy_s(chrName, MAX_NAME_LEN, "BLEND");		
	}

	COctree computeOctree()
	{
		if(m_children.size() == 0) return m_octree;
		m_octree = m_children[0]->getOctree();
		for(size_t i = 1; i < m_children.size(); i++)
			m_octree.csgUnion(m_children[i]->getOctree());				
		return m_octree;
	}

	bool isOperator() { return true;}

	BlobNodeType getNodeType() {return bntOpBlend;}
};
}
}

#endif