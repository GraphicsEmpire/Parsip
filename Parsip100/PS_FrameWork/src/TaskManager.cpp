// Copyright 2009 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#include "TaskManager.h"

#include <tbb/task_scheduler_init.h>
#include <tbb/task.h>
#include <tbb/task_group.h>
#include <tbb/tbb_thread.h>
#include <assert.h>

#define ASSERT(x) assert(x)


// This is for singleton support.
TaskManager *TaskManager::sm_pTaskManager = NULL;

// This function returns the singleton, after creating it if necessary.
TaskManager *TaskManager::getTaskManager()
{
    if(sm_pTaskManager == NULL)
    {
        sm_pTaskManager = new TaskManager();
    }
    
    return sm_pTaskManager;
}

TaskManager::JobResult::JobResult() :
m_bReady(true)
{
	m_tChildCount = 0;
}

TaskManager::JobResult::~JobResult()
{
	waitUntilDone();
}

void TaskManager::JobResult::markAsReady(void)
{
	if(m_tChildCount.fetch_and_decrement() == 1)
	{
		m_bReady = true;
	}
}

void TaskManager::JobResult::addJobFunctor(JobFunctor &tFunctor)
{
	m_tChildCount++;
	m_bReady = false;
	m_tGroup.run(tFunctor);
}

void TaskManager::JobResult::addSplittableJobFunctor(SplittableJobFunctor &tFunctor)
{
	m_tChildCount++;
	m_bReady = false;
	m_tGroup.run(tFunctor);
}

void TaskManager::JobResult::addSynchronizedJobFunctor(SynchronizedJobFunctor &tFunctor)
{
	m_tChildCount++;
	m_bReady = false;
	m_tGroup.run(tFunctor);
}

// This function guarantees completion of the work promised by the JobResult.
// The first call to this function may block, but subsequent calls will not.
void TaskManager::JobResult::waitUntilDone()
{
	m_tGroup.wait();
}

// This function actually calls the function pointer specified in the asynchronous
// request.
void TaskManager::JobFunctor::operator() (void)
{
    ASSERT(m_pResult != NULL);
    ASSERT(m_tFunc != NULL);
	
	m_tFunc(m_pParam);
    m_pResult->markAsReady();
}

// This function eventually calls the function pointer specified in the asynchronous
// request, but first splits the work up as much as possible. The expectation is that
// other threads in the thread pool will steal these split pieces, and possibly split
// them further.
void TaskManager::SplittableJobFunctor::operator() (void)
{
	ASSERT(m_pResult != NULL);
    ASSERT(m_tRangeFunc != NULL);
	ASSERT(m_tSplitFunc != NULL);
	
	unsigned int uSplit = 0;
	uSplit = m_tSplitFunc(m_pParam, m_uStart, m_uEnd);
	while(uSplit > 0)
	{
		// We split the task in two and switch focus to one of the pieces.

		// We allocate the functor for the second half, then add it.
		SplittableJobFunctor tFunctor(
			m_tRangeFunc,
			m_tSplitFunc,
			m_pParam,
			uSplit,
			m_uEnd,
			m_pResult
		);
		m_pResult->addSplittableJobFunctor(tFunctor);
		
		// We change our parameters to focus on the first half.
		m_uEnd = uSplit;

		uSplit = m_tSplitFunc(m_pParam, m_uStart, m_uEnd);
	}
	
	// We're done splitting, so just execute.
	m_tRangeFunc(m_pParam, m_uStart, m_uEnd);
	m_pResult->markAsReady();
}

// This function actually calls the function pointer specified in the asynchronous
// request.
void TaskManager::SynchronizedJobFunctor::operator() (void)
{
	ASSERT(m_tFunc != NULL);
	m_tFunc(m_pParam);
	(*m_pAtomicCount)--;
    
	while(*m_pAtomicCount > 0)
	{
		// We yield while waiting.
		TaskManager::yield();
	}
}

// This is the TaskManager constructor.
TaskManager::TaskManager():
	m_pTaskScheduler(NULL),
	m_uThreadCount(0)
{}

