#pragma once
#ifndef CAFFINE_H
#define CAFFINE_H

#include <math.h>
#include "CBlobTree.h"

namespace PS{
namespace BLOBTREE{

class CAffine : public CBlobTree
{
private:
	//Forward
	vec3f m_scale;
	CQuaternion m_rotate;	
	vec3f m_translate;

	//Reverse
	vec3f m_revScale;
	CQuaternion m_revRotate;	
	vec3f m_revTranslate;

	CMatrix m_mtxForward;
	CMatrix m_mtxBackward;

public:
        enum AffineTransformationType {attTranslate, attRotate, attScale};
        enum AffineDirection {adForward, adReverse};

	CAffine() 
	{
		init();		
	}

	CAffine(CBlobTree * child)
	{
		init();
		addChild(child);		
	}

	CAffine(CBlobTree * child, vec3f scale, CQuaternion rot, vec3f translate)
	{
		init();
		addChild(child);		
		m_scale = scale;
		m_rotate = rot;
		m_translate = translate;		
		setReverseTransformations();
	}

	void init()
	{		
		m_scale = vec3f(1, 1, 1);
		m_revScale = vec3f(1, 1, 1);
		m_translate = vec3f(0, 0, 0);		
		m_revTranslate = vec3f(0, 0, 0);
		m_rotate.identity();
		m_revRotate.identity();

		m_mtxBackward.identity();
		m_mtxBackward.identity();
		//m_bTranslate = m_bRotate = m_bScale = false;
	}

	void setParamsFrom(CBlobTree* input)
	{
		CAffine* aff = dynamic_cast<CAffine*>(input);
		this->m_scale = aff->m_scale;
		this->m_translate = aff->m_translate;
		this->m_rotate	  = aff->m_rotate;		
		this->setReverseTransformations();
	}

	//Getters
	vec3 getTranslate(AffineDirection ad = adForward) const
	{
		if(ad == adForward)
			return m_translate;
		else
			return m_revTranslate;
	}

	vec3 getScale(AffineDirection ad = adForward) const
	{
		if(ad == adForward)
			return m_scale;
		else
			return m_revScale;
	}

	quat getRotation(AffineDirection ad = adForward) const
	{
		if(ad == adForward)
			return m_rotate;
		else
			return m_revRotate;
	}

	//Setters
	void setScale(const vec3f & scale)
	{
		m_scale = scale;
		setReverseTransformations();
	}

	void setRotation(float degree, const vec3f & axis)
	{
		m_rotate.fromAngleAxis(degree * DEG_TO_RAD, axis);
		setReverseTransformations();
	}

	void setRotation(quat q)
	{
		m_rotate = q;
		setReverseTransformations();
	}

	void setTranslate(const vec3f & translate)
	{
		m_translate = translate;	
		setReverseTransformations();
	}

	//Set reverse transforms
	void setReverseTransformations()
	{	
		//Compute reverse components
		m_revScale.set(1.0f / m_scale.x, 1.0f / m_scale.y, 1.0f / m_scale.z);	
		m_revRotate = m_rotate.inverse();						
		m_revTranslate.set(-1.0f * m_translate.x, -1.0f * m_translate.y, -1.0f * m_translate.z);

		CMatrix mtxRot;
		m_mtxForward.identity();

		//Translate to origin
		m_mtxForward.setTranslate(m_translate * -1.0f);

		//Perform SRT transformation
		m_mtxForward.setScale(m_scale);
		
		m_rotate.toMatrix(mtxRot);
		m_mtxForward.multiply(mtxRot);
		m_mtxForward.setTranslate(m_translate);

		m_mtxForward.invert(m_mtxBackward);		
	}

	//Forward Transforms
	vec3f applyForwardTransform(vec3f v)
	{
		return m_mtxForward.transform(v);		
	}

	vec3f applyForwardRotate(vec3f v)
	{
		return m_rotate.transform(m_revRotate, v);		
	}

	CMatrix getForwardRotate()
	{
		CMatrix m;
		m_rotate.toMatrix(m);		
		return m;
	}

	CMatrix getBackwardRotate()
	{
		CMatrix m;
		m_revRotate.toMatrix(m);
		return m;
	}

	CMatrix& getForwardMatrix()  {	return m_mtxForward;}
	CMatrix& getBackwardMatrix() {	return m_mtxBackward;}

	//Backward Transforms	
	vec3f applyBackwardTransform(vec3f v)
	{			
		return m_mtxBackward.transform(v);		
	}

	vec3f applyBackwardRotate(vec3f v)
	{
		return m_revRotate.transform(m_rotate, v);		
	}
	
	vec3f getNormalChild(vec3f p)
	{
		return m_children[0]->normal(p);
	}

	vec3f getNormalScale(vec3f p)
	{
		return m_children[0]->normal(p) * m_scale;
	}

	vec3f getNormalRotate(vec3f p)
	{
		return m_rotate.transform(m_children[0]->normal(p));
	}

	vec3f getNormalRotateScale(vec3f p)
	{
		return m_rotate.transform(m_children[0]->normal(p)) * m_scale;
	}

	float fieldValue(vec3f p)
	{			
		vec3f transformed = applyBackwardTransform(p);
		float result = m_children[0]->fieldValue(transformed);
		return result;
	}

	vec3f normal(vec3f p, float delta)
	{
		vec3f transformed = applyBackwardTransform(p);
		return m_children[0]->normal(p, delta);
	}

	float curvature(vec3f p)
	{
		if(FLOAT_EQ(m_scale.sum(), 0.0f, EPSILON))
			return (m_children[0]->curvature(p));
		else
			return (m_children[0]->curvature(p) * (3.0f / m_scale.sum()));
	}

	vec4 baseColor(vec3f p)
	{
		return m_children[0]->baseColor(applyBackwardTransform(p));
	}


	CMaterial baseMaterial(vec3f p)
	{
		return m_children[0]->baseMaterial(applyBackwardTransform(p));
	}

	void getName(char * chrName)
	{
            strncpy(chrName, "AFFINE", MAX_NAME_LEN);
	}

	COctree computeOctree()
	{
		m_octree = m_children[0]->getOctree();
		m_octree.transform(m_mtxForward);		
		return m_octree;
	}


	bool isOperator() { return true;}
	BlobNodeType getNodeType() {return bntOpAffine;}
};

}
}

#endif
