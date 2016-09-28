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

// ****************************************************************************
// *** CALLBACKS
// ****************************************************************************
void CPhotoshopLuce::DoParameters()
{
	bWillAskParams = true;
}

void CPhotoshopLuce::DoPrepare()
{
	// 
	oEffect.alpha.pBits = NULL;
	//
	if (pFilterRecord->parameters != NULL && (*pDataHandle) != 0)
	{
		LockHandles();
		InitData();
	}
	else
	{
		if (pFilterRecord->parameters == NULL)
			CreateParametersHandle();
		if ((*pDataHandle) == 0)
			CreateDataHandle();
		if (*pResult == noErr)
		{
			LockHandles();
			InitData();
			InitParameters();
		}
	}
	
	if (*pResult != noErr) return;
	int32 imageSize =	pDerivedInfos->imageSize.v *
						pDerivedInfos->imageSize.h; 
	// check memory needs, we needs:
	//  * DoneBuffer: imageSize of booleans
	//  * InputBuffer: imageSize of pixel (see pFilterRecord->depth)
	//  * OutputBuffer: imageSize of pixel (see pFilterRecord->depth)
	//  * AlphaBuffer(optional): imageSize of pixel (see pFilterRecord->depth)
	pFilterRecord->bufferSpace = imageSize;
	if( pDerivedInfos->bAlpha )
	{
		pFilterRecord->bufferSpace *= (1+pFilterRecord->depth/8);
	}
	// check maxSpace 
	int32 totalSize =	pFilterRecord->bufferSpace + 
						imageSize * (2*pFilterRecord->depth/8);
	// this is worst case and can be dropped considerably
	if (pFilterRecord->maxSpace > totalSize)
		pFilterRecord->maxSpace = totalSize;
}

void CPhotoshopLuce::ProgressCallback(float v)
{
	if( sPlugin.pFilterRecord->progressProc )
	{
		int32 value = sPlugin.pDerivedInfos->currPlane*1000 + (int32)(v*1000);
		int32 end = sPlugin.pFilterRecord->planes*1000;
		sPlugin.pFilterRecord->progressProc(value,end);
	}
}

void CPhotoshopLuce::DoStart()
{
	oEffect.width = pDerivedInfos->imageSize.h;
	oEffect.height = pDerivedInfos->imageSize.v;

	oEffect.pOpt = pParams;

	ReadRegistryParameters();
	ReadScriptParameters();

	if( bWillAskParams  && !DoDialog() )
	{
		bWillAskParams = false;
		*pResult = userCanceledErr;
		FreeMemory();
		return;
	}
	bWillAskParams = false;
	
	if( pDerivedInfos->bAlpha )
	{
		pDerivedInfos->currPlane = pDerivedInfos->nColorPlane;
		CallAdvance();
		oEffect.alpha.iPixelSize = pFilterRecord->depth/8;
		oEffect.alpha.iStride = oEffect.width*oEffect.alpha.iPixelSize;
		size_t planeSize = oEffect.width * oEffect.height * oEffect.alpha.iPixelSize;
		oEffect.alpha.pBits = (unsigned char*)CallAlloc(planeSize, &alphaBufferId);
		//memcpy( oEffect.alpha.pBits,  pFilterRecord->inData, planeSize);
		unsigned char *pSrc,*pDst;
		pSrc = (unsigned char*)(pFilterRecord->inData);
		pDst = oEffect.alpha.pBits;
		for(long y=0;y<oEffect.height;y++)
		{
			memmove( pDst,  pSrc, oEffect.alpha.iStride);
			pDst+=oEffect.alpha.iStride;
			pSrc+=pFilterRecord->inRowBytes;
		}
	}

	pDerivedInfos->currPlane=0;
	CallAdvance();
	if(*pResult != noErr) 
	{
		FreeMemory();
		return;
	}

	oEffect.SetProgressFn( ProgressCallback );
	CallLuce();
}

