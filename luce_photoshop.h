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

#include <PIFilter.h>
#include <SPPlugs.h>
#include <SPAccess.h>
#include <PIBufferSuite.h>
#include <windows.h>

#include "PhotoUtilsSuites.h"
#include "luce.h"

class CPhotoshopLuce
{
public:
	static CPhotoshopLuce& Get() { return sPlugin; }
	void SetDLLInstance(HINSTANCE hi) { hInstance = hi; }
	static SPBasicSuite *GetBasicSuite() { return sPlugin.pSPBasic; }
	static FilterRecord* GetFilterRecord() { return sPlugin.pFilterRecord; }
	static const PSBufferSuite1* GetBufferSuite() { return (*sPlugin.sSPBuffer); }
	static VPoint GetImageSize() { return sPlugin.pDerivedInfos->imageSize; }
	static int GetNColors() { return sPlugin.pDerivedInfos->nColorPlane; }
	static bool IsImageGrayScale() { return sPlugin.pDerivedInfos->nColorPlane == 1; }
	static bool HasImageAlpha() { return sPlugin.pDerivedInfos->bAlpha; }
	static bool HasImageMask() { return sPlugin.pDerivedInfos->bMask; }

	void Main(const int16 selector,FilterRecordPtr filterRecord,intptr_t * data,int16 * result);


	void DoAbout();
	void DoParameters();
	void DoPrepare();
	void DoStart();
	void DoContinue();
	void DoFinish();

	void* CallAlloc(size_t size,BufferID* bufferID);
	void CallFree(void** ptr,BufferID bufferID);

private:
	void FreeMemory();

	static void ProgressCallback(float v);

	//! Lock the handles and get the pointers for gData and gParams
	//! Set the global error, *gResult, if there is trouble
	void LockHandles();
	//! Create a handle to our Parameters structure. Photoshop will take ownership of
	//! this handle and delete it when necessary.
	void CreateParametersHandle();
	//
	void CreateDataHandle();
	//
	void InitParameters();
	//
	void InitData();
	//
	void CallAdvance();
	// 
	void SetZeroRects();
	//
	void CallLuce();	
	// inside luce_photoshop_dialog.cpp
	bool DoDialog();
	// inside luce_photoshop_params.cpp
	OSErr ReadRegistryParameters();
	OSErr WriteRegistryParameters();
	OSErr ReadScriptParameters();
	OSErr WriteScriptParameters();


	CPhotoshopLuce(); 

	static CPhotoshopLuce sPlugin;

	// windows's instance
	HINSTANCE hInstance;
	// 
	SPBasicSuite *pSPBasic;
	// 
	AutoSuite<PSBufferSuite1> sSPBuffer;
	// 
	FilterRecord* pFilterRecord;
	//
	intptr_t* pDataHandle;
	//
	int16* pResult;
	//
	LuceOptions* pParams;
	//
	CLuce oEffect;
	//
	BufferID doesBufferId;
	BufferID alphaBufferId;

	bool bWillAskParams;
	struct sUsefulInfos
	{
		int16 currPlane;
		VPoint imageSize; 
		int nColorPlane;
		bool bAlpha;
		bool bMask;
	} *pDerivedInfos;
};