// This function initializes the TaskManager with a specified number of threads. The parameter
// is optional and can be omitted to use a number of threads equal to the number of logical
// cores.
void TaskManager::init(int iThreadCount)
{
	m_pTaskScheduler = new tbb::task_scheduler_init(iThreadCount);

	if(iThreadCount == tbb::task_scheduler_init::automatic)
	{
		m_uThreadCount = tbb::task_scheduler_init::default_num_threads();
	}
	else
	{
		m_uThreadCount = (unsigned int)iThreadCount;
	}
}

// This function deallocates memory created during initialization. The calling code
// is responsible for ensuring that all pending asynchronous work is complete.
void TaskManager::shutdown()
{
    if(m_pTaskScheduler != NULL)
    {
        delete m_pTaskScheduler;
        m_pTaskScheduler = NULL;
    }
}

// This function returns the number of threads managed by the TaskManager.
unsigned int TaskManager::getThreadCount(void)
{
	return m_uThreadCount;
}

// This function adds some asynchronous work for execution. The JobFunction is passed
// the specified parameter. Wait on the JobResult to guarantee completion.
void TaskManager::dispatch(JobResult *pResult, JobFunction tFunc, void *pParam)
{
	JobFunctor tFunctor(tFunc, pParam, pResult);
	pResult->addJobFunctor(tFunctor);
}

// This function adds multiple pieces of asynchronous work for execution. uCount pieces of
// work are scheduled.  For each, the JobFunction is passed a parameter from the pParams array,
// whose elements are of the specified size. Wait on the JobResult to guarantee completion.
void TaskManager::dispatchMultiple(
	JobResult *pResult,
	JobFunction tFunc,
	void *pParams,
	size_t uParamSize,
	unsigned int uCount
)
{
	// We get pParams as a one-byte pointer, for doing the pointer arithmetic later.
	char *pParamsAsOneByte = (char *)pParams;

	for(unsigned int i = 0; i < uCount; i++)
	{
		JobFunctor tFunctor(tFunc, (pParamsAsOneByte + (i * uParamSize)), pResult);
		pResult->addJobFunctor(tFunctor);
	}
}	

// This function adds some asynchronous work for execution, which may be later split. The range specified
// by uStart and uEnd is passed to the JobSplitFunction to determine if splitting is necessary. When a
// split is not necessary, the same range is passed to the JobRangeFunction.
void TaskManager::dispatchSplittable(
	JobResult *pResult,
	JobRangeFunction tRangeFunc,
	JobSplitFunction tSplitFunc,
	void *pParam,
	unsigned int uStart,
	unsigned int uEnd
)
{
    SplittableJobFunctor tFunctor(tRangeFunc, tSplitFunc, pParam, uStart, uEnd,  pResult);
	pResult->addSplittableJobFunctor(tFunctor);
}	

// This function assigns each thread in the thread pool to execute the JobFunction exactly once. This
// function does not return until all the threads have executed the JobFunction.
void TaskManager::dispatchSynchronizedAndWait(JobFunction tFunc, void *pParam)
{
	// We call tFunc on each thread managed by this scheduler.
	int iThreads = getThreadCount();
	tbb::atomic<int> tAtomicCount;
	tAtomicCount = iThreads;
	
	JobResult tResult;

	for(int i = 0; i < iThreads; i++)
	{
		SynchronizedJobFunctor tFunctor(tFunc, pParam, &tAtomicCount);
		tResult.addSynchronizedJobFunctor(tFunctor);
	}

	tResult.waitUntilDone();
}

// This function waits on the result of one of the dispatch calls.
void TaskManager::wait(JobResult *pResult)
{
	ASSERT(pResult != NULL);
	pResult->waitUntilDone();
}

// This function is a convenience for the API-specific method of yielding.
void TaskManager::yield()
{
	tbb::this_tbb_thread::yield();
}

// This function is a convenience for the API-specific method of sleeping.
void TaskManager::sleep(unsigned int uMilliseconds)
{
	tbb::tick_count::interval_t tInterval((double)uMilliseconds * 0.001);
	tbb::this_tbb_thread::sleep(tInterval);
}