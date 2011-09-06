#include "CSkeletonPrimitive.h"

namespace PS{
	namespace BLOBTREE{
		bool CSkeletonPrimitive::findSeedPoint(bool bFindHot, float iso_value, vec3f& p, float& fp)
		{
			if(bFindHot == (fp > iso_value)) return true;
			if(fp == 0.0f) return false;


			vec3f c = p;
			float fc = fp;		
			vec3f grad;		
			float travel;
			//float delta;
			float search_step = 0.001f;

			grad = this->gradient(c, NORMAL_DELTA, fc);
			if(bFindHot)
			{
				while(fc < iso_value)
				{		
					c  += search_step*grad;

					this->fieldValueAndGradient(c, NORMAL_DELTA, grad, fc);
					if(fp != fc)
					{
						travel = (c - p).length();
						search_step = fabsf((iso_value - fc)*travel / (fp - fc));
					}

					p = c;
					fp = fc;			
				}
			}
			else
			{
				//Find cold
				while(fc > iso_value)
				{		
					c  += search_step*(-1.0f)*grad;

					this->fieldValueAndGradient(c, NORMAL_DELTA, grad, fc);
					if(fp != fc)
					{
						travel = (c - p).length();
						search_step = fabsf((iso_value - fc)*travel / (fp - fc));
					}

					p = c;
					fp = fc;			
				}
			}
			

			return true;		
		}


		bool CSkeletonPrimitive::findSeedPoint(bool bFindHot, float iso_value, float search_step, vec3f search_dir, vec3f& p, float& fp)
		{
			if(bFindHot == (fp > iso_value)) return true;
			if(fp == 0.0f) return false;

			int iStep = 1;
			while(bFindHot != (fp > iso_value))
			{			
				p  += static_cast<float>(iStep)*search_step*search_dir;
				fp = fieldValue(p);				
				search_step *= 1.3f;
				iStep++;			
			}

			return true;
		}


		COctree CSkeletonPrimitive::computeOctree()
		{
			static const float target_field = ISO_VALUE - ISO_VALUE_EPSILON;
			float init_step_width = 0.0001f;

			vec3 dir[6];		
			dir[0] = vec3(-1.0f, 0.0f, 0.0f);
			dir[1] = vec3(0.0f, -1.0f, 0.0f);
			dir[2] = vec3(0.0f, 0.0f, -1.0f);
			dir[3] = vec3(1.0f, 0.0f, 0.0f);
			dir[4] = vec3(0.0f, 1.0f, 0.0f);
			dir[5] = vec3(0.0f, 0.0f, 1.0f);


			CMatrix mtxForwardRot = m_transform.getForwardRotate();
			//Transform Directions to the new warped space		
			for (int i=0; i<6; i++)
			{			
				dir[i] = mtxForwardRot.transform(dir[i]);
				dir[i].normalize();
			}		

			//First get the extremes of the skeleton
			//If the extreme is well-defined then compute the bounding octree	
			//From extremes otherwise use searching method
			vec3f lo, hi;
			vec3f p;
			float fp;						
			vec3f region[6];

			
			if(m_skeleton->getType() == sktRing)
			{
				CSkeletonRing* sring = reinterpret_cast<CSkeletonRing*>(m_skeleton);

				vec3f p = sring->getPosition();
				vec3f d = sring->getDirection();
				float radius = sring->getRadius();
	

				int set[6];
				float arrFields[6];
				vec3f searchDir[6];

				for(int i=0;i<6; i++)
					set[i] = 0;

				vec3f invRingDir = vec3f(ZERO_CLAMP(cos(d.x * PiOver2)), ZERO_CLAMP(cos(d.y * PiOver2)), ZERO_CLAMP(cos(d.z * PiOver2)));				
			

				for(int i=0; i<3; i++)
				{					
					if(invRingDir[i])
					{
						searchDir[i + 0] = vec3f(0.0f, 0.0f, 0.0f);
						searchDir[i + 3] = vec3f(0.0f, 0.0f, 0.0f);
						searchDir[i + 0][i] = invRingDir[i];
						searchDir[i + 3][i] = -invRingDir[i];

						region[i + 0] = m_transform.applyForwardTransform(p + radius*searchDir[i]);
						region[i + 3] = m_transform.applyForwardTransform(p - radius*searchDir[i]);

						//Rotate direction with matrix						
						searchDir[i + 0] = mtxForwardRot.transform(searchDir[i + 0]);
						searchDir[i + 0].normalize();
						searchDir[i + 3] = mtxForwardRot.transform(searchDir[i + 3]);
						searchDir[i + 3].normalize();					


						set[i + 0] = 1;
						set[i + 3] = 1;
					}
				}
			
				//Now let's do
				//Root finder to converge to the point at target_field
				CRootFinder *rootFinder = new CRootFinder(this, rfmBisection, DEFAULT_ITERATIONS, target_field);						

				vec3 arrPoints[6];
				int ctPoints = 0;
				float maxDisplaced = 0.0f;
				float fpCold, fpHot;
				vec3f cold, hot, onSkeleton;
				for(int i=0;i<6; i++)
				{
					if(set[i])
					{
						onSkeleton = region[i];
						arrFields[i] = this->fieldValue(region[i]);

						//Find Hot
						if(arrFields[i] < target_field)
							findSeedPoint(true, target_field + ISO_VALUE_EPSILON, region[i], arrFields[i]);

						//Keep original point and field value
						hot = region[i];
						fpHot = arrFields[i];
						
						//Find Cold
						if(findSeedPoint(false, target_field, init_step_width, searchDir[i], region[i], arrFields[i]))
						{
							cold = region[i];
							fpCold = arrFields[i];
							rootFinder->findRoot(hot, cold, fpHot, fpCold, region[i], arrFields[i]);			
							
							arrPoints[ctPoints] = region[i];
							ctPoints++;
							float dist = onSkeleton.distance(region[i]);
							if(dist > maxDisplaced)
								maxDisplaced = dist;
						}

					}
				}

				SAFE_DELETE(rootFinder);

				m_octree.set(arrPoints, ctPoints);
				m_octree.expand(vec3f(maxDisplaced + 0.1f, maxDisplaced + 0.1f, maxDisplaced + 0.1f));
			}
			else			
			{				
				vec3f c  = getPolySeedPoint();
				//vec3f c  = m_skeleton->getPolySeedPoint();
				float fc = this->fieldValue(c);	

				//fc should be greater than the target_field or we are not gonna intersect with the surface
				if(fc < target_field)
					findSeedPoint(true, target_field + ISO_VALUE_EPSILON, c, fc);

				//Root finder to converge to the point at target_field
				CRootFinder *rootFinder = new CRootFinder(this, rfmBisection, DEFAULT_ITERATIONS, target_field);						

				//Now search for a point with field-value less than target_field
				for(int i=0; i<6; i++)
				{
					//Init step width and start position
					p = c;
					fp = fc;

					if(findSeedPoint(false, target_field, init_step_width, dir[i], p, fp))						
						rootFinder->findRoot(c, p, fc, fp, p, fp);

					//Region
					region[i] = p;
				}
				//=============================================
				SAFE_DELETE(rootFinder);

				//Now we have origin points
				m_octree.set(region, 6);
				m_octree.expand(vec3f(0.3f, 0.3f, 0.3f));
			}

			m_bOctreeValid = m_octree.isValid();
			return m_octree;
		}


	}

}