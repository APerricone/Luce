// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "timer.h"

CTimer::CTimer()
{
#ifdef WIN32
    QueryPerformanceFrequency(&frequency);
#endif
}

void CTimer::Start()
{
#ifdef WIN32
    QueryPerformanceCounter(&startValue);
#else
    gettimeofday(&startValue, NULL);
#endif
}

double CTimer::GetTime()
{
#ifdef WIN32
	LARGE_INTEGER current;
    QueryPerformanceCounter(&current);

    return (double)(current.QuadPart - startValue.QuadPart) / (double)frequency.QuadPart;
#else
	timeval current;
    gettimeofday(&current, NULL);

	return 
		double(current.tv_sec - startValue.tv_sec) +
		double(current.tv_usec - startValue.tv_usec) / 1000000.;
#endif
}
