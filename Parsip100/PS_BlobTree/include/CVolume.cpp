#include "CVolume.h"
#include "CVolumeBox.h"
#include <algorithm>

using namespace std;

namespace PS{
namespace BLOBTREE{
namespace Vol{
		CVolume* CVolume::newFrom(const CVolume* other)
		{			
			CVolume* input = const_cast<CVolume*>(other);
			if(input->isBox())							
				return new CVolumeBox(input);			
			else			
				return new CVolumeSphere(input);			
		}

		CVolume* CVolume::emptyVolume()
		{
			return CVolumeBox::emptyVolume();
		}

		float CVolume::Min(vec3f v)
		{
			return min(v.z, min(v.x, v.y));
		}

		float CVolume::Max(vec3f v)
		{
			return max(v.z, max(v.x, v.y));
		}

		vec3f CVolume::vectorMin(vec3f a, vec3f b)
		{
			vec3f c(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
			return c;
		}

		vec3f CVolume::vectorMax(vec3f a, vec3f b)
		{
			vec3f c(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
			return c;
		}

		vec3f CVolume::vectorAbs(vec3f v) const
		{
			vec3f result;
			result.x = fabsf(v.x);
			result.y = fabsf(v.y);
			result.z = fabsf(v.z);
			return result;
		}

	}
}
}