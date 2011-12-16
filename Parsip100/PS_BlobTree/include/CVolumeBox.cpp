#include "CVolumeBox.h"
#include "CVolumeSphere.h"

using namespace std;

namespace PS{
namespace BLOBTREE{
	namespace Vol{
		//Constructor with min and max position aka, lower and upper corners
		CVolumeBox::CVolumeBox(const vec3f min, const vec3f max) 
		{ 
			m_minCorner = min;
			m_maxCorner = max;
		}

		CVolumeBox::CVolumeBox(float x1, float y1, float z1, float x2, float y2, float z2)
		{
			m_minCorner.set(x1, y1, z1);
			m_maxCorner.set(x2, y2, z2);
		}

		CVolumeBox::CVolumeBox(CVolume* vol)
		{
			if(vol->isBox())
			{
				CVolumeBox* b = dynamic_cast<CVolumeBox*>(vol);
				m_minCorner = b->lower();
				m_maxCorner = b->upper();
			}
			else
			{
				CVolumeSphere* s = dynamic_cast<CVolumeSphere*>(vol);
				m_minCorner = s->center() - s->radius();
				m_maxCorner = s->center() + s->radius();
			}
		}

		vec3f CVolumeBox::center() 
		{
			vec3f res;
			res = (m_minCorner + m_maxCorner) * 0.5f;
			return res;
		}

		void CVolumeBox::set(vec3f lo, vec3f hi)
		{
			m_minCorner = lo;
			m_maxCorner = hi;
		}

		void CVolumeBox::set(float l, float b, float n, float r, float t, float f)
		{
			m_minCorner.set(l, b, n);
			m_maxCorner.set(r, t, f);
		}

		CVolumeBox* CVolumeBox::fromCenterSize(vec3f c, vec3f sz)
		{
			vec3f halfSize = 0.5f * sz;
			CVolumeBox *res = new CVolumeBox(c - halfSize, c + halfSize);
			return res;
		}

		vec3f CVolumeBox::dimensions()
		{
			return m_maxCorner - m_minCorner;
		}

		void CVolumeBox::correct()
		{
			float temp;
			if(m_minCorner.x > m_maxCorner.x)
			{
				temp = m_minCorner.x;
				m_minCorner.x = m_maxCorner.x;
				m_maxCorner.x = temp;
			}	

			if(m_minCorner.y > m_maxCorner.y)
			{
				temp = m_minCorner.y;
				m_minCorner.y = m_maxCorner.y;
				m_maxCorner.y = temp;
			}	

			if(m_minCorner.z > m_maxCorner.z)
			{
				temp = m_minCorner.z;
				m_minCorner.z = m_maxCorner.z;
				m_maxCorner.z = temp;
			}	
		}

		vec3f CVolumeBox::getCorner(int index)
		{
			vec3f res((index & 1) == 0 ? m_minCorner.x : m_maxCorner.x,
				(index & 2) == 0 ? m_minCorner.y : m_maxCorner.y, 
				(index & 4) == 0 ? m_minCorner.z : m_maxCorner.z);
			return res;
		}

		vec3f CVolumeBox::getCorner(bool bLeft, bool bBottom, bool bFront)
		{
			vec3f res(bLeft ? m_minCorner.x : m_maxCorner.x,
				bBottom ? m_minCorner.y : m_maxCorner.y,
				bFront ? m_minCorner.z : m_maxCorner.z);
			return res;
		}

		//**************************************************************
		bool CVolumeBox::isBox()
		{
			return true;
		}


		float CVolumeBox::size()
		{
			vec3f v = m_maxCorner - m_minCorner;
			return v.x * v.y * v.z;
		}

		bool CVolumeBox::isInside(vec3f pt)
		{
			if((pt.x < m_minCorner.x)||(pt.x > m_maxCorner.x))
				return false;
			if((pt.y < m_minCorner.y)||(pt.y > m_maxCorner.y))
				return false;
			if((pt.z < m_minCorner.z)||(pt.z > m_maxCorner.z))
				return false;
			return true;
		}

		CVolume* CVolumeBox::emptyVolume()
		{
			vec3f lower(0.0f, 0.0f, 0.0f);
			return new CVolumeBox(lower, lower);
		}

