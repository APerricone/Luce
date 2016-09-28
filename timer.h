// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#pragma once

#ifdef WIN32   // Windows system specific
#include <windows.h>
#else          // Unix based system specific
#include <sys/time.h>
#endif

class CTimer
{
public:
	CTimer();

	void Start();
	double GetTime();

private:

#ifdef WIN32
    LARGE_INTEGER frequency;
    LARGE_INTEGER startValue;
#else
    timeval startValue;
#endif
};
