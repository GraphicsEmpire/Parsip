#ifndef CBLOBTREENODE_H
#define CBLOBTREENODE_H

#include "PS_FrameWork/include/PS_Material.h"
#include "PS_FrameWork/include/PS_Vector.h"
#include "PS_FrameWork/include/PS_Matrix.h"
#include "PS_FrameWork/include/PS_Quaternion.h"
#include "PS_FrameWork/include/_dataTypes.h"
#include "PS_FrameWork/include/PS_Octree.h"
#include "PS_FrameWork/include/PS_AffineTransformation.h"
#include "PS_FrameWork/include/PS_Lock.h"
#include "DSystem/include/DContainers.h"

#include "_constSettings.h"
#include "CVolumeBox.h"


using namespace std;
using namespace PS::MATH;
using namespace PS::BLOBTREE::Vol;

namespace PS{
namespace BLOBTREE{

enum MajorAxices {xAxis, yAxis, zAxis};
enum WarpOrder {woLinear, woQuadratic};
enum BlobNodeType{bntPrimSkeleton, bntPrimQuadricPoint, bntPrimFastQuadraticPointSet, bntPrimHalfPlane,
                  bntOpUnion, bntOpIntersect, bntOpDif, bntOpSmoothDif, bntOpBlend, bntOpRicciBlend,
                  bntOpAffine, bntOpWarpTwist, bntOpWarpTaper, bntOpWarpBend, bntOpWarpShear, bntOpCache, bntOpTexture, bntOpPCM};

//This will help for managing different properties that each BlobTree Node exposes
class CBlobTree 
{
protected:
	int		  m_id;
	DVec<CBlobTree*> m_children;
	COctree	  m_octree;
	CMaterial m_material;
	vec4f	  m_color;	
	bool      m_bDeleteChildrenUponCleanup;

	CEditLock m_lock;
	CAffineTransformation m_transform;

public:
	CBlobTree() 
	{		
		m_octree.lower.zero();
		m_octree.upper.zero();		
		m_id = 0;
		m_bDeleteChildrenUponCleanup = true;
	}
	
	~CBlobTree()
	{
		if(m_bDeleteChildrenUponCleanup)
			removeAllChildren();	
	}

	void setDeleteChildrenUponCleanup(bool bDelete) {m_bDeleteChildrenUponCleanup = bDelete;}
	CEditLock& getLock() {return m_lock;}
	CAffineTransformation& getTransform() {return m_transform;}
	void setID(int id) {m_id = id;}
	int getID() const {return m_id;}
	
	COctree getOctree() 
	{
		if(m_octree.isValid())
			return m_octree;
		else
			return computeOctree();
	}

	void setOctree(vec3f lo, vec3f hi)
	{
		m_octree.lower = lo;
		m_octree.upper = hi;
	}

	//Remove Child from index
	void removeChild(int index)
	{
		if(isChildIndex(index))
		{
			CBlobTree* child = m_children[index];
			SAFE_DELETE(child);
			m_children.erase(m_children.begin() + index);
		}
	}

	//Remove all Children
	void removeAllChildren()
	{
		size_t ctChildren = m_children.size();		
		for(size_t i=0; i < ctChildren; i++)			
		{
			delete m_children[i];
		}
		m_children.clear();
	}

	//Detach Child by its index
	//Detach won't free the object
	void detachChild(int index)
	{	
		if(isChildIndex(index))
		{
			m_children.detach(m_children.begin() + index);
		}
	}

	int getChildIndex(CBlobTree* kid)
	{
		size_t ctChildren = m_children.size();		
		for(size_t i=0; i < ctChildren; i++)			
		{
			if(m_children[i] == kid)
				return (int)i;
		}
		return -1;
	}

	//Manage Color
	vec4f getColor() const {return m_color;}
	void setColor(vec4f color) {m_color = color;}

	//Manage Material
	CMaterial getMatrial() const { return m_material;}
	void setMaterial(CMaterial mtrl) { m_material = mtrl;}

	bool hasChildren() {return (m_children.size() > 0);}

	//////////////////////////////////////////////////////////////////////////
	int recursive_CountPrimitives()
	{
		int count = (isOperator() == false)?1:0;
		for(size_t i =0; i < this->m_children.size(); i++)
		{
			count += m_children[i]->recursive_CountPrimitives();
		}
		return count;
	}

	int recursive_CountOperators()
	{
		int count = (isOperator())?0:1;
		for(size_t i =0; i < this->m_children.size(); i++)
		{
			count += m_children[i]->recursive_CountOperators();
		}
		return count;
	}

	int recursive_CountAllNodes()
	{
		int count = 1;
		for(size_t i =0; i < this->m_children.size(); i++)
		{
			count += m_children[i]->recursive_CountAllNodes();
		}
		return count;
	}

	int recursive_Depth()
	{
		if(hasChildren())
		{
			int depth = 0;
			for(size_t i=0; i < this->m_children.size(); i++)
				depth = MATHMAX(depth, m_children[i]->recursive_Depth());
			return depth + 1;
		}
		else
			return 0;
	}
	//////////////////////////////////////////////////////////////////////////
	size_t countChildren() const { return m_children.size();}
	CBlobTree * getFirstChild() const
	{
		if(m_children.size() > 0)
			return m_children[0];
		else
			return NULL;
	}

	CBlobTree * getLastChild() const
	{
		if(m_children.size() > 0)
			return m_children[m_children.size() - 1];
		else
			return NULL;
	}

	CBlobTree * getChild(size_t index)
	{
                if(index < countChildren())
			return m_children[index];
		else 
			return NULL;
	}

	DVec<CBlobTree*>& getChildren() {return m_children;}

	void addChild(CBlobTree * child)
	{
		if(child == NULL) return;
		m_children.push_back(child);
	}