		bool CVolumeBox::intersect(CVolume* bv)
		{
			if(bv->isBox())
			{
				CVolumeBox b(bv);
				if ((m_minCorner.x >= b.m_maxCorner.x) || (m_maxCorner.x <= b.m_minCorner.x))
					return false;
				if ((m_minCorner.y >= b.m_maxCorner.y) || (m_maxCorner.y <= b.m_minCorner.y))
					return false;
				if ((m_minCorner.z >= b.m_maxCorner.z) || (m_maxCorner.z <= b.m_minCorner.z))
					return false;

				return true;
			}
			else
			{
				CVolumeSphere s(bv);
				float dist = 0.0f;

				float d0 = -s.center().x + s.radius() + m_maxCorner.x;
				float d1 =  s.center().x + s.radius() - m_minCorner.x;
				if (d0 <= 0.0 || d1 <= 0.0)
					return false;

				d0 = max(s.radius() - min(d0, d1), 0.0f);
				dist += d0 * d0;
				d0 = -s.center().y + s.radius() + m_maxCorner.y;
				d1 =  s.center().y + s.radius() - m_minCorner.y;
				if (d0 <= 0.0 || d1 <= 0.0)
					return false;
				d0 = max(s.radius() - min(d0, d1), 0.0f);
				dist += d0 * d0;
				d0 = -s.center().z + s.radius() + m_maxCorner.z;
				d1 =  s.center().z + s.radius() - m_minCorner.z;
				if (d0 <= 0.0 || d1 <= 0.0)
					return false;

				d0 = max(s.radius() - min(d0, d1), 0.0f);
				dist += d0 * d0;
				return (dist < s.radius2());
			}

		}

		CVolume* CVolumeBox::rotate(quat rotate)
		{
			vec3f center = this->center();
			vec3f cn(0.0f, 0.0f, 0.0f);
			for(int i=0; i< 3; i++)
				cn = vectorMax(cn, vectorAbs(rotate.transform(getCorner(i) - center))); 		

			center = rotate.transform(center);
			CVolumeBox * box = new CVolumeBox(center - cn, center + cn);
			CVolumeSphere* sphere = new CVolumeSphere(center, (upper() - lower()).length() * 0.5f);
			if (box->size() <= sphere->size())
			{
				delete sphere; sphere = NULL;
				return box;
			}
			else
			{
				delete box; box = NULL;
				return sphere;
			}
		}


		CVolume* CVolumeBox::scale(vec3f scale)
		{
			CVolumeBox* box = new CVolumeBox(lower()*scale, upper()*scale);
			return box;
		}

		CVolume* CVolumeBox::translate(vec3f trans)
		{
			CVolumeBox* box = new CVolumeBox(lower() + trans, upper() + trans);
			return box;
		}

		void CVolumeBox::tranform( const CMatrix& m )
		{
			m_minCorner = m.transform(m_minCorner);
			m_maxCorner = m.transform(m_maxCorner);
		}


		void CVolumeBox::scaleSelf(vec3f scaleVector)
		{
			m_minCorner = m_minCorner*scaleVector;
			m_maxCorner = m_maxCorner*scaleVector;
		}

		void CVolumeBox::translateSelf(vec3f translateVector)
		{
			m_minCorner = m_minCorner + translateVector;
			m_maxCorner = m_maxCorner + translateVector;
		}

		CVolume* CVolumeBox::UnionSphere(const CVolume* sphere)
		{		
			CVolumeSphere* sphere1 = dynamic_cast<CVolumeSphere*>(const_cast<CVolume*>(sphere));
			CVolumeSphere* sphere2 =  new CVolumeSphere(this);
			vec3f diff = sphere1->center() - sphere2->center();
			float l = diff.length();
			if (l == 0.0f)
			{
				float radius = max(sphere2->radius(),sphere1->radius());
				CVolumeSphere* result = new CVolumeSphere(sphere2->center(), radius);
				delete sphere2; sphere2 = NULL;
				return result;
			}
			else
			{
				CVolumeSphere* result = NULL;

				diff *= (1.0f / l);

				vec3f point1 = (diff* -sphere2->radius()) +  sphere2->center();
				vec3f point2 = diff * sphere1->radius() + sphere1->center();

				if (sphere1->isInside(point1))
					result = new CVolumeSphere(sphere1);					
				else if (sphere2->isInside(point2))
					result = new CVolumeSphere(sphere2);					
				else
				{
					float radius = (l + sphere1->radius() + sphere2->radius())*0.5f;
					result = new CVolumeSphere(0.5f * (point1 + point2), radius);			
				}
				delete sphere2; sphere2 = NULL;
				return result;
			}
		}

