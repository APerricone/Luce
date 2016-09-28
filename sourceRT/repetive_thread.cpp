// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "repetive_thread.h"

//             | T|*      ~|~|~----|-|-|-- ~|~|~-**
// waitToStart | C|--------|-|-    | | | ~--|-|- 
//             | 0|        | |     | | |    | |               
//             |  |        | |     | | |    | |                     
//             | T|*     --|-|---  | |~|~---|-|  **                   
//  working    | C| - -    | |  ~--|-|-|-   | |
//             | 0|- - -   | |     | | |    | |           
//                |^CREATE |1|^    |2|3|^   |1|^
//                  ^^^^     |START| | |WAIT| |FINISH|
//					Here the caller thread wait for 
//					repetitive to lock working
// T Repetitive Thread
// C Caller thread
// 1 state before Start
// 2 Repetitive Thread is working
// 3 state before wait
// ~ Wait for locking
// - Lock
// * Thread does not exist
// (see repetive_thread.svg for a better version)
// 

CRepetitiveThread::CRepetitiveThread(Func* _fn,void* _ptr)
{
	InitializeCriticalSection(&waitToStart);
	InitializeCriticalSection(&working);
	bWorking = false;
	bFinish = false;
	fn = _fn;
	ptr = _ptr;
	//
	EnterCriticalSection(&waitToStart);
	//
	hThread = CreateThread(0,0,ThreadFunc,this,0,0);
	// wait for start
	BOOL b = TRUE;
	while(b)
	{
		b = TryEnterCriticalSection(&working);
		if( b )
		{
			LeaveCriticalSection(&working);
			SwitchToThread();
		}
	}
}

CRepetitiveThread::~CRepetitiveThread()
{
	bFinish = true;
	LeaveCriticalSection(&waitToStart);
	WaitForMultipleObjects(1,&hThread,TRUE,INFINITE);
	//
	DeleteCriticalSection(&waitToStart);
	DeleteCriticalSection(&working);
}

void CRepetitiveThread::Start()
{
	bWorking = true;
	LeaveCriticalSection(&waitToStart);
	EnterCriticalSection(&working);
}

void CRepetitiveThread::WaitEnd()
{
	LeaveCriticalSection(&working);
	EnterCriticalSection(&waitToStart);
}

DWORD WINAPI CRepetitiveThread::ThreadFunc(LPVOID ptr)
{
	CRepetitiveThread* pThis = (CRepetitiveThread*)ptr;
	EnterCriticalSection(&pThis->working);
	while(1)
	{
		EnterCriticalSection(&pThis->waitToStart);
		LeaveCriticalSection(&pThis->working);

		if( pThis->bFinish ) break;

		if( pThis->bWorking ) 
			pThis->fn(pThis->ptr);
		else
			SwitchToThread();

		pThis->bWorking = false;

		EnterCriticalSection(&pThis->working);
		LeaveCriticalSection(&pThis->waitToStart);
	}
	return 0;
}
