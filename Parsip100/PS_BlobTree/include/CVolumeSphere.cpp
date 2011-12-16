#include "CVolumeSphere.h"
#include "CVolumeBox.h"
#include "PS_FrameWork/include/_dataTypes.h"

using namespace std;

namespace PS{
namespace BLOBTREE{
	namespace Vol{

		CVolumeSphere::CVolumeSphere(vec3f center, float radius)
		{
			m_center = center;
			m_radius = radius;
		}

		CVolumeSphere::CVolumeSphere(CVolume* vol)
		{
			if(vol->isBox())
			{
				CVolumeBox* b = dynamic_cast<CVolumeBox*>(vol);
				m_center = b->center();
				m_radius = 0.5f * (b->upper() - b->lower()).length();				
			}
			else
			{
				CVolumeSphere* s = dynamic_cast<CVolumeSphere*>(vol);
				m_center = s->center();
				m_radius = s->radius();
			}
		}

		void CVolumeSphere::set(vec3f center, float radius)
		{
			m_center = center;
			m_radius = radius;
		}

		void CVolumeSphere::setRadius(float radius) { m_radius = radius;}

		void CVolumeSphere::setCenter(vec3f center) { m_center = center;}

		float CVolumeSphere::size() 
		{
			return 4.0f * (Pi * m_radius * m_radius * m_radius) / 3.0f;
		}

		bool CVolumeSphere::isBox() {return false;}

		bool CVolumeSphere::isEmpty()
		{
			return (m_radius == 0.0f);
		}

		float CVolumeSphere::distance(vec3f v)
		{
			return (v - m_center).length() - m_radius;
		}

		bool CVolumeSphere::isInside(vec3f pt)
		{
			return ((pt - m_center).length2() <= m_radius* m_radius);
		}

	
		CVolume* CVolumeSphere::emptyVolume()
		{
			vec3f center(0.0f, 0.0f, 0.0f);
			return new CVolumeSphere(center, 0.0f);
		}


		bool CVolumeSphere::intersect(CVolume* bv)
		{
			if(bv->isBox())
			{			
				CVolumeBox b(bv);

				double dist = 0.0;
				double d0 = -m_center.x + m_radius + b.upper().x;
				double d1 =  m_center.x + m_radius - b.lower().x;
				if (d0 <= 0.0 || d1 <= 0.0)
					return false;
				d0 = max(m_radius - min(d0, d1), 0.0);
				dist += d0 * d0;

				d0 = -m_center.y + m_radius + b.upper().y;
				d1 = m_center.y + m_radius - b.lower().y;
				if (d0 <= 0.0 || d1 <= 0.0)
					return false;
				d0 = max(m_radius - min(d0, d1), 0.0);
				dist += d0 * d0;

				d0 = -m_center.z + m_radius + b.upper().z;
				d1 = m_center.z + m_radius - b.lower().z;
				if (d0 <= 0.0 || d1 <= 0.0)
					return false;
				d0 = max(m_radius - min(d0, d1), 0.0);
				dist += d0 * d0;

				return dist < radius2();
			}
			else
			{
				CVolumeSphere s(bv);
				double r = s.radius() + this->radius();
				return ((s.center() - this->center()).length2() < r * r);
			}
		}


		void CVolumeSphere::tranform( const CMatrix& m )
		{
			CVolumeBox* box = new CVolumeBox(this);
			box->tranform(m);

			m_center = box->center();
			m_radius = 0.5f * (box->upper() - box->lower()).length();				

			SAFE_DELETE(box);
		}

		CVolume* CVolumeSphere::scale(vec3f s)
		{
			CVolumeSphere* sphere = new CVolumeSphere(m_center * s, m_radius * Max(s));
			CVolumeBox* box = new CVolumeBox(this);
			box->scaleSelf(s);
			if(box->size() <= sphere->size())
			{
				SAFE_DELETE(sphere);
				return box;
			}
			else
			{
				SAFE_DELETE(box);
				return sphere;
			}
		}

		CVolume* CVolumeSphere::translate(vec3f trans)
		{
			CVolumeSphere* result = new CVolumeSphere(m_center + trans, m_radius);
			return result;
		}

