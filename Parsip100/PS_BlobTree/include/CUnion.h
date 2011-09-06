#ifndef CUNION_H
#define CUNION_H

#include <math.h>
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

class  CUnion : public CBlobTree
{
public:
	CUnion() {;}
	CUnion(CBlobTree * child)
	{
		addChild(child);
	}

	CUnion(CBlobTree * child1, CBlobTree * child2)
	{
		addChild(child1);
		addChild(child2);
	}

	CUnion(CBlobTree * child1, CBlobTree * child2, CBlobTree * child3)
	{
		addChild(child1);
		addChild(child2);
		addChild(child3);
	}

	CUnion(BLOBTREECHILDREN children)
	{
		addChild(children);
	}

	float fieldValue(vec3f p)
	{
		float result = 0.0f;
		for(size_t i=0; i < m_children.size(); i++)
		{
			result = max(result, m_children[i]->fieldValue(p));
		}
		return result;
	}

	float curvature(vec3f p)
	{
		CBlobTree * maxNode = getChildMax(p);
		if(maxNode)
			return maxNode->curvature(p);
		else
			return 0.0f;
	}

	vec4f baseColor(vec3f p)
	{		
		CBlobTree * maxNode = getChildMax(p);
		if(maxNode)
			return maxNode->baseColor(p);
		else
			return m_children[0]->baseColor(p);
	}

	CMaterial baseMaterial(vec3f p)
	{
		CBlobTree * maxNode = getChildMax(p);
		if(maxNode)
			return maxNode->baseMaterial(p);
		else
			return m_children[0]->baseMaterial(p);
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

	CBlobTree* operator |(CBlobTree* other)
	{
		CUnion * result = new CUnion(this, other);
		return result;
	}

	void getName(char * chrName)
	{
		strcpy_s(chrName, MAX_NAME_LEN, "UNION");		
	}

	//Octree manipulation
	COctree computeOctree()
	{
		m_octree = m_children[0]->getOctree();
		for(size_t i = 1; i < m_children.size(); i++)
			m_octree.csgUnion(m_children[i]->getOctree());		

		return m_octree;
	}


	bool isOperator() { return true;}
	BlobNodeType getNodeType() {return bntOpUnion;}
};

}
}

#endif

