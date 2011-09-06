#include "DUtils_Stats.h"

#include <windows.h>
#include <wtypes.h>

namespace ParsStats
{

	size_t getCPUCoresCount()
	{
		SYSTEM_INFO SysInfo;
		GetSystemInfo(&SysInfo);
		return SysInfo.dwNumberOfProcessors;
	}
}