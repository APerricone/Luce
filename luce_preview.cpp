// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "luce_preview.h"

#include "luce_photoshop.h"
#include <PIChannelPortsSuite.h>

CLucePreview::CLucePreview()
{
	currSrc = NULL;
	currDst = NULL;
}

void CLucePreview::Init(HWND hDlg,int idArea)
{
	AutoSuite<PSChannelPortsSuite1> sPSChannelPortsSuite(kPSChannelPortsSuite,kPSChannelPortsSuiteVersion3);

	GetWindowRect(GetDlgItem(hDlg, idArea), &totalRect);	
	ScreenToClient(hDlg, (LPPOINT)&totalRect);
	ScreenToClient(hDlg, (LPPOINT)&(totalRect.right));

	nChannel = CPhotoshopLuce::GetNColors();
	bAlpha = CPhotoshopLuce::HasImageAlpha();
	if( bAlpha ) nChannel++;
	long maxW = (totalRect.right - totalRect.left);
	long maxH = (totalRect.bottom - totalRect.top);

	currSrc = (unsigned char*)CPhotoshopLuce::Get().CallAlloc( maxW * maxH * nChannel, &srcBufferId );
	currDst = (unsigned char*)CPhotoshopLuce::Get().CallAlloc( maxW * maxH * nChannel, &dstBufferId );
	AdjustRect();

	memset(&psPixelMap,0,sizeof(PSPixelMap));
	psPixelMap.version = 1;
	psPixelMap.bounds.left = 0;
	psPixelMap.bounds.top = 0;
	psPixelMap.bounds.right = previewRect.right - previewRect.left;
	psPixelMap.bounds.bottom = previewRect.bottom - previewRect.top;
	psPixelMap.imageMode = (CPhotoshopLuce::GetNColors()==1)? plugInModeGrayScale : plugInModeRGBColor;
	psPixelMap.rowBytes = (previewRect.right - previewRect.left) * nChannel;
	psPixelMap.colBytes = nChannel;
	psPixelMap.planeBytes = 1;
	psPixelMap.baseAddr = currDst;
	if( bAlpha )
	{
		psPixelMask.next = 0;
		psPixelMask.maskData = currDst+3; 
		psPixelMask.rowBytes = (previewRect.right - previewRect.left) * nChannel; 
		psPixelMask.colBytes = nChannel; 
		psPixelMask.maskDescription = kSimplePSMask; 

		psPixelMap.masks = &psPixelMask;
	}

	readRect.left = 0;
	readRect.top = 0;
	readRect.right = CPhotoshopLuce::GetImageSize().h;
	readRect.bottom = CPhotoshopLuce::GetImageSize().v;
	PSScaling scale;
	scale.sourceRect.left = 0;
	scale.sourceRect.top = 0;
	scale.sourceRect.right = CPhotoshopLuce::GetImageSize().h;
	scale.sourceRect.bottom = CPhotoshopLuce::GetImageSize().v;
	scale.destinationRect.left = 0;
	scale.destinationRect.top = 0;
	scale.destinationRect.right = (previewRect.right - previewRect.left);
	scale.destinationRect.bottom = (previewRect.bottom - previewRect.top);
	PixelMemoryDesc destDesc;
	destDesc.data = currSrc;
	destDesc.colBits = nChannel * 8;
	destDesc.rowBits = scale.destinationRect.right * destDesc.colBits; 
	destDesc.depth = 8;
	
	ReadChannelDesc *src[5] = { 0,0,0,0,0 };
	if( CPhotoshopLuce::GetFilterRecord()->documentInfo )
	{
		if( CPhotoshopLuce::GetNColors() == 1 )
		{
			src[0] = CPhotoshopLuce::GetFilterRecord()->documentInfo->targetCompositeChannels;
			src[2] = src[1] = src[0];
		} else
		{
			src[0] = CPhotoshopLuce::GetFilterRecord()->documentInfo->targetCompositeChannels;
			src[1] = src[0]->next;
			src[2] = src[1]->next;
		}
		src[CPhotoshopLuce::GetNColors()] = CPhotoshopLuce::GetFilterRecord()->documentInfo->targetTransparency;
		src[CPhotoshopLuce::GetNColors()+1] = CPhotoshopLuce::GetFilterRecord()->documentInfo->targetLayerMask;
	}

	for( unsigned char i=0;i<nChannel;i++)
		if( src[i]!= NULL ) // preview error :(
		{
			destDesc.bitOffset = i*8;
			if( sPSChannelPortsSuite.IsSupported() )
			{
				sPSChannelPortsSuite->ReadScaledPixels(src[i]->port,&readRect,&scale,&destDesc);
			} else
			if( CPhotoshopLuce::GetFilterRecord()->channelPortProcs != NULL )
			{
				CPhotoshopLuce::GetFilterRecord()->channelPortProcs->readPixelsProc(
					src[i]->port, &scale, &scale.destinationRect, &destDesc, &readRect );
			}
		}

	readRect.right = (previewRect.right - previewRect.left);
	readRect.bottom = (previewRect.bottom - previewRect.top);
}

