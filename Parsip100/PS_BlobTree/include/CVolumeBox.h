#ifndef CVOLUMEBOX_H
#define CVOLUMEBOX_H

#include "CVolume.h"
#include "CVolumeSphere.h"

namespace PS{
namespace BLOBTREE{
	namespace Vol{

		class  CVolumeBox : public CVolume
		{
		private:
			vec3f m_minCorner;
			vec3f m_maxCorner;

		public:
			CVolumeBox() {;}
			CVolumeBox(CVolume* vol);
			CVolumeBox(const vec3f min, const vec3f max);
			CVolumeBox(float x1, float y1, float z1, float x2, float y2, float z2);

			vec3f center();

			void set(vec3 lo, vec3 hi);
			void set(float l, float b, float n, float r, float t, float f);

			vec3f lower() const { return m_minCorner;}
			vec3f upper() const { return m_maxCorner;}

			static CVolumeBox* fromCenterSize(vec3f c, vec3f sz);
			static CVolume* emptyVolume();


			float left() const { return m_minCorner.x;}
			float right() const { return m_maxCorner.x;}
			float bottom() const { return m_minCorner.y;}
			float top() const { return m_maxCorner.y;}
			float front() const { return m_minCorner.z;}
			float back() const { return m_maxCorner.z;}

			float width()  const { return m_maxCorner.x - m_minCorner.x; }
			float height() const { return m_maxCorner.y - m_minCorner.y; }
			float depth()  const { return m_maxCorner.z - m_minCorner.z; }

			vec3f dimensions();

			void correct();

			vec3f getCorner(int index);

			vec3f getCorner(bool bLeft, bool bBottom, bool bFront);
			//**************************************************************
			bool isBox();

			float size();

			bool isInside(vec3f pt);
				
			bool intersect(CVolume* bv);

			void tranform(const CMatrix& m);
			CVolume* scale(vec3 scale);
			CVolume* rotate(quat rotate);			
			CVolume* translate(vec3 trans);
			void scaleSelf(vec3 scaleVector);
			void translateSelf(vec3 translateVector);


			CVolume* UnionSphere(const CVolume* sphere);
			CVolume* UnionBox(const CVolume* box);
			CVolume* Union(const CVolume* bv);

			CVolume* intersectionSphere(const CVolume* sphere);
			CVolume* intersectionBox(const CVolume* box);
			CVolume* intersection(const CVolume* bv);
			
			void operator =(const CVolume* other);

			CVolumeBox operator -(CVolumeBox b);

			CVolumeBox operator +(CVolumeBox b);

			CVolumeBox operator *(float scale);
		};
	}
}
}

#endif