#pragma once
#ifndef PS_PERFLOGGER_H
#define PS_PERFLOGGER_H

#define	QuickPerformanceLog PS::CPerfLogger::CPerfEvent prof(PS::CPerfLogger::getInstance(), __FILE__, __FUNCTION__, __LINE__);

#ifndef MAX_LINE_SIZE
#define MAX_LINE_SIZE 1024
#endif 


#include <vector>
#include <string>
#include <fstream>

#include "PS_String.h"
#include "_dataTypes.h"

#ifdef WIN32
#include <windows.h>
#endif

using namespace std;

namespace PS{
	class CPerfLogger
	{
	public:
		struct EVENTENTRY{
			DAnsiStr strFile;
			DAnsiStr strFunction;
			int lineNumber;

			double timeMS;
			I64 startTS;
			I64 endTS;
		};
		//===========================================================
		class CPerfEvent
		{
		private:
			CPerfLogger* m_logger;
                        U32	m_eventID;

			void init();
		public:
			CPerfEvent(CPerfLogger* logger, const char* strFileName, const char* strFuncName, int lineNumber)
			{
				this->m_logger = logger;
				m_eventID = m_logger->startEvent(strFuncName, strFileName, lineNumber);
			}

			~CPerfEvent()
			{
				m_logger->endEvent(m_eventID);
			}
		};
		//===========================================================

		CPerfLogger();
		~CPerfLogger();

		void cleanup();

		//Get an static instance of CPerfLogger
		static CPerfLogger* getInstance();
		//Release the only static instance
		static void release();

		//Timing and Event Control
		static I64 getPerfCounter();
		static	double convertTimeTicksToMS( I64 ticks );

                U32 startEvent(const char* strFunc, const char* strFile, int lineNumber);
                void endEvent(U32 id);

		//Some statistical functions
		//Get Most time-consuming event
		CPerfLogger::EVENTENTRY* getEventByFuncName(DAnsiStr& strAbsFuncName);
		CPerfLogger::EVENTENTRY* getMostCosty();
		CPerfLogger::EVENTENTRY* getLeastCosty();
		double getTotalTime();

		//Getters
		size_t getEventCount() const { return m_pendingEvents.size(); }
		CPerfLogger::EVENTENTRY* getEvent(size_t idx);


		//Convert to string
		DAnsiStr toString(CPerfLogger::EVENTENTRY* e, bool bIncludeFileLine = true);
		DAnsiStr toString(size_t idx, bool bIncludeFileLine = true);
		
		static DAnsiStr toString(double t);
		static DAnsiStr toString(int t);


		bool writeFile();
		void flushPendingEvents();
	private:
                vector<DAnsiStr>  m_content;
		vector<EVENTENTRY*> m_pendingEvents;
		DAnsiStr m_strFileName;


		void init();


	protected:
		//Instance
		static CPerfLogger* m_instance;
	};

}

#endif