CLucePreview::~CLucePreview()
{
	if(currSrc) CPhotoshopLuce::Get().CallFree((void**)&currSrc,srcBufferId);
	if(currDst) CPhotoshopLuce::Get().CallFree((void**)&currDst,dstBufferId);
}

void CLucePreview::AdjustRect()
{
	previewRect.left = totalRect.left;
	previewRect.top = totalRect.top;
	previewRect.right = totalRect.right;
	previewRect.bottom = totalRect.bottom;

	long oldWidth = (totalRect.right - totalRect.left);
	long oldHeight = (totalRect.bottom - totalRect.top);
	long newWidth = oldWidth;
	long newHeight = oldHeight;

	if(	oldWidth > CPhotoshopLuce::GetImageSize().h && 
		oldHeight > CPhotoshopLuce::GetImageSize().v )
	{
		newWidth = (CPhotoshopLuce::GetImageSize().h);
		newHeight = (CPhotoshopLuce::GetImageSize().v);
	} else
	{
		float imageAspectRatio = 
			(float)(CPhotoshopLuce::GetImageSize().h) / 
			(float)(CPhotoshopLuce::GetImageSize().v);
		float previewAspectRatio = (float)(oldWidth) / (float)(oldHeight);
		if( previewAspectRatio > imageAspectRatio )
		{ // reduce width
			newWidth = long(oldHeight * imageAspectRatio );
		}
		if( previewAspectRatio < imageAspectRatio )
		{ // reduce height
			newHeight = long(oldWidth / imageAspectRatio);
		}
	}
	if( oldWidth != newWidth )
	{
		long center = (totalRect.right + totalRect.left) / 2;
		previewRect.left = center - newWidth/2;
		previewRect.right = center + newWidth/2;
		if( newWidth&1 ) previewRect.right++;
	}
	if( oldHeight != newHeight )
	{
		long center = (totalRect.bottom - totalRect.top) / 2;
		previewRect.top = center - newHeight/2;
		previewRect.bottom = center + newHeight/2;
		if( newHeight&1 ) previewRect.bottom++;
	}
}

void CLucePreview::UpdatePreview(const LuceOptions& pOpt)
{
	CLuce luce;
	luce.width = (previewRect.right - previewRect.left);
	luce.height = (previewRect.bottom - previewRect.top);
	luce.input.iPixelSize = nChannel;
	luce.input.iStride = nChannel * luce.width;
	luce.input.pBits = currSrc;
	luce.output.iPixelSize = nChannel;
	luce.output.iStride = nChannel * luce.width;
	luce.output.pBits = currDst;
	
	luce.pOpt = &pOpt;
	if( bAlpha )
	{
		luce.alpha.pBits = currSrc+3;
		luce.alpha.iStride = nChannel * luce.width;
		luce.alpha.iPixelSize = nChannel;
	} else
	{
		luce.alpha.pBits = NULL;
	}
	
	for( unsigned char i=0;i<nChannel;i++)
	{
		if( bAlpha && i==nChannel-1)
		{
			luce.alpha.pBits = NULL;
		}
		luce.currentZero = i;
		luce.Do8Bit();
		luce.input.pBits++;
		luce.output.pBits++;
	}
}

void CLucePreview::CallDraw(HWND hDlg)
{
	InvalidateRect(hDlg,&previewRect,FALSE);
}

bool CLucePreview::IsInside(long x,long y)
{
	if( x < totalRect.left ) return false;
	if( x > totalRect.right ) return false;
	if( y < totalRect.top ) return false;
	if( y > totalRect.bottom ) return false;
	return true;
}

