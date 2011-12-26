#include "PS_PerfTest.h"
#include "PS_FrameWork/include/TaskManager.h"

namespace PS{


	CPerfTest::CPerfTest()
	{
		FOnPerformTest = NULL;
		FOnOverallProgress = NULL;
		m_strFileName = PS::FILESTRINGUTILS::CreateNewFileAtRoot(".csv");
		//PS::FILESTRINGUTILS::changeFileExt(PS::FILESTRINGUTILS::GetExePath(), std::string(".csv"));		
		setHeaders();
	}

	CPerfTest::~CPerfTest()
	{
		cleanup();
	}

	//Add new test configurations
	void CPerfTest::addConfig(TestConfig* cfg)
	{
		m_lstTestConfigs.push_back(cfg);
	}

	//Add new test files
	void CPerfTest::addTestPathFileName(DAnsiStr strPathFileName)
	{
		m_lstTestFiles.push_back(strPathFileName);
	}

	void CPerfTest::cleanup()
	{
		for(size_t i=0; i < m_lstTestConfigs.size(); i++)
			SAFE_DELETE(m_lstTestConfigs[i]);
		m_lstTestConfigs.resize(0);
		m_lstTestFiles.resize(0);
		m_content.resize(0);
		m_headers.resize(0);
	}

	void CPerfTest::runTests()
	{
		if(this->FOnPerformTest == NULL)
			return;

		//count csv file events
		size_t rowoffset = countFileEvents();

		//Some variables 
		TestConfig* pConfig = NULL;		
		CPerfLogger* perf = PS::CPerfLogger::getInstance();


		//Count
		int ctConfigs = (int)m_lstTestConfigs.size();
		int ctFiles = (int)m_lstTestFiles.size();

		//Get TASK Manager
		TaskManager::JobResult arrResult;
		RenderTaskParams param;
		//TaskManager* pTaskManager = TaskManager::getTaskManager();


		DAnsiStr strTestFile; 
		DAnsiStr temp;
		for(int i=0; i < ctConfigs; i++)
		{
			pConfig = dynamic_cast<TestConfig*>(m_lstTestConfigs[i]);

			//Run all files with pConfig
			for(int j=0; j < ctFiles; j++)
			{			
				strTestFile = m_lstTestFiles[j];
				temp = PS::FILESTRINGUTILS::ExtractFileName(strTestFile);			

				//Output some progress status
				if(FOnOverallProgress)
					FOnOverallProgress(temp.ptr(), static_cast<float>(i * ctFiles + j) / static_cast<float>(ctConfigs * ctFiles) );

				//Control all performance outputs				
				if(FOnPerformTest)
				{
					param.strPathFileName = strTestFile;
					param.lpConfig        = pConfig;
					FOnPerformTest(&param);
					//pTaskManager->dispatch(&arrResult, TaskManager::JobFunction(FOnPerformTest), &param);
				}

				//FOnPerformTest(strTestFile.ptr(), *pConfig);
				//pTaskManager->wait(&arrResult);

				//Store test result
				storeTestResult(strTestFile, pConfig, perf, rowoffset);

				//Clear Performance
				perf->cleanup();
			}
		}

		//Write Final CSV file
		this->writeCSV();
	}

	void CPerfTest::setHeaders()
	{
		m_headers.resize(0);
		m_headers.push_back("XP#");
		m_headers.push_back("Bucket");
		m_headers.push_back("Threads");
		m_headers.push_back("Executable");
		m_headers.push_back("Project Config");
		m_headers.push_back("Model Name");
		m_headers.push_back("Events");
		m_headers.push_back("Costy event");
		m_headers.push_back("Target Event");
		m_headers.push_back("Total Time");		
	}