	void addChild(const DVec<CBlobTree *> & children)
	{
		if(children.size() == 0) return;

		size_t count = children.size();
		for(size_t i=0; i< count; i++)
			m_children.push_back(children[i]);
	}

	bool isChildIndex(size_t index)
	{
                return (index < m_children.size());
	}
	//////////////////////////////////////////////////////////////////////////
	bool inside(vec3f p)
	{
		return (fieldValue(p) >= ISO_VALUE);		
	}

	
	float fieldValue(float x, float y, float z)
	{
		return fieldValue(vec3f(x,y,z));
	}
	
	/**
      * Computes gradient of the model at point p, uses delta for forward differencing,
	  * Six field evaluations will be performed.
      * @param p the point to find gradient for
      * @param delta forward differencing 
      * @return gradient vector which is NOT normalized
      */
	vec3f gradient(vec3f p, float delta)
	{
		//Refer to page 5 of BlobTree paper for this numerical approximation of gradient
		float x = p.x; float y = p.y; float z = p.z;
		float invTwoDelta = 1 / (2.0f * delta);
		float fx = invTwoDelta * (fieldValue(x + delta, y, z) - fieldValue(x - delta, y, z));
		float fy = invTwoDelta * (fieldValue(x, y + delta, z) - fieldValue(x, y - delta, z));
		float fz = invTwoDelta * (fieldValue(x, y, z + delta) - fieldValue(x, y, z - delta));
		
		return vec3(fx, fy, fz);
	}

	/**
      * Computes gradient of the model at point p, uses delta and passed fieldvalue for forward differencing,
	  * Three field evaluations will be performed.
      * @param p the point to find gradient for
      * @param delta forward differencing param
	  * @param inFieldValue Computed fieldvalue at point p
      * @return gradient vector which is NOT normalized
      */
	vec3f gradient(vec3f p, float delta, float inFieldValue)
	{
		float fx = (fieldValue(p.x + delta, p.y, p.z) - inFieldValue)/delta;
		float fy = (fieldValue(p.x, p.y + delta, p.z) - inFieldValue)/delta;
		float fz = (fieldValue(p.x, p.y, p.z + delta) - inFieldValue)/delta;

		return vec3(fx, fy, fz);
	}


	/**
      * Computes fieldvalue and normal of the model at point p, uses delta for optimized forward differencing,
	  * Four field evaluations will be performed.
      * @param p the point to find gradient for
      * @param delta forward differencing 
	  * @param outNormal Output param for normalized inverted gradient
	  * @param outField Output param for computed fieldvalue at point p
      * @return 4 which is the number of field evaluations
      */
	int fieldValueAndNormal(vec3f p, float delta, vec3f &outNormal, float &outField)
	{
		vec3f n;
		float fp = fieldValue(p);
		n.x = (fieldValue(p.x + delta, p.y, p.z) - fp)/delta;
		n.y = (fieldValue(p.x, p.y + delta, p.z) - fp)/delta;
		n.z = (fieldValue(p.x, p.y, p.z + delta) - fp)/delta;
		n.normalize();    

		outNormal = -1.0f * n;		
		outField = fp;
		return 4;
	}

	vec3f normal(vec3f p)
	{
		return normal(p, NORMAL_DELTA);
	}
	 
	vec3f normal(vec3f p, float delta)
	{
		float fp = fieldValue(p);
		return normal(p, fp, delta);
	}

	/**
      * Computes normal of the model at point p, uses delta for optimized forward differencing,
	  * Three field evaluations will be performed.
      * @param p the point to find normal for
      * @param delta forward differencing 
      * @return normal which is the normalized negated gradient
      */
	vec3f normal(vec3f p, float inFieldValue, float delta)
	{
		vec3f n;		
		n.x = (fieldValue(p.x + delta, p.y, p.z) - inFieldValue)/delta;
		n.y = (fieldValue(p.x, p.y + delta, p.z) - inFieldValue)/delta;
		n.z = (fieldValue(p.x, p.y, p.z + delta) - inFieldValue)/delta;
		n.normalize();    				
		n = -1.0f * n;
		return n;
	}
	//////////////////////////////////////////////////////////////////////////	
	/**
      * Computes fieldvalue and gradient of the model at point p, uses delta for optimized forward differencing,
	  * Four field evaluations will be performed.
      * @param p the point to find gradient for
      * @param delta forward differencing 
	  * @param outGradient Output param for Un-normalized gradient
	  * @param outField Output param for computed fieldvalue at point p
      * @return 4 which is the number of field evaluations
      */
	virtual int fieldValueAndGradient(vec3 p, float delta, vec3 &outGradient, float &outField)
	{
		float fp = fieldValue(p);
		float fx = (fieldValue(p.x + delta, p.y, p.z) - fp)/delta;
		float fy = (fieldValue(p.x, p.y + delta, p.z) - fp)/delta;
		float fz = (fieldValue(p.x, p.y, p.z + delta) - fp)/delta;

		outGradient.set(fx, fy, fz);
		outField = fp;

		return 4;
	}

	//Following methods are required to be implemented by all inherited BlobTrees
	virtual float fieldValue(vec3f p) = 0;

	virtual float curvature(vec3f p) {return p.x+p.y+p.z;}
	
	virtual COctree computeOctree() = 0;

	//Material weighted by fieldvalue at point p
	virtual CMaterial baseMaterial(vec3f p) = 0;

	//Color weighted by fieldvalue at point p
	virtual vec4f baseColor(vec3f p) = 0;

	virtual void getName(char * chrName) = 0;

	virtual bool isOperator() = 0;

	virtual BlobNodeType getNodeType() = 0;
};


typedef DVec<CBlobTree *> BLOBTREECHILDREN;

}
}

#endif