void CPhotoshopLuce::DoContinue()
{
	if (*pResult != noErr) return;
	pDerivedInfos->currPlane++;
	if( pDerivedInfos->currPlane < pFilterRecord->planes )
	{
		CallAdvance();
		if(*pResult != noErr) 
		{
			FreeMemory();
			return;
		}
		CallLuce();
	} else
		SetZeroRects();
}

void CPhotoshopLuce::DoFinish()
{
	LockHandles();
	WriteScriptParameters();
	WriteRegistryParameters();
	FreeMemory();
}

void CPhotoshopLuce::FreeMemory()
{
	if( pDerivedInfos->bAlpha )
		CallFree((void**)&oEffect.alpha.pBits, alphaBufferId);
}

void CPhotoshopLuce::CallLuce()
{
	oEffect.input.iPixelSize = pFilterRecord->depth/8;
	oEffect.output.iPixelSize = pFilterRecord->depth/8;

	oEffect.input.iStride = pFilterRecord->inRowBytes;
	oEffect.output.iStride = pFilterRecord->outRowBytes;

	oEffect.input.pBits = (unsigned char*)(pFilterRecord->inData);
	oEffect.output.pBits = (unsigned char*)(pFilterRecord->outData);
	if( pDerivedInfos->currPlane >= pDerivedInfos->nColorPlane )
	{
		oEffect.alpha.iPixelSize = 0;
		oEffect.alpha.iStride = 0;
		oEffect.alpha.pBits = 0;
	}

	oEffect.currentZero = pDerivedInfos->currPlane;
	if( oEffect.currentZero == pDerivedInfos->nColorPlane )
	{
		oEffect.currentZero = 3;
	} else
	if( oEffect.currentZero == pDerivedInfos->nColorPlane+1 )
		oEffect.currentZero = 4;


	switch( pFilterRecord->depth )
	{
	case 8: oEffect.Do8Bit(); break;
	case 16: oEffect.Do16Bit(); break;
	case 32: oEffect.Do32Bit(); break;
	}
}

// ****************************************************************************
// *** 
// ****************************************************************************
void CPhotoshopLuce::LockHandles()
{
	if (pFilterRecord->parameters == NULL || (*pDataHandle) == 0)
	{
		*pResult = filterBadParameters;
		return;
	}
	pParams = (LuceOptions*)pFilterRecord->handleProcs->lockProc
				(pFilterRecord->parameters, TRUE);
	pDerivedInfos = (sUsefulInfos*)pFilterRecord->handleProcs->lockProc
		        ((Handle)*pDataHandle, TRUE);
	if (pParams == NULL || pDerivedInfos == NULL)
	{
		*pResult = memFullErr;
		return;
	}
}

void CPhotoshopLuce::CreateParametersHandle(void)
{
	pFilterRecord->parameters = pFilterRecord->handleProcs->newProc
											(sizeof(LuceOptions));
	if (pFilterRecord->parameters == NULL)
		*pResult = memFullErr;
}

void CPhotoshopLuce::CreateDataHandle(void)
{
	Handle h = pFilterRecord->handleProcs->newProc(sizeof(sUsefulInfos));
	if (h != NULL)
		*pDataHandle = (intptr_t)h;
	else
		*pResult = memFullErr;
}

void CPhotoshopLuce::InitParameters()
{
	pParams->InitDefault();
}
	
void CPhotoshopLuce::InitData()
{
	if( pFilterRecord->bigDocumentData &&
		pFilterRecord->bigDocumentData->PluginUsing32BitCoordinates )
	{
		pDerivedInfos->imageSize = pFilterRecord->bigDocumentData->imageSize32;
	} else
	{
		pDerivedInfos->imageSize.h = pFilterRecord->imageSize.h;
		pDerivedInfos->imageSize.v = pFilterRecord->imageSize.v;
	}	
	bool bGrayScale = (
		(pFilterRecord->imageMode == plugInModeGrayScale) || 
		(pFilterRecord->imageMode == plugInModeGray16) ||
		(pFilterRecord->imageMode == plugInModeGray32) );
	pDerivedInfos->nColorPlane = bGrayScale? 1 : 3;
	pDerivedInfos->bAlpha = pFilterRecord->planes > pDerivedInfos->nColorPlane;
	pDerivedInfos->bMask = pFilterRecord->planes > (pDerivedInfos->nColorPlane+1);
}

