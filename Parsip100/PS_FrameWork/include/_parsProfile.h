//This file is created for profiling runs.
//Here we defined 16 timers

#include "windows.h"

static LARGE_INTEGER G_PARS_PROFILER_STARTS[16];
static LARGE_INTEGER G_PARS_PROFILER_ENDS[16];
static LARGE_INTEGER G_PARS_PROFILER_ACCUMS[16];

static void parsProfilerStart(int i)
{
	QueryPerformanceCounter(&G_PARS_PROFILER_STARTS[i]);
}

static void parsProfilerEnd(int i)
{
	QueryPerformanceCounter(&G_PARS_PROFILER_ENDS[i]);
}

static void parsProfilerAccumInit(int i)
{
	memset(&G_PARS_PROFILER_ACCUMS[i], 0, sizeof(LARGE_INTEGER));
}

static void parsProfilerAccumulate(int i)
{
	G_PARS_PROFILER_ACCUMS[i].QuadPart += (G_PARS_PROFILER_ENDS[i].QuadPart - G_PARS_PROFILER_STARTS[i].QuadPart);
}

static double parsProfilerTime(int i)
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return (double)( (G_PARS_PROFILER_ENDS[i].QuadPart - G_PARS_PROFILER_STARTS[i].QuadPart)) / (double)freq.QuadPart;
}

static double parsProfilerAccumTime(int i)
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return (double)( G_PARS_PROFILER_ACCUMS[i].QuadPart) / (double)freq.QuadPart;
}

static void parsProfilerPrint(char * str, double timeval)
{
	char buf[512];
	sprintf_s(buf, 512, "Profiler>>%s: %f\n", str, timeval);
	OutputDebugStringA(buf);
}