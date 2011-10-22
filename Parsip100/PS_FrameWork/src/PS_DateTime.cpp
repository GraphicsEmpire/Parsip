#include "PS_DateTime.h"
#include <time.h>
#include <cmath>

namespace PS{
namespace DATETIMEUTILS{

DAnsiStr getCurrentDateTime()
{
	time_t rawtime;	
        char buffer[256];

        time( &rawtime);
#ifdef WIN32
        struct tm timeinfo;
        localtime_s(&timeinfo, &rawtime);
	asctime_s(buffer, 256, &timeinfo);		
#else
        struct tm * lpTimeInfo;
        lpTimeInfo = localtime(&rawtime);

        strncpy(buffer, asctime(lpTimeInfo), 256);
#endif
	return DAnsiStr(buffer);
}

DAnsiStr miliSecondsToString(double dtime)
{
	int hh, mm, ss;
	int mili = static_cast<int>(ceil(dtime));
	ss = mili / 1000;
	mm = ss / 60;
	hh = mm / 60;
	mili = mili % 1000;

	DAnsiStr strOut = printToAStr("%d:%d:%d.%d", hh, mm, ss, mili);
	return strOut;
}

}
}
