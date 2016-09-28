// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "luce_photoshop.h"

#include "DialControl.h"

#include <PIAbout.h>

CPhotoshopLuce CPhotoshopLuce::sPlugin;

CPhotoshopLuce::CPhotoshopLuce() : 
	pSPBasic(NULL),
	sSPBuffer(kPSBufferSuite, kPSBufferSuiteVersion1),
	pFilterRecord(NULL),
	pDataHandle(NULL),
	pResult(NULL),
	pParams(NULL),
	bWillAskParams(false),
	pDerivedInfos(NULL)
{
}


void CPhotoshopLuce::Main(const int16 selector,FilterRecordPtr filterRecord,intptr_t * data,int16 * result)
{
	// update our global parameters
	pFilterRecord = filterRecord;
	pDataHandle = data;
	pResult = result;

	if (selector == filterSelectorAbout)
	{
		pSPBasic = ((AboutRecord*)pFilterRecord)->sSPBasic;
	}
	else
	{
		pSPBasic = pFilterRecord->sSPBasic;

		if (pFilterRecord->bigDocumentData != NULL)
			pFilterRecord->bigDocumentData->PluginUsing32BitCoordinates = true;
	}

	// do the command according to the selector
	switch (selector)
	{
		case filterSelectorAbout:
			DoAbout();
			break;
		case filterSelectorParameters:
			DoParameters();
			break;
		case filterSelectorPrepare:
			DoPrepare();
			break;
		case filterSelectorStart:
			DoStart();
			break;
		case filterSelectorContinue:
			DoContinue();
			break;
		case filterSelectorFinish:
			DoFinish();
			break;
		default:
			break;
	}
	
	// unlock our handles used by gData and gParams
	if (selector != filterSelectorAbout)
	{
		if ((*pDataHandle) != 0)
			pFilterRecord->handleProcs->unlockProc((Handle)*pDataHandle);
		if (pFilterRecord->parameters != NULL)
			pFilterRecord->handleProcs->unlockProc(pFilterRecord->parameters);
	}
}

// ****************************************************************************
// Utils global functions
BOOL APIENTRY DllMain(HANDLE hModule,
					  DWORD ul_reason_for_call,
					  LPVOID /*lpReserved*/)
{
	// very odd, when running SimpleFormat and selecting multiple images
	// in the open dialog I get dettach messages at about the 7th image ?????
	// the detach messages come with a valid hModule so no if ?????
	// if (ul_reason_for_call == DLL_PROCESS_ATTACH ||
	// 	   ul_reason_for_call == DLL_THREAD_ATTACH)
	CPhotoshopLuce::Get().SetDLLInstance(static_cast<HINSTANCE>(hModule));
	// else
	// dllInstance = NULL;
	return  true;
}

#ifndef DLLExport
	#define DLLExport extern "C" __declspec(dllexport)
#endif

DLLExport MACPASCAL void PluginMain(const int16 selector,FilterRecordPtr filterRecord,intptr_t * data,int16 * result)
{
	CPhotoshopLuce::Get().Main(selector,filterRecord,data,result);
}
