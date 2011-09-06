#include "PS_Exceptions.h"

namespace PS
{
	CAExceptions::CAExceptions( const char *pFmt, ... )
	{
		va_list	vl;
		va_start( vl, pFmt );

		char	buff[4096];

		vsnprintf_s( buff, sizeof(buff), _countof(buff)-1, pFmt, vl );
		va_end( vl );

		m_strMsg.copyFromT(buff);
	}

	CAExceptions::CAExceptions( const DAnsiStr& strMsg )
	{
		m_strMsg = strMsg;
	}

}
