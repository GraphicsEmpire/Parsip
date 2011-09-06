#ifndef CBLOBTREE_ANIMATION_H
#define CBLOBTREE_ANIMATION_H

#include "PS_BlobTree/include/CBlobTree.h"
#include "PS_FrameWork/include/PS_SplineCatmullRom.h"

using namespace PS;
using namespace PS::BLOBTREE;

#define DEFAULT_SEL_RADIUS 0.3f

namespace PS{

namespace BLOBTREEANIMATION{

class CAnimObject
{
public:
	CAnimObject(CBlobTree* lpModel)
	{
		model			= lpModel;
		path			= new CSplineCatmullRom();
		bTranslate		= true;
		bScale			= false;
		startVal		= vec4f(1.0f, 1.0f, 1.0f, 1.0f);
		endVal			= vec4f(1.0f, 1.0f, 1.0f, 1.0f);
		idxSelCtrlPoint = -1;
	}

	~CAnimObject()
	{
		model = NULL;
		SAFE_DELETE(path);
	}

	void advance(float animTime);

	void gotoStart();
	void gotoEnd();

	void drawPathCtrlPoints();
	void drawPathCurve();

public:
	CBlobTree* model;
	CSplineCatmullRom* path;
	bool bTranslate;
	bool bScale;
	vec4f startVal;
	vec4f endVal;	
	int idxSelCtrlPoint;
};

//////////////////////////////////////////////////////////////////////////
/**
*	Manages animation of several BlobTree objects.
*/
class CAnimManager
{
private:
	DVec<CAnimObject*> m_lstObjects;
	float m_selRadius;
	int m_idxSelObject;

public:
	CAnimManager() {
		m_idxSelObject = -1;
		m_selRadius = DEFAULT_SEL_RADIUS;
	}

	~CAnimManager()
	{
		removeAll();
	}

	size_t countObjects() const { return m_lstObjects.size();}
	CAnimObject* getObject(CBlobTree* root);
	CAnimObject* getObject(int index);


	int queryHitPathOctree(const PS::MATH::CRay& ray, float t0, float t1);
	bool queryHitPathCtrlPoint(const PS::MATH::CRay& ray, float t0, float t1, int& idxPath, int& idxCtrlPoint);
	void queryHitSetSelRadius(float radius) {m_selRadius = radius;}
	void queryHitResetAll();
	
	void setActiveObject(int index);
	CAnimObject* getActiveObject();
	bool hasSelectedCtrlPoint();

	void addModel(CBlobTree* lpModel);
	bool removeModel(CBlobTree* lpModel);
	bool remove(int index);
	void removeAll();

	void advanceAnimation(float animTime);

	static CAnimManager* GetAnimManager();

	CAnimObject* operator[](const int index) const 
	{
		if(m_lstObjects.isItemIndex((size_t)index))
			return m_lstObjects[index];
		else 
			return NULL;	
	}
protected:
	static CAnimManager* sm_pAnimManager;
};

}
}
#endif