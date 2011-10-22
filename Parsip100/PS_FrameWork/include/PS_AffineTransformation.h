#pragma once
#ifndef CAFFINETRANSFORMATION_H
#define CAFFINETRANSFORMATION_H

#include <math.h>
#include "PS_Matrix.h"
#include "PS_Quaternion.h"

namespace PS{

	class CAffineTransformation
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

		CAffineTransformation() 
		{
			init();		
		}
		
		CAffineTransformation(const CAffineTransformation& rhs)
		{			
			set(rhs);
		}
	
		CAffineTransformation(vec3f scale, CQuaternion rot, vec3f translate)
		{		
			set(scale, rot, translate);
		}

		void add(const CAffineTransformation& rhs)
		{
			m_scale *= rhs.m_scale;
			m_rotate = m_rotate.multiply(rhs.m_rotate);
			m_translate += rhs.m_translate;
			setReverseTransformations();
		}

		void set(const CAffineTransformation& rhs)
		{
			m_scale = rhs.m_scale;
			m_rotate = rhs.m_rotate;
			m_translate = rhs.m_translate;
			setReverseTransformations();
		}

		void set(vec3f scale, CQuaternion rot, vec3f translate)
		{
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

		vec4f getRotationVec4(AffineDirection ad = adForward) const
		{
			quat rot;
			if(ad == adForward)
				rot = m_rotate;
			else
				rot = m_revRotate;
			return vec4f(rot.q.x, rot.q.y, rot.q.z, rot.w);
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

		//Accumulate transformations		
		void addScale(vec3f v)
		{
			setScale(m_scale + v);
		}

		void addRotate(quat rot)
		{
			setRotation(m_rotate.multiply(rot));				
		}

		void addTranslate(vec3f v)
		{
			setTranslate(m_translate + v);						
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

		CMatrix getForwardRotate() const
		{
			CMatrix m;
			m_rotate.toMatrix(m);		
			return m;
		}

		CMatrix getBackwardRotate() const
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


	};


}

#endif
