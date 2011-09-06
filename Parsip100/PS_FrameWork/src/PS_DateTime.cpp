#include "PS_DateTime.h"
#include <time.h>
#include <cmath>

namespace PS{
namespace DATETIMEUTILS{

DAnsiStr getCurrentDateTime()
{
	time_t rawtime;
	struct tm timeinfo;		
	time( &rawtime);
	localtime_s(&timeinfo, &rawtime);

	char buffer[256];
	asctime_s(buffer, 256, &timeinfo);		
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