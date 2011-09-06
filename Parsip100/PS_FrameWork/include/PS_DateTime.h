#pragma once;

#ifndef PS_DATETIME_H
#define PS_DATETIME_H

#include "PS_String.h"

namespace PS{
	namespace DATETIMEUTILS{

		DAnsiStr getCurrentDateTime();
		DAnsiStr miliSecondsToString(double dtime);
	
}
}




#endif