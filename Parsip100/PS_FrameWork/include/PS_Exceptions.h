#pragma once

#ifndef PS_EXCEPTIONS_H
#define PS_EXCEPTIONS_H

#include "PS_String.h"

namespace PS
{


class CAExceptions
{
private:
	DAnsiStr m_strMsg;
public:
	CAExceptions( const char *pFmt, ... );
	CAExceptions( const DAnsiStr& strMsg );

	~CAExceptions()
	{
	}

	const DAnsiStr& getMessage() const
	{
		return m_strMsg;
	}
};

}
#endif