bool CLucePreview::ChangePos(long x,long y,LuceOptions *pOpt)
{
	if( x < totalRect.left ) return false;
	if( x > totalRect.right ) return false;
	if( y < totalRect.top ) return false;
	if( y > totalRect.bottom ) return false;
	if( pOpt->IsPointLight() )
	{
		double lx = double(x - previewRect.left) / double(previewRect.right - previewRect.left);
		double ly = double(y - previewRect.top) / double(previewRect.bottom - previewRect.top);
		pOpt->SetCenter(lx,ly);
	} else
	{
		long dx = x - (totalRect.right + totalRect.left)/2;
		long dy = y - (totalRect.top + totalRect.bottom)/2;
		if( (dx != 0) && (dy != 0))
			pOpt->SetDirection(float(dx),float(dy));
	}
	return true;
}

bool CLucePreview::Pick(long x,long y,LuceOptions *pOpt)
{
	if( x < previewRect.left ) return false;
	if( x > previewRect.right ) return false;
	if( y < previewRect.top ) return false;
	if( y > previewRect.bottom ) return false;
	
	AutoSuite<PSChannelPortsSuite1> sPSChannelPortsSuite(kPSChannelPortsSuite,kPSChannelPortsSuiteVersion3);

	VRect tmpReadRect;
	long srcX = MulDiv((x-previewRect.left),CPhotoshopLuce::GetImageSize().h,(previewRect.right-previewRect.left));
	long srcY = MulDiv((y-previewRect.top),CPhotoshopLuce::GetImageSize().v,(previewRect.bottom-previewRect.top));
	tmpReadRect.left = srcX;
	tmpReadRect.top = srcY;
	tmpReadRect.right = srcX+1;
	tmpReadRect.bottom = srcY+1;

	union
	{
		unsigned char c;
		unsigned short s;
		float f;
	} d;
	PixelMemoryDesc destDesc;
	destDesc.data = &d;
	destDesc.colBits = 32;
	destDesc.rowBits = 32;
	destDesc.bitOffset = 0;
	destDesc.depth = CPhotoshopLuce::GetFilterRecord()->depth;
	
	ReadChannelDesc *src[5] = { 0,0,0,0,0 };
	if( CPhotoshopLuce::GetFilterRecord()->documentInfo )
	{
		if( CPhotoshopLuce::GetNColors() == 1 )
		{
			src[0] = CPhotoshopLuce::GetFilterRecord()->documentInfo->targetCompositeChannels;
			src[2] = src[1] = src[0];
		} else
		{
			src[0] = CPhotoshopLuce::GetFilterRecord()->documentInfo->targetCompositeChannels;
			src[1] = src[0]->next;
			src[2] = src[1]->next;
		}
		src[3] = CPhotoshopLuce::GetFilterRecord()->documentInfo->targetTransparency;
		src[4] = CPhotoshopLuce::GetFilterRecord()->documentInfo->targetLayerMask;
	}

	PSScaling scale;
	scale.sourceRect = tmpReadRect;
	scale.destinationRect.left = 0;
	scale.destinationRect.top = 0;
	scale.destinationRect.right = 1;
	scale.destinationRect.bottom = 1;

	bool retValue = true;
	for(int i=0;i<5;i++)
	{
		if( src[i] != NULL )
		{
			SPErr e;
			if( sPSChannelPortsSuite.IsSupported() )
			{
				e = sPSChannelPortsSuite->ReadPixelsFromLevel(src[i]->port,0,
					&tmpReadRect,&destDesc);
			} else
			if( CPhotoshopLuce::GetFilterRecord()->channelPortProcs )
			{
				e = CPhotoshopLuce::GetFilterRecord()->channelPortProcs->readPixelsProc(
					src[i]->port, &scale, &scale.sourceRect, &destDesc, &readRect );
			} else
				e = kSPBadParameterError;
			if( e == noErr )
			{
				switch(CPhotoshopLuce::GetFilterRecord()->depth)
				{
				case 8: pOpt->SetZero(i, d.c/255.f); break;
				case 16: pOpt->SetZero(i, d.s/32768.f); break;
				case 32: pOpt->SetZero(i, d.f); break;
				}
			}
			else
				retValue = false;
		}
	}

	return retValue;
}

void CLucePreview::Draw(HDC hDC)
{
	CPhotoshopLuce::GetFilterRecord()->displayPixels(
		&psPixelMap,&readRect,
		previewRect.top,previewRect.left,(void*)hDC);
}
