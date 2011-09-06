#ifndef CDIFFERENCE_H
#define CDIFFERENCE_H

#include "math.h"
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

class  CDifference : public CBlobTree
{
public:
	CDifference() {;}
	CDifference(CBlobTree * child)
	{
		addChild(child);
	}

	CDifference(CBlobTree * child1, CBlobTree * child2)
	{
		addChild(child1);
		addChild(child2);
	}

	CDifference(CBlobTree * child1, CBlobTree * child2, CBlobTree * child3)
	{
		addChild(child1);
		addChild(child2);
		addChild(child3);
	}

	CDifference(BLOBTREECHILDREN children)
	{
		addChild(children);
	}

	float fieldValue(vec3f p)
	{
		float result = m_children[0]->fieldValue(p);
		for(size_t i=1; i < m_children.size(); i++)
		{
			result = min(result, MAX_FIELD_VALUE - m_children[i]->fieldValue(p));
		}
		return result;
	}

	float curvature(vec3f p)
	{
		CBlobTree * minNode = getChildMinDif(p);
		if(minNode)
			return minNode->curvature(p);
		else
			return 0.0f;
	}

	vec4f baseColor(vec3f p)
	{		
		CBlobTree * minNode = getChildMinDif(p);
		if(minNode)
			return minNode->baseColor(p);
		else
			return m_children[0]->baseColor(p);
	}

	CMaterial baseMaterial(vec3f p)
	{
		CBlobTree * minNode = getChildMinDif(p);
		if(minNode)
			return minNode->baseMaterial(p);
		else 
			return m_children[0]->baseMaterial(p);
	}

	CBlobTree * getChildMinDif(vec3f p)
	{
		if(countChildren() == 0)
			return NULL;

		float minField = m_children[0]->fieldValue(p);
		size_t iMin = 0;
		float curField;
		for(size_t i = 1; i < m_children.size(); i++)
		{
			curField = MAX_FIELD_VALUE - m_children[i]->fieldValue(p);
			if(curField < minField)
			{
				minField = curField;
				iMin = i;
			}
		}
		return m_children[iMin];
	}

	void getName(char * chrName)
	{
		strcpy_s(chrName, MAX_NAME_LEN, "DIFFERENCE");				
	}

	COctree computeOctree()
	{
		m_octree = m_children[0]->getOctree();		
		return m_octree;
	}


	bool isOperator() { return true;}
	BlobNodeType getNodeType() {return bntOpDif;}
};

}
}
#endif
