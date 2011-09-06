//==================================================================
/// DUtils.h
///
/// Created by Davide Pasca - 2008/12/31
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DUTILS_H
#define DUTILS_H

#include "DTypes.h"
#include "DUtils_Base.h"
//#include "DUtils_Files.h"
//#include "DUtils_MemFile.h"
//#include "DUtils_FileManager.h"

//==================================================================
namespace DUT
{

//==================================================================
void StrStripBeginEndWhite( char *pStr );
const char *StrStrI( const char *pStr, const char *pSearch );

I64 GetTimeTicks();
double TimeTicksToMS( I64 ticks );

void SleepMS( U32 sleepMS );

//==================================================================
/// QuickProf
//==================================================================
class QuickProf
{
	const char *mpMsg;
	I64			mStart;

public:
	QuickProf( const char *pMsg ) :
		mpMsg(pMsg)
	{
		mStart = DUT::GetTimeTicks();
	}

	~QuickProf()
	{
		I64	elapsed = DUT::GetTimeTicks() - mStart;
		printf( "%s: %4.2lf ms\n", mpMsg, DUT::TimeTicksToMS( elapsed ) );
	}
};

//==================================================================
/// TimeOut
//==================================================================
class TimeOut
{
	I64	mStartTicks;
	U32	mTimeOutMS;

public:
	TimeOut( U32 timeoutMS ) :
		mTimeOutMS(timeoutMS),
		mStartTicks(GetTimeTicks())
	{
	}

	bool IsExpired() const
	{
		I64 delta = GetTimeTicks() - mStartTicks;

		return TimeTicksToMS( delta ) > mTimeOutMS;
	}
};

class CDExceptions
{
private:
	static const int MAX_EXCEPTION_SIZE = 4096;
	char* m_strMsg;
public:
	CDExceptions( const char *pFmt, ... );	

	~CDExceptions()
	{
		if(m_strMsg)
		{
			delete m_strMsg;
			m_strMsg = NULL;
		}

	}

	const char* getMessage() const
	{
		return m_strMsg;
	}
};


//==================================================================
}

#endif