		void CVolumeSphere::scaleSelf(vec3f scaleVector)
		{
			m_center = m_center * scaleVector;
			m_radius = m_radius * Max(scaleVector);
		}

		void CVolumeSphere::translateSelf(vec3f translateVector)
		{
			m_center = m_center + translateVector;
		}


		CVolume* CVolumeSphere::rotate(quat rot)
		{
			CVolumeSphere* result = new CVolumeSphere(rot.transform(m_center), m_radius);
			return result;
		}

		CVolume* CVolumeSphere::UnionSphere(const CVolume* sphere)
		{
			CVolumeSphere * input = dynamic_cast<CVolumeSphere*>(const_cast<CVolume*>(sphere));
			vec3f diff = input->center() - this->center();
			float l = diff.length();
			if (l == 0.0f)
				if (m_radius < input->radius())
					return newFrom(input);
				else
					return newFrom(this);
			else
			{
				l = 1.0f/l;
				vec3f point1 = diff*(-m_radius*l) + m_center;
				vec3f point2 = diff*input->radius()*l + input->center();
				if (isInside(point2))
					return newFrom(this);
				if (input->isInside(point1))
					return newFrom(input);

				float rad = (point2 - point1).length()*0.5f;
				vec3f cent = 0.5f*(point1 + point2);
				return new CVolumeSphere(cent, rad);
			}
		}

		CVolume* CVolumeSphere::UnionBox(const CVolume * box)
		{
			CVolumeBox * input = dynamic_cast<CVolumeBox*>(const_cast<CVolume*>(box));	
			return new CVolumeBox(vectorMin(m_center.add(- m_radius), input->lower()), vectorMax(m_center.add(m_radius), input->upper()));
		}


		CVolume* CVolumeSphere::Union(const CVolume* bv)
		{
			CVolume * input = const_cast<CVolume*>(bv);
			if(input->isBox())
				return UnionBox(bv);
			else
				return UnionSphere(bv);
		}

		CVolume* CVolumeSphere::intersection(const CVolume* bv)
		{
			CVolume * input = const_cast<CVolume*>(bv);
			if(input->isBox())
				return intersectionBox(input);
			else
				return intersectionSphere(input);
		}

		CVolume* CVolumeSphere::intersectionSphere(const CVolume* bv)
		{
			CVolumeSphere * sphere = dynamic_cast<CVolumeSphere*>(const_cast<CVolume*>(bv));
			vec3f diff = m_center - sphere->center();
			float d = diff.length();
			if (d == 0.0f)
				if (m_radius < sphere->radius())
					return newFrom(this);
				else
					return newFrom(sphere);

			float d2 = 0.5f*(d + (sphere->radius2() - this->radius2())/ d );
			if (d2 <= 0.0f)
				return newFrom(sphere);
			else if (d2 >= d)
				return newFrom(this);

			float r2 = sphere->radius2() - d2*d2;
			if (r2 <= 0.0f)
				return emptyVolume();
			else
				return new CVolumeSphere(diff.multiply(d2/d).add(sphere->center()), sqrt(r2));
		}

		CVolume* CVolumeSphere::intersectionBox(const CVolume* bv)
		{
			CVolumeBox * box = dynamic_cast<CVolumeBox*>(const_cast<CVolume*>(bv));
			vec3f min = vectorMax(m_center.add(- m_radius), box->lower());
			vec3f max = vectorMin(m_center.add(  m_radius), box->upper());
			if ((min.x > max.x) || (min.y > max.y) || (min.z > max.z))
				return emptyVolume();
			else
			{
				vec3f cent = min.add(max).multiply(0.5f);
				CVolumeBox * b = new CVolumeBox(cent, max.subtract(cent));				
				if (b->size() <= this->size())
					return b;
				else
				{
					delete b; b = NULL;
					return newFrom(this);
				}
			}
		}

		void CVolumeSphere::operator =(const CVolume* other)
		{
			CVolumeSphere * sphere = dynamic_cast<CVolumeSphere*>(const_cast<CVolume*>(other));
			m_center = sphere->m_center;
			m_radius = sphere->m_radius;
		}

	}
}
}