	bool CPerfTest::storeTestResult(DAnsiStr strTestFile, TestConfig* pUsedConfig, CPerfLogger* perf, size_t rowoffset)
	{
		//szPrev + szCurrent + 1
		size_t idxXP = rowoffset + m_content.size() + 1;

		DAnsiStr str = DAnsiStr(perf->toString((int)idxXP));		
		str  = str + "," + DAnsiStr(perf->toString(pUsedConfig->m_threads_count));
		str  = str + "," + pUsedConfig->m_strExeName;

		//Project Config
		str = str + "," + pUsedConfig->m_strConfigName + " + ";
		
		DAnsiStr temp;
		if(pUsedConfig->m_exeType == etDebug)
			temp += "Debug + ";
		else
			temp += "Release + ";

		if(pUsedConfig->m_bUseTBB)
			temp += "TBB + ";
		else
			temp += "Serial + ";

		if(pUsedConfig->m_bUseGPUComputeShaders)
			temp += "GPU + ";
		else
			temp += "CPU + ";

		if(pUsedConfig->m_exeTargetMachine == etm32)
			temp += "32";
		else
			temp += "64";		
		str += temp;

		//Save config
		m_lastSysConfig = temp;

		//Model	
		str  = str + "," + PS::FILESTRINGUTILS::ExtractFileName(strTestFile);		

		//Events
		for (size_t i=0; i < perf->getEventCount(); i++)
		{			
			if( i == 0)
				str  = str + "," + perf->toString(i, false);				
			else
				str  = str + ";;" + perf->toString(i, false);				
		}

		//Find 
		DAnsiStr strTarget;
		CPerfLogger::EVENTENTRY *pe = NULL;
		pe = perf->getEventByFuncName(pUsedConfig->m_strTargetEventName);
		if(pe != NULL)
			strTarget = perf->toString(pe->timeMS);
		else
			strTarget = "NA";

		m_lastMostCosty	   = perf->toString(perf->getMostCosty(),  false);		 
		m_lastTotalTime	   = perf->toString(perf->getTotalTime());

		str = str + "," + m_lastMostCosty;
		str = str + "," + strTarget;
		str = str + "," + m_lastTotalTime;		

		m_content.push_back(str);
		return true;
	}

	size_t CPerfTest::countFileEvents()
	{
		ifstream ifs(m_strFileName.ptr(), ios::in);
		if(!ifs.is_open())
			return 0;

		size_t ctEvents = 0;
		CAString strLine;
		while( !ifs.eof())
		{			
			ifs >> strLine;
			strLine.trim();
			if(strLine.length() > 4)
				ctEvents++;		
		}	
		ifs.close();
		if(ctEvents > 0)
			return ctEvents - 1;
		else
			return 0;
	}

	bool CPerfTest::readFileContent()
	{
		ifstream ifs(m_strFileName.ptr(), ios::in);
		if(!ifs.is_open())
			return false;

		char line[MAX_LINE_SIZE];		

		int ctEvents = 0;
		while( !ifs.eof())
		{
			ifs.getline(line, MAX_LINE_SIZE);			
			if((strlen(line) > 0) && (ctEvents > 0))
			{				
				//Push the content to the front
                                m_content.push_back(DAnsiStr(line));
			}
			ctEvents++;
		}	
		ifs.close();
		return true;
	}


	//Write down our csv file
	bool CPerfTest::writeCSV()
	{
		ofstream ofs;
		bool bWriteHeader = true;

		if(m_writeResultsBehavior == wtrIgnore)
			return false;

		if(m_writeResultsBehavior == wtrAppendExisting)
		{
			if(PS::FILESTRINGUTILS::GetFileSize(m_strFileName.ptr()) > 0)
				bWriteHeader = false;
			ofs.open(m_strFileName.ptr(), ios::out | ios::app);
		}
		else
			ofs.open(m_strFileName.ptr(), ios::out | ios::trunc);

		if(!ofs.is_open())
			return false;
		
		//Write header
		if((m_headers.size() > 0)&&(bWriteHeader))
		{
			DAnsiStr strHeader;
			for(size_t i=0; i < m_headers.size(); i++)
			{
				if(i == m_headers.size() - 1)
					strHeader += m_headers[i];
				else
					strHeader += m_headers[i] + ",";
			}			
			ofs << strHeader << '\0' << '\n';
		}

		DAnsiStr strLine;
		//Write test results		
		for(size_t i=0; i < m_content.size(); i++)
		{
			strLine = m_content[i];
			if(strLine.length() > 0)		
				ofs << strLine << '\0' << '\n';			
		}
		ofs.close();

		return true;
	}

}
