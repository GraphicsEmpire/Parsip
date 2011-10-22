//#include "stdafx.h"
#include "PS_PerfLogger.h"
#include "PS_FileDirectory.h"
#ifdef linux
    #include <sys/time.h>
#endif

using namespace PS::FILESTRINGUTILS;

namespace PS
{

CPerfLogger* CPerfLogger::m_instance = NULL;

//========================================================================================
CPerfLogger::CPerfLogger()
{	
	init();
}
//========================================================================================
CPerfLogger::~CPerfLogger()
{	
	flushPendingEvents();
	writeFile();

	//clean up
	cleanup();
	m_instance = NULL;
}

void CPerfLogger::cleanup()
{
	m_content.resize(0);
	for(size_t i=0; i < m_pendingEvents.size(); i++)
	{
		EVENTENTRY* e = m_pendingEvents[i];
		SAFE_DELETE(e);
	}
	m_pendingEvents.resize(0);
}

void CPerfLogger::release()
{
	if(m_instance)
		SAFE_DELETE(m_instance);
}

CPerfLogger* CPerfLogger::getInstance()
{
	if(!m_instance)
	{
		m_instance = new CPerfLogger();				
	}

	return m_instance;
}

//========================================================================================
void CPerfLogger::flushPendingEvents()
{
	char strLine[MAX_LINE_SIZE];
	for(size_t i=0; i < m_pendingEvents.size(); i++)
	{
		EVENTENTRY* e = m_pendingEvents[i];
                snprintf(strLine, MAX_LINE_SIZE, "%s(%d) : %s : %4.2lf ms", e->strFile.c_str(), e->lineNumber, e->strFunction.c_str(), e->timeMS );
		m_content.push_back(DAnsiStr(strLine));
	}
}
//========================================================================================
DAnsiStr CPerfLogger::toString(double t)
{
	char strLine[32];
        snprintf(strLine, 32, "%4.2lf", t );
	return DAnsiStr(strLine);
}
//========================================================================================
DAnsiStr CPerfLogger::toString(int t)
{
	char strLine[32];
        snprintf(strLine, 32, "%d", t );
	return DAnsiStr(strLine);
}
//========================================================================================
CPerfLogger::EVENTENTRY* CPerfLogger::getMostCosty()
{
	if(m_pendingEvents.size() == 0) return NULL;

	double t = dynamic_cast<EVENTENTRY*>(m_pendingEvents[0])->timeMS;

	EVENTENTRY* e;
	EVENTENTRY* candidate = m_pendingEvents[0];
	for(size_t i=1; i < m_pendingEvents.size(); i++)
	{
		e = m_pendingEvents[i];
		if(t < e->timeMS)
		{
			t = e->timeMS;
			candidate = e;
		}
	}
	
	return candidate;
}

//========================================================================================
 CPerfLogger::EVENTENTRY* CPerfLogger::getEvent(size_t idx)
{
	if(idx >= 0 && idx < m_pendingEvents.size())
		return m_pendingEvents[idx];
	else
		return NULL;
}
//========================================================================================
 CPerfLogger::EVENTENTRY* CPerfLogger::getEventByFuncName(DAnsiStr& strAbsFuncName)
 {
	 if(m_pendingEvents.size() == 0) return NULL;

	 strAbsFuncName.toUpper();
	 
	 DAnsiStr strFunc;
	 CPerfLogger::EVENTENTRY* pe = NULL;
	 for(size_t i=0; i < m_pendingEvents.size(); i++)
	 {		 
		 pe = m_pendingEvents[i];
		 strFunc = pe->strFunction;
		 strFunc.toUpper();

		 if(strFunc == strAbsFuncName )
		 {
			 return pe;
		 }
	 }

	 return NULL;

 }
//========================================================================================
CPerfLogger::EVENTENTRY* CPerfLogger::getLeastCosty()
{
	if(m_pendingEvents.size() == 0) return NULL;

	double t = dynamic_cast<EVENTENTRY*>(m_pendingEvents[0])->timeMS;

	EVENTENTRY* e;
	EVENTENTRY* candidate = m_pendingEvents[0];
	for(size_t i=1; i < m_pendingEvents.size(); i++)
	{
		e = m_pendingEvents[i];
		if(t > e->timeMS)
		{
			t = e->timeMS;
			candidate = e;
		}
	}

	return candidate;
}

//========================================================================================
double CPerfLogger::getTotalTime()
{
	EVENTENTRY* e = NULL;

	double accTime = 0;
	for(size_t i=0; i < m_pendingEvents.size(); i++)
	{
		e = m_pendingEvents[i];
		accTime += e->timeMS;
	}

	return accTime;
}

//========================================================================================
U32 CPerfLogger::startEvent(const char* strFunc, const char* strFile, int lineNumber)
{
	EVENTENTRY* e = new EVENTENTRY;
	e->startTS = getPerfCounter();
	e->strFunction = DAnsiStr(strFunc);
	e->strFile = DAnsiStr(strFile);
	e->lineNumber = lineNumber;
	m_pendingEvents.push_back(e);
	return m_pendingEvents.size()-1;
}

//========================================================================================
void CPerfLogger::endEvent(U32 id)
{
	if(id >= 0 && id < m_pendingEvents.size())
	{
		EVENTENTRY* e = m_pendingEvents[id];
		e->endTS = getPerfCounter();
		e->timeMS = convertTimeTicksToMS(e->endTS - e->startTS);
		
                printf("%s(%d) : %s : %4.2lf ms\n", ExtractFileName(e->strFile).ptr(), e->lineNumber, e->strFunction.c_str(), e->timeMS );
		//printf("%s(%d) : %s : %4.2lf ms\n", e->strFile.c_str(), e->lineNumber, e->strFunction.c_str(), e->timeMS );
		//printf( "%s: %4.2lf ms\n", mpMsg, DUT::TimeTicksToMS( elapsed ) );
	}
}
//========================================================================================
bool CPerfLogger::writeFile()
{
	ofstream ofs(m_strFileName.ptr(), ios::out | ios::trunc);
	if(!ofs.is_open())
		return false;

	char line[MAX_LINE_SIZE];
	for(size_t i=0; i < m_content.size(); i++)
	{
                snprintf(line, MAX_LINE_SIZE, "%s\n", m_content[i].c_str());
		ofs << line;
	}
	ofs.close();

	return true;
}

//========================================================================================
I64 CPerfLogger::getPerfCounter()
{
	I64	val;

#if defined(_MSC_VER)
	QueryPerformanceCounter( (LARGE_INTEGER *)&val );

#else
	timeval		timeVal;

	gettimeofday( &timeVal, NULL );

	val = (I64)timeVal.tv_sec * (1000*1000) + (I64)timeVal.tv_usec;
#endif

	return val;
}

//==================================================================
double CPerfLogger::convertTimeTicksToMS( I64 ticks )
{
	static double	coe;

#if defined(_MSC_VER)
	static I64  	freq;

	if ( freq == 0 )
	{
		QueryPerformanceFrequency( (LARGE_INTEGER *)&freq );
		coe = 1000.0 / freq;
	}

#else
	coe = 1.0 / 1000.0;

#endif

	return ticks * coe;
}
//========================================================================================
void CPerfLogger::init()
{
	this->m_strFileName = ChangeFileExt(GetExePath(), DAnsiStr(".perf"));
/*
#if defined(_MSC_VER)
	if(m_freq == 0)	
		QueryPerformanceFrequency((LARGE_INTEGER*)&m_freq);			
	m_periodMS = 1000.0 / (double)m_freq;
#else
	if(m_freq == 0)	
		m_freq = 1;
	m_periodMS = 1.0 / 1000.0;
#endif
	*/
}

//========================================================================================
DAnsiStr CPerfLogger::toString(CPerfLogger::EVENTENTRY* e, bool bIncludeFileLine)
{
	DAnsiStr strOut;
	if(e == NULL)
		return strOut;

	char strLine[MAX_LINE_SIZE];
	if(bIncludeFileLine)
                snprintf(strLine, MAX_LINE_SIZE, "%s(%d) : %s : %4.2lf ms", e->strFile.c_str(), e->lineNumber, e->strFunction.c_str(), e->timeMS );
	else
                snprintf(strLine, MAX_LINE_SIZE, "%s : %4.2lf ms", e->strFunction.c_str(), e->timeMS );
	strOut.copyFromT(strLine);
	return strOut;
}
//========================================================================================
DAnsiStr CPerfLogger::toString(size_t idx, bool bIncludeFileLine)
{
	return toString(this->getEvent(idx), bIncludeFileLine);
}
//========================================================================================

}
