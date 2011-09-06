#pragma once
#ifndef PS_SKETCHCONFIG_H
#define PS_SKETCHCONFIG_H

#include <vector>
#include "PS_FrameWork/include/PS_AppConfig.h"
#include "PS_FrameWork/include/PS_Vector.h"

using namespace std;
using namespace PS::MATH;

namespace PS{

	class CSketchConfig : public CAppConfig
	{
	public:
		CSketchConfig();

		CSketchConfig(const std::vector<DAnsiStr>& inputContent);

		CSketchConfig(DAnsiStr strFileName, FileMode mode)
		{
			set(strFileName, mode);
		};

		~CSketchConfig()
		{

		}

		DAnsiStr readString(DAnsiStr section, DAnsiStr variable);


		bool readIntArray(DAnsiStr section, DAnsiStr variable, int ctExpected, std::vector<int>& arrayInt);
		int	writeIntArray(DAnsiStr section, DAnsiStr variable, const std::vector<int>& arrayInt);
		
		vec3f readVec3f(DAnsiStr section, DAnsiStr variable);
		void writeVec3f(DAnsiStr section, DAnsiStr variable, vec3f val);

		
		vec4f readVec4f(DAnsiStr section, DAnsiStr variable);
		void writeVec4f(DAnsiStr section, DAnsiStr variable, vec4f val);

	};


}
#endif