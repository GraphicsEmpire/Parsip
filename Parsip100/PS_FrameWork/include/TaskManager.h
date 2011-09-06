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

// The TaskManager class provides generic methods for running asynchronous callbacks.
// There is some specialization for running "splittable" functions, which iterate over
// a range.

#pragma once


#include <tbb/task_scheduler_init.h>
#include <tbb/task.h>
#include <tbb/task_group.h>
#include <tbb/atomic.h>


class TaskManager
{
protected:
	class JobFunctor;
	class SplittableJobFunctor;
	class SynchronizedJobFunctor;

public:
    static TaskManager *getTaskManager();

    typedef void ( *JobFunction )( void * );
	typedef void ( *JobRangeFunction )( void *, unsigned int, unsigned int );
	typedef unsigned int ( *JobSplitFunction )( void *, unsigned int, unsigned int );
	
	// The JobResult class provides a "future" object as a return value for an asynchronous
	// request. Threads can wait on the JobResult to complete the asynchronous request.
	// If the asynchronous request introduces other asynchronous work, the waiting thread
	// will be assigned any pending work that isn't already underway.
    class JobResult
    {
	public:
		JobResult();
		~JobResult();

		bool isReady(void) { return m_bReady; }
		void markAsReady(void);
		
		void addJobFunctor(JobFunctor &tFunctor);
		void addSplittableJobFunctor(SplittableJobFunctor &tFunctor);
		void addSynchronizedJobFunctor(SynchronizedJobFunctor &tFunctor);
		void waitUntilDone(void);

    protected:
   		tbb::task_group m_tGroup;
		bool m_bReady;
		tbb::atomic<unsigned int> m_tChildCount;
    };

protected:
	// The JobFunctor class is used internally to schedule an asynchronous request.
	class JobFunctor
    {
    public:
        JobFunctor(JobFunction tFunc, void *pParam, JobResult *pResult):
			m_tFunc(tFunc),
			m_pParam(pParam),
			m_pResult(pResult)
		{}
        void operator() (void);

	protected:
        JobFunction m_tFunc;
        void *m_pParam;
		JobResult *m_pResult;
    };

	// The SplittableJobFunctor class is used internally to schedule a splittable
	// asynchronous request.
	class SplittableJobFunctor
    {
    public:
        SplittableJobFunctor(
			JobRangeFunction tRangeFunc,
			JobSplitFunction tSplitFunc,
			void *pParam,
			unsigned int uStart,
			unsigned int uEnd,
			JobResult *pResult
		):
			m_tRangeFunc(tRangeFunc),
			m_tSplitFunc(tSplitFunc),
			m_pParam(pParam),
			m_uStart(uStart),
			m_uEnd(uEnd),
			m_pResult(pResult)
		{}
        void operator() (void);

	protected:
        JobRangeFunction m_tRangeFunc;
        JobSplitFunction m_tSplitFunc;
        void *m_pParam;
		unsigned int m_uStart;
		unsigned int m_uEnd;
        JobResult *m_pResult;
    };

	// The SynchronizedJobFunctor class is used internally to schedule work that must
	// be run exactly once on each of the threads managed by the TaskManager.
	class SynchronizedJobFunctor
	{
	public:
		SynchronizedJobFunctor(JobFunction tFunc, void *pParam, tbb::atomic<int> *pAtomicCount):
			m_tFunc(tFunc),
			m_pParam(pParam),
			m_pAtomicCount(pAtomicCount)
		{}
		
		void operator() (void);
	    
	private:
		JobFunction m_tFunc;
		void *m_pParam;
		tbb::atomic<int> *m_pAtomicCount;
	};

public:
    TaskManager();

	void init(int iThreadCount = tbb::task_scheduler_init::automatic);
    void shutdown();
    
    unsigned int getThreadCount(void);

    void dispatch(
		JobResult *pResult,	// structure to track completion
		JobFunction tFunc,	// pointer to task function
		void *pParam		// context object for task
	);
	void dispatchMultiple(
		JobResult *pResult,	// structure to track completion
		JobFunction tFunc,	// pointer to task function
		void *pParams,		// array of context objects for tasks
		size_t uParamSize,	// size of one context object
		unsigned int uCount	// number of tasks to create
	);
	void dispatchSplittable(
		JobResult *pResult,				// structure used to track completion
		JobRangeFunction tRangeFunc,	// pointer to task function
		JobSplitFunction tSplitFunc,	// function that splits a range
		void *pParam,					// context object for tasks
		unsigned int uStart,			// beginning of range
		unsigned int uEnd				// end of range
	);
	void dispatchSynchronizedAndWait(
		JobFunction tFunc,	// pointer to task function
		void *pParam		// context object for tasks
	);
	void wait(
		JobResult *pResult	// pointer to previously dispatched result
	);

	static void yield();
	static void sleep(unsigned int uMilliseconds);
	
protected:
    static TaskManager *sm_pTaskManager;
    tbb::task_scheduler_init *m_pTaskScheduler;
	unsigned int m_uThreadCount;
};
