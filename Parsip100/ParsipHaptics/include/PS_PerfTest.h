#ifndef PS_PERF_TEST_H
#define PS_PERF_TEST_H

#include <vector>
#include "PS_FrameWork/include/PS_PerfLogger.h"
#include "PS_FrameWork/include/PS_FileDirectory.h"

using namespace std;


namespace PS
{
	using namespace PS::FILESTRINGUTILS;

	class CPerfTest{
	public:
                enum Exetype {etDebug, etRelease};
                enum ExeTargetMachine {etm32, etm64};

		class TestConfig{
		public:
			DAnsiStr m_strConfigName;
			DAnsiStr m_strExeName;
			DAnsiStr m_strResPath;
			
			DAnsiStr m_strComputeShadersPath;
			DAnsiStr m_strTargetEventName;

			Exetype		m_exeType;
			ExeTargetMachine m_exeTargetMachine;

			bool		m_bUseTBB;
			bool		m_bUseTBBPipeline;
			bool		m_bUseGPUComputeShaders;
			bool		m_bUseSSE;			
			int			m_threads_count;

		public:
			//Public Functions
			TestConfig()
			{
				setDefault();
			}

			~TestConfig(){;}

			void setDefault()
			{
				m_strConfigName = "DEFAULT";
				m_strExeName = ExtractFileName( GetExePath() );
				m_strResPath = ExtractFilePath( GetExePath() );
				m_strResPath += "Resources";
				m_strComputeShadersPath = m_strResPath + "\\DXComputeShaders";
				m_strTargetEventName = "";
#ifdef DEBUG
				m_exeType = etDebug;
#else
				m_exeType = etRelease;
#endif // _DEBUG
				m_exeTargetMachine = etm32;
				m_bUseTBB = true;
				m_bUseSSE = true;
				m_bUseGPUComputeShaders = true;
			}

			TestConfig& operator=(const TestConfig& rhs)
			{
				this->m_bUseGPUComputeShaders = rhs.m_bUseGPUComputeShaders;
				this->m_bUseSSE = rhs.m_bUseSSE;
				this->m_bUseTBB = rhs.m_bUseTBB;
				this->m_exeTargetMachine = rhs.m_exeTargetMachine;
				this->m_exeType = rhs.m_exeType;

				this->m_strConfigName = rhs.m_strConfigName;
				this->m_strExeName = rhs.m_strExeName;
				this->m_strResPath = rhs.m_strResPath;
				this->m_strComputeShadersPath = rhs.m_strComputeShadersPath;
				return (*this);
			}
		};

		//Define callback for running the test on the selected file and config		
		//typedef void (*OnPerformTest)(const char* strPathFileName,  TestConfig& config);
		struct RenderTaskParams
		{
			DAnsiStr strPathFileName;
			CPerfTest::TestConfig* lpConfig;
		};

		typedef void (*OnPerformTest)(RenderTaskParams* param);
                enum WriteTestResults {wtrAppendExisting, wtrCreateNew, wtrIgnore};


	public:
		OnPerformTest FOnPerformTest;
		OnOverallProgress FOnOverallProgress;

		//Constructors
		CPerfTest();
		~CPerfTest();		

		void cleanup();
		void runTests();		
		bool writeCSV();

		void addTestPathFileName(DAnsiStr strPathFileName);
                void addTestPathFileName(vector<DAnsiStr>& lstTestFiles)
		{
			m_lstTestFiles.resize(0);
			for(size_t i=0; i < lstTestFiles.size(); i++)
				m_lstTestFiles.push_back(lstTestFiles[i]);
		}

		void addConfig(TestConfig* cfg);

		void setHeaders();
                void setHeaders(vector<DAnsiStr>& headers)
		{
			m_headers.resize(0);
			for(size_t i=0; i < headers.size(); i++)
				m_headers.push_back(headers[i]);
		}

		void setOutputFileName(DAnsiStr strPathFileName, WriteTestResults howToWrite = wtrAppendExisting)
		{
			m_strFileName = strPathFileName;
			m_writeResultsBehavior = howToWrite;
		}

		void setAppendMode(WriteTestResults howToWrite)
		{
			m_writeResultsBehavior = howToWrite;
		}

		DAnsiStr getContent(size_t idx) const 
		{ 
			DAnsiStr temp;
			if(idx >=0 && idx < m_content.size())
				temp = m_content[idx];
			return temp;
		}
		void addToContent(DAnsiStr str) {m_content.push_back(str);}

		DAnsiStr getLastMostCosty() const { return m_lastMostCosty;}
		DAnsiStr getLastTotalTime() const { return m_lastTotalTime;}
		DAnsiStr getLastSysConfig() const { return m_lastSysConfig;}

		size_t countFileEvents();
		bool storeTestResult(DAnsiStr strTestFile, TestConfig* pUsedConfig, CPerfLogger* perf, size_t rowOffset = 0);
		

	private:			
		DAnsiStr m_lastMostCosty;
		DAnsiStr m_lastTotalTime;
		DAnsiStr m_lastSysConfig;

		DAnsiStr m_strFileName;

                vector<DAnsiStr> m_lstTestFiles;
                vector<TestConfig*> m_lstTestConfigs;

		//Content of the csv file for excel output
                vector<DAnsiStr> m_headers;
                vector<DAnsiStr> m_content;

		//Writing test results behavior
		WriteTestResults m_writeResultsBehavior;

		bool readFileContent();
	};


}


#endif
