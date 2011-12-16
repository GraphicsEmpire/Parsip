#ifndef CUNION_H
#define CUNION_H

#include <math.h>
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

class  CUnion : public CBlobNode
{
public:
	CUnion() {;}
	CUnion(CBlobNode * child)
	{
		addChild(child);
	}

	CUnion(CBlobNode * child1, CBlobNode * child2)
	{
		addChild(child1);
		addChild(child2);
	}

	CUnion(CBlobNode * child1, CBlobNode * child2, CBlobNode * child3)
	{
		addChild(child1);
		addChild(child2);
		addChild(child3);
	}

	CUnion(BLOBNODECHILDREN children)
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
		CBlobNode * maxNode = getChildMax(p);
		if(maxNode)
			return maxNode->curvature(p);
		else
			return 0.0f;
	}

	CBlobNode * getChildMax(vec3f p)
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

        string getName()
	{
            return "UNION";
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

