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

#include "windows.h"

class CRepetitiveThread
{
public:
	typedef void Func(void*);
	CRepetitiveThread(Func* fn,void* ptr);
	~CRepetitiveThread();

	void Start();
	void WaitEnd();

private:
	Func* fn;
	void* ptr;
	HANDLE hThread;
	CRITICAL_SECTION waitToStart;
	CRITICAL_SECTION working;
	bool bWorking;
	bool bFinish;


	static DWORD WINAPI ThreadFunc(LPVOID ptr);
};