void CPhotoshopLuce::CallAdvance()
{
	if( pFilterRecord->bigDocumentData &&
		pFilterRecord->bigDocumentData->PluginUsing32BitCoordinates )
	{
		pFilterRecord->bigDocumentData->inRect32.left = 0;
		pFilterRecord->bigDocumentData->inRect32.top = 0;
		pFilterRecord->bigDocumentData->inRect32.right = pFilterRecord->bigDocumentData->imageSize32.h;
		pFilterRecord->bigDocumentData->inRect32.bottom = pFilterRecord->bigDocumentData->imageSize32.v;
		pFilterRecord->bigDocumentData->outRect32 =
		pFilterRecord->bigDocumentData->maskRect32 =
			pFilterRecord->bigDocumentData->inRect32;

	} else
	{
		pFilterRecord->inRect.left = 0;
		pFilterRecord->inRect.top = 0;
		pFilterRecord->inRect.right = pFilterRecord->imageSize.h;
		pFilterRecord->inRect.bottom = pFilterRecord->imageSize.v;
		pFilterRecord->outRect = pFilterRecord->maskRect =
			pFilterRecord->inRect;

	}
	pFilterRecord->inLoPlane = pFilterRecord->inHiPlane = 
	pFilterRecord->outLoPlane = pFilterRecord->outHiPlane = 
		pDerivedInfos->currPlane;
	*pResult = pFilterRecord->advanceState();
}

void CPhotoshopLuce::SetZeroRects()
{
	if( pFilterRecord->bigDocumentData &&
		pFilterRecord->bigDocumentData->PluginUsing32BitCoordinates )
	{
		pFilterRecord->bigDocumentData->inRect32.left = 0;
		pFilterRecord->bigDocumentData->inRect32.top = 0;
		pFilterRecord->bigDocumentData->inRect32.right = 0;
		pFilterRecord->bigDocumentData->inRect32.bottom = 0;
		pFilterRecord->bigDocumentData->outRect32 =
			pFilterRecord->bigDocumentData->maskRect32 =
			pFilterRecord->bigDocumentData->inRect32;

	} else
	{
		pFilterRecord->inRect.left = 0;
		pFilterRecord->inRect.top = 0;
		pFilterRecord->inRect.right = 0;
		pFilterRecord->inRect.bottom = 0;
		pFilterRecord->outRect = pFilterRecord->maskRect =
			pFilterRecord->inRect;

	}
}

void* CPhotoshopLuce::CallAlloc(size_t size,BufferID* bufferID)
{
	if( sSPBuffer.IsSupported() )
	{
		return sSPBuffer->New(NULL,(unsigned32)size);
	} else
	if( pFilterRecord->bufferProcs )
	{
		pFilterRecord->bufferProcs->allocateProc((int32)size,bufferID);
		return pFilterRecord->bufferProcs->lockProc(*bufferID,true);
	} else
	{
		return malloc(size);
	}
}

void CPhotoshopLuce::CallFree(void** ptr,BufferID bufferID)
{
	if((*ptr) == NULL ) return;
	if( sSPBuffer.IsSupported() )
	{
		sSPBuffer->Dispose((Ptr*)(ptr));
	} else
	if( pFilterRecord->bufferProcs )
	{
		pFilterRecord->bufferProcs->unlockProc(bufferID);
		pFilterRecord->bufferProcs->freeProc(bufferID);
	} else
	{
		free(*ptr);
		*ptr = NULL;
	}
}
