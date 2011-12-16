#ifndef CSMOOTHDIFFERENCE_H
#define CSMOOTHDIFFERENCE_H

#include "math.h"
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

class  CSmoothDifference : public CBlobNode
{
private:
	bool m_bCombineMaterials;
public:
	CSmoothDifference() {;}
	CSmoothDifference(CBlobNode * child)
	{
		addChild(child);
	}

	CSmoothDifference(CBlobNode * child1, CBlobNode * child2)
	{
		addChild(child1);
		addChild(child2);
	}

	CSmoothDifference(CBlobNode * child1, CBlobNode * child2, CBlobNode * child3)
	{
		addChild(child1);
		addChild(child2);
		addChild(child3);
	}

	CSmoothDifference(CBlobNode * child, bool bCombineMaterials)
	{
		addChild(child);
		m_bCombineMaterials = bCombineMaterials;
	}

	CSmoothDifference(CBlobNode * child1, CBlobNode * child2, bool bCombineMaterials)
	{
		addChild(child1);
		addChild(child2);
		m_bCombineMaterials = bCombineMaterials;
	}

	CSmoothDifference(CBlobNode * child1, CBlobNode * child2, CBlobNode * child3, bool bCombineMaterials)
	{
		addChild(child1);
		addChild(child2);
		addChild(child3);
		m_bCombineMaterials = bCombineMaterials;
	}

	CSmoothDifference(BLOBNODECHILDREN children)
	{
		addChild(children);
	}

	bool getCombineStatus()
	{
		return m_bCombineMaterials;
	}

	void setParamFrom(CBlobNode* input)
	{
		this->m_bCombineMaterials = dynamic_cast<CSmoothDifference*>(input)->m_bCombineMaterials;
	}

	float fieldValue(vec3f p)
	{
		float result = m_children[0]->fieldValue(p);
		for(size_t i=1; i < m_children.size(); i++)
		{
			result *= MAX_FIELD_VALUE - m_children[i]->fieldValue(p);
		}
		return result;
	}

	float curvature(vec3f p)
	{
		CBlobNode * minNode = getChildMinDif(p);
		if(minNode)
			return minNode->curvature(p);
		else
			return 0.0f;
	}

	CBlobNode * getChildMinDif(vec3f p)
	{
		if(countChildren() == 0)
			return NULL;

		float minField = m_children[0]->fieldValue(p);
		size_t iMin = 0;
		float curField;
		for(size_t i = 1; i < m_children.size(); i++)
		{
			curField = 1.0f - m_children[i]->fieldValue(p);
			if(curField < minField)
			{
				minField = curField;
				iMin = i;
			}
		}
		return m_children[iMin];
	}

        string getName()
	{
            return "SMOOTH DIFFERENCE";
	}

	COctree computeOctree()
	{
		m_octree = m_children[0]->getOctree();		
		return m_octree;
	}

	bool isOperator() { return true;}

	BlobNodeType getNodeType() {return bntOpSmoothDif;}
};

}
}
#endif
