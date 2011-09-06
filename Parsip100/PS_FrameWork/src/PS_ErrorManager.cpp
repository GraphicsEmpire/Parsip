//#include "stdafx.h"
#ifdef _WIN32
	#include <windows.h>
#endif // _WIN32

#include <iostream>
#include <stdexcept>
//#include <vector>

#include "PS_Exceptions.h"
#include "PS_FileDirectory.h"
#include "PS_ErrorManager.h"


namespace PS{

DVec<DAnsiStr> CErrorManager::m_errorStack = DVec<DAnsiStr>();
CErrorManager* CErrorManager::sm_pErrorManager = 0;
bool CErrorManager::m_isDestroyed = false;

/**
*/
void CErrorManager::PushError(const char *file, int line)
{
	if (m_errorStack.size() < 100)
		m_errorStack.push_back(FormatError(file, line, "An assertion failure has occurred."));
}

/**
*/
void CErrorManager::PushError(const char *file, int line, const char *message)
{
	if (m_errorStack.size() < 100 && message)
	{
		DAnsiStr entireMessage(message);
		DVec<DAnsiStr> partialMessageArray;
		DAnsiStr temp;

		size_t last, current;

		// The last element of the string.
		last = entireMessage.length();

		// Walk backward through the string.
		while (entireMessage.rfind('\n', current))
		{
			current++;

			// Push each message into the vector. Current is incremented 
			// before copying to avoid copying the delimiter.
			temp = entireMessage.substr(current, last - current);

			partialMessageArray.push_back(temp);

			// Back over the delimiter we just found, and set last to the end 
			// of the next message.
			current -= 2;
			last = current;			
		}

		temp = entireMessage.substr(0, last - current);

		// Pick up the first message - it's not preceded by a delimiter.
		partialMessageArray.push_back(temp);

		while (partialMessageArray.size() != 0)
		{
			if (!partialMessageArray.back().empty())
			{
				temp = FormatError(file, line, partialMessageArray.back().ptr());
				m_errorStack.push_back(temp);
			}

			partialMessageArray.pop_back();
		}
	}
}

/**
*/
void CErrorManager::PushError(const char *message)
{
	if (m_errorStack.size() < 100 && message)
	{
		DAnsiStr entireMessage(message);
		DVec<DAnsiStr> partialMessageArray;
		DAnsiStr temp;

		size_t last, current;

		// The last element of the string.
		last = entireMessage.length();
		
		// Walk backward through the string.
		while (entireMessage.rfind('\n', current))
		{
			current++;

			// Push each message into the vector. Current is incremented 
			// before copying to avoid copying the delimiter.
			temp = entireMessage.substr(current, last - current);

			partialMessageArray.push_back(temp);

			// Back over the delimiter we just found, and set last to the end 
			// of the next message.
			current -= 2;
			last = current;			
		}
		
		temp = entireMessage.substr(0, last - current);

		// Pick up the first message - it's not preceded by a delimiter.
		partialMessageArray.push_back(temp);

		while (partialMessageArray.size() != 0)
		{
			if (!partialMessageArray.back().empty())
			{
				temp = FormatError(partialMessageArray.back().ptr());
				m_errorStack.push_back(temp);
			}

			partialMessageArray.pop_back();
		}
	}
}

void CErrorManager::PopMostRecentThenCleanup() const
{
	if(m_errorStack.size() == 0) return;
	DAnsiStr strError = m_errorStack.back();
	if (strError.length() == 0) return;
	
	DWideStr wstrMsg = printToWStr("There are %d errors. I am going to clear all of them but this is the most recent one:\n", m_errorStack.size());
	wstrMsg.appendFromA(strError.ptr());

#ifdef _WIN32
	MessageBox(0, (LPCWSTR)wstrMsg.ptr(), (LPCWSTR)L"Error", MB_OK);
#else // !_WIN32
	cout << wstrMsg << endl;
#endif // _WIN32	
	m_errorStack.clear();
}

/**
*/
void CErrorManager::PopError(void) const
{
	if(m_errorStack.size() == 0) return;

	DAnsiStr strError = m_errorStack.back();
	if (strError.length() == 0) return;

	DWideStr wstr(strError.ptr(), strError.length());
#ifdef _WIN32
	MessageBox(0, (LPCWSTR)wstr.ptr(), (LPCWSTR)L"Error", MB_OK);
#else // !_WIN32
	cout << strError << endl;
#endif // _WIN32
	m_errorStack.pop_back();
}

void CErrorManager::ClearErrors()
{
	if(m_errorStack.size() > 0 )
	{
		DAnsiStr strMsg = printToAStr("Clearing %d errors.", m_errorStack.size());
		OutputDebugStringA(strMsg.ptr());
		m_errorStack.clear();
	}	
}

/**
*/
void CErrorManager::FlushErrors() const
{
	int errorCount;

	errorCount = 0;

	while (m_errorStack.size() > 0)
	{
		PopError();

		errorCount++;
	}

	//if (errorCount)
	//	cout << errorCount << " error(s)" << endl;
}

/**
*/
bool CErrorManager::IsOk(void) const
{
	return (m_errorStack.size() == 0);
}

/**
*/
CErrorManager::CErrorManager(void)
{
	m_errorStack.clear();
}

/**
*/
CErrorManager::~CErrorManager(void)
{
	sm_pErrorManager = 0;
	m_isDestroyed = true;
}

/**
*/
void CErrorManager::Create(void)
{
	static CErrorManager theInstance;

	m_errorStack.clear();

	sm_pErrorManager = &theInstance;
}

/**
*/
void CErrorManager::OnDeadReference(void)
{
	//throw runtime_error("Dead Reference Detected.");
	throw PS::CAExceptions("Dead Reference Detected.");
}

/**
*/
DAnsiStr CErrorManager::FormatError(const char *file, int line, const char *message) const
{
	char lineString[64];
	DAnsiStr errorString;

#ifdef _WIN32
	sprintf_s(lineString, 64, "%i", line);
#else // !_WIN32
	sprintf(lineString, "%i", line);
#endif // _WIN32

	errorString = file;
	errorString += "(";
	errorString += lineString;
	errorString += ")";
	errorString += " : error: ";

	if (message)
		errorString += message;
	else 
		errorString += "(invalid error message)";

	return errorString;
}

/**
*/
DAnsiStr CErrorManager::FormatError(const char *message) const
{
	DAnsiStr errorString;

	errorString = "error: ";

	if (message)
		errorString += message;
	else 
		errorString += "(invalid error message)";

	return errorString;
}

}
