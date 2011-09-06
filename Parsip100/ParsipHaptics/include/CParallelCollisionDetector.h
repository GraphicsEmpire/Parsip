#ifndef CPARALLEL_COLLISION_DETECTOR_H
#define CPARALLEL_COLLISION_DETECTOR_H

#include "CBlobTreeAnimation.h"

namespace PS{
	namespace BLOBTREEANIMATION{

	class CParCollisionDetector{
	private:
		DVec<CBlobTree*> m_vModels;
	public:
		CParCollisionDetector() {}
		~CParCollisionDetector() {}

		int detect();

	};


	}
}
#endif