		CVolume* CVolumeBox::UnionBox(const CVolume* box)
		{			
			CVolumeBox * input = dynamic_cast<CVolumeBox*>(const_cast<CVolume*>(box));
			CVolumeBox* result = new CVolumeBox(vectorMin(m_minCorner, input->m_minCorner), vectorMax(m_maxCorner, input->m_maxCorner));
			return result;
		}

		CVolume* CVolumeBox::Union(const CVolume* bv)
		{	
			CVolume * input = const_cast<CVolume*>(bv);
			if(input->isBox())
				return UnionBox(bv);
			else
				return UnionSphere(bv);			
		}

		CVolume* CVolumeBox::intersectionSphere(const CVolume* sphere)
		{
			CVolumeSphere * input = dynamic_cast<CVolumeSphere*>(const_cast<CVolume*>(sphere));
			vec3f min = vectorMax(input->center().subtract(input->radius()), this->lower());
			vec3f max = vectorMin(input->center().add(input->radius()), this->upper());
			if ((min.x > max.x) || (min.y > max.y) || (min.z > max.z))
				return emptyVolume();
			else
			{				
				CVolumeBox box(min, max);				
				if (box.size() <= input->size())
					return new CVolumeBox(min, max);
				else
					return new CVolumeSphere(input);							
			}
		}

		CVolume* CVolumeBox::intersectionBox(const CVolume* box)
		{
			CVolumeBox * input = dynamic_cast<CVolumeBox*>(const_cast<CVolume*>(box));
			vec3f minV;
			vec3f maxV;
			if ((m_minCorner.x > input->upper().x)||( m_maxCorner.x < input->lower().x))
				return emptyVolume();
			else
			{
				minV.x = max(m_minCorner.x, input->lower().x);
				maxV.x = min(m_maxCorner.x, input->upper().x);
			} 

			if ((m_minCorner.y > input->upper().y)||( m_maxCorner.y < input->lower().y))
				return emptyVolume();
			else
			{
				minV.y = max(m_minCorner.y, input->lower().y);
				maxV.y = min(m_maxCorner.y, input->upper().y);
			} 
			if ((m_minCorner.z > input->upper().z) ||( m_maxCorner.z < input->lower().z))
				return emptyVolume();
			else
			{
				minV.z = max(m_minCorner.z, input->lower().z);
				maxV.z = min(m_maxCorner.z, input->upper().z);
			} 

			return new CVolumeBox(minV, maxV);
		}

		CVolume* CVolumeBox::intersection(const CVolume* bv)
		{
			CVolume * vol = const_cast<CVolume*>(bv);
			if(vol->isBox())
				return intersectionBox(bv);
			else
				return intersectionSphere(bv);
		}

		CVolumeBox CVolumeBox::operator -(CVolumeBox b)
		{
			CVolumeBox res(this->lower() - b.upper(), this->upper() - b.lower());
			return res;
		}

		CVolumeBox CVolumeBox::operator +(CVolumeBox b)
		{
			CVolumeBox res(this->lower() + b.lower(), this->upper() + b.upper());
			return res;
		}

		CVolumeBox CVolumeBox::operator *(float scale)
		{
			CVolumeBox res(scale * this->lower(), scale * this->upper());
			return  res;
		}

		void CVolumeBox::operator =(const CVolume* other)
		{
			CVolumeBox * box = dynamic_cast<CVolumeBox*>(const_cast<CVolume*>(other));
			m_minCorner = box->m_minCorner;
			m_maxCorner = box->m_maxCorner;
		}

	}
}
}

