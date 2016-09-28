// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#include "luce.h"
#include <cstring>
#include <math.h>
#include <stdio.h>
#include <float.h>

#include <vector>
#include <windows.h>

void LuceOptions::SetNumThreadsToUse(unsigned short n)
{
	if( n < 1 ) 
		numThreadsToUse = 1; 
	else
		numThreadsToUse = n; 

}

void LuceOptions::SetDirection(float x,float y)
{
	float l=sqrtf(x*x+y*y);
	direction.x=x/l; 
	direction.y=y/l; 
}

// templates
template<> float CLuce::GetValue<unsigned char>(void* ptr,int idx)
{
	return ((unsigned char*)(ptr))[idx] / 255.f;
}

template<> float CLuce::GetValue<unsigned short>(void* ptr,int idx)
{
	return ((unsigned short*)(ptr))[idx] / 65535.f;
}

template<> float CLuce::GetValue<float>(void* ptr,int idx)
{
	return ((float*)(ptr))[idx];
}

template<> void CLuce::SetValue<unsigned char>(void* ptr,int idx,float v)
{
	unsigned char* p = &((unsigned char*)(ptr))[idx];
	if( v > 1.f ) *p = 255; else
	if( v < 0.f ) *p = 0; else
		*p = (unsigned char)(v*255.f);
}

template<> void CLuce::SetValue<unsigned short>(void* ptr,int idx,float v)
{
	unsigned short* p = &((unsigned short*)(ptr))[idx];
	if( v > 1.f ) *p = 65535; else
	if( v < 0.f ) *p = 0; else
		*p = (unsigned short)(v*65535.f);
}

template<> void CLuce::SetValue<float>(void* ptr,int idx,float v)
{
	((float*)(ptr))[idx] = v;
}

template<typename T>
void CLuce::Line(long px,long py)
{
	// check start point
	long x;
	long y;
	unsigned char* srcPtr;
	unsigned char* dstPtr;
	unsigned char* alphaPtr;
	if( pOpt->IsPointLight() )
	{
		x = lx;
		y = ly;
		srcPtr = lightInput;
		dstPtr = lightOutput;
		alphaPtr = lightAlpha;
	} else
	{
		x = px + lx;
		y = py + ly;
		SetupLightPointers(x,y,srcPtr,dstPtr,alphaPtr);
	}
	// do line
	long dx=px-x; //< delta x
	long dy=py-y; //< delta y
	unsigned long nx=dx<0? -dx :dx; //< number of pixel along x
	unsigned long ny=dy<0? -dy :dy; //< number of pixel along y
	unsigned long np; //< number of pixel along the line
	long xDeltas[2];
	long yDeltas[2];
	ptrdiff_t srcDeltas[2];
	ptrdiff_t dstDeltas[2];
	ptrdiff_t alphaDeltas[2];
	unsigned long increment;
	int xIdx;
	if(nx>ny)
	{
		np = nx;
		increment = ny;
		xIdx = 0;
	} else
	{
		np = ny;
		increment = nx;
		xIdx = 1;
	}
	xDeltas[1-xIdx] = 0;
	yDeltas[xIdx] = 0;
	if( dx > 0)
	{
		xDeltas[xIdx] = 1;
		srcDeltas[xIdx] = input.iPixelSize;
		dstDeltas[xIdx] = output.iPixelSize;
		alphaDeltas[xIdx] = alpha.iPixelSize;
	} else
	{
		xDeltas[xIdx] = -1;
		srcDeltas[xIdx] = -input.iPixelSize;
		dstDeltas[xIdx] = -output.iPixelSize;
		alphaDeltas[xIdx] = -alpha.iPixelSize;
	}
	if( dy > 0 )
	{
		yDeltas[1-xIdx] = 1;
		srcDeltas[1-xIdx] = input.iStride;
		dstDeltas[1-xIdx] = output.iStride;
		alphaDeltas[1-xIdx] = alpha.iStride;
	} else
	{
		yDeltas[1-xIdx] = -1;
		srcDeltas[1-xIdx] = -input.iStride;
		dstDeltas[1-xIdx] = -output.iStride;
		alphaDeltas[1-xIdx] = -alpha.iStride;
	}
	float sum[5]={0.f,0.f,0.f,0.f,0.f};
	float n=0.f;
	int lightFormState = 0;
	unsigned long fraction=(increment>>1);
	for(unsigned long i=1;i<=np;i++)
	{
		x+=xDeltas[0];
		y+=yDeltas[0];
		fraction+=increment;
		srcPtr+=srcDeltas[0];
		dstPtr+=dstDeltas[0];
		alphaPtr+=alphaDeltas[0];
		if(fraction>=np)
		{
			x+=xDeltas[1];
			y+=yDeltas[1];
			srcPtr+=srcDeltas[1];
			dstPtr+=dstDeltas[1];
			alphaPtr+=alphaDeltas[1];
			fraction-=np;
		}
		if( x >= 0 && x < width &&
			y >= 0 && y < height )
		{
			// update n
			float a = 1;
			if( alphaPtr!=0)
			{
				a = GetValue<T>(alphaPtr,0);
			}
			n += a;
			// update output
			for(long c=0;c<numchannels;c++)
			{
				float v = GetValue<T>(srcPtr,c);
				if( pOpt->IsZeroMode() )
				{
					v -= pOpt->GetZero(currentZero+c);
				} else
				{
					// update Light form
					switch(lightFormState)
					{
					case 0: if( v > 0.5f ) lightFormState++; break;
					case 1: if( v < 0.5f ) lightFormState++; break;
					case 2: v=-v;
					}
				}
				v *= a;
				if( v < 0 ) 
				{
					v *= negativeFactor;
				} else
				{
					v *= positiveFactor;
				}
				float l,mn=n;
				if( c == alphaInInputChannel ) mn = (float)i;
				if( pOpt->IsDirectionaLight() ) 
				{	// directional light does not attenuate 
					// between point and light pixel
					v*=mn/dirScale;
				}
				if( pOpt->IsLinearAttenuation() ) 
				{
					sum[c] += v;
					l = sum[c]/mn;
				} else
				{
					sum[c] += v*(2*mn-1);
					l = sum[c]/(mn*mn);
				}
				SetValue<T>(dstPtr,c,l);
			}
		}
	}
}

template<typename T> 
unsigned long CLuce::ThreadLightFunc(void* ptr)
{
	ThreadLightData* pData = (ThreadLightData*)ptr;
	int w = pData->pLuce->width;
	int h = pData->pLuce->height;
	long idx = 0;
	if( pData->pLuce->doLeft || pData->pLuce->doTop ) 
	{
		if( idx >= pData->startPixel && idx < pData->endPixel )
			pData->pLuce->Line<T>(0,0);
		idx++;
	}
	if( pData->pLuce->doTop )
	{
		for(int x=1;x<w-1;x++) 
		{
			if( idx >= pData->startPixel && idx < pData->endPixel )
				pData->pLuce->Line<T>(x,0);
			idx++;
		}
	}
	if( pData->pLuce->doRight || pData->pLuce->doTop ) 
	{
		if( idx >= pData->startPixel && idx < pData->endPixel )
			pData->pLuce->Line<T>(w-1,0);
		idx++;
	}
	if( pData->pLuce->doRight )
	{
		for(int y=1;y<h-1;y++)
		{
			if( idx >= pData->startPixel && idx < pData->endPixel )
				pData->pLuce->Line<T>(w-1,y);
			idx++;
		}
	}
	if( pData->pLuce->doRight || pData->pLuce->doBottom ) 
	{
		if( idx >= pData->startPixel && idx < pData->endPixel )
			pData->pLuce->Line<T>(w-1,h-1);
		idx++;
	}
	if( pData->pLuce->doBottom )
	{
		for(int x=w-2;x>=1;x--) 
		{
			if( idx >= pData->startPixel && idx < pData->endPixel )
				pData->pLuce->Line<T>(x,h-1);
			idx++;
		}
	}
	if( pData->pLuce->doLeft || pData->pLuce->doBottom ) 
	{
		if( idx >= pData->startPixel && idx < pData->endPixel )
			pData->pLuce->Line<T>(0,h-1);
		idx++;
	}
	if( pData->pLuce->doLeft )
	{
		for(int y=h-2;y>=1;y--)
		{
			if( idx >= pData->startPixel && idx < pData->endPixel )
				pData->pLuce->Line<T>(0,y);
			idx++;
		}
	}
	return 1;
}


template<typename T> 
void CLuce::DoPoint()
{
	lx = long(pOpt->center.x * width);
	ly = long(pOpt->center.y * height);
	SetupLightPointers(lx,ly,lightInput,lightOutput,lightAlpha);
	if( lx >= 0 && lx < width &&
		ly >= 0 && ly < height )
	{
		for(long c=0;c<numchannels;c++)
		{
			SetValue<T>(lightOutput,c,0.f);
		}
	}
	doLeft = pOpt->center.x>0;
	doTop = pOpt->center.y>0;
	doRight = pOpt->center.x<1;
	doBottom = pOpt->center.y<1;
	CallThreads<T>();
}

template<typename T> 
void CLuce::DoDirectional()
{
	// calculate lx and ly (here are deltas)
	float dirAspect;
	if( fabsf(pOpt->direction.y)>1e-3f )
		dirAspect = fabsf(pOpt->direction.x) / fabsf(pOpt->direction.y);
	else
		dirAspect = FLT_MAX;
	float imageAspect = float(width) / float(height);
	if( dirAspect < imageAspect )
	{
		if( pOpt->direction.y < 0 ) ly = -height;
							   else ly = height;
		lx = long(height * pOpt->direction.x / fabsf(pOpt->direction.y));
		// note: pOpt->direction.y is not near 0 because dirAspect is not infinite 
		// (it is less than imageAspect)
	} else
	{
		if( pOpt->direction.x < 0 ) lx = -width;
							   else lx = width;
		ly = long(width * pOpt->direction.y / fabsf(pOpt->direction.x));
		// same as before: pOpt->direction.x is not near 0 because dirAspect is 
		// not near 0 (it is greater than imageAspect)
	}
	// calculate dir scale
	if( width > height )
		dirScale = float(width)/4.f;
	else
		dirScale = float(height)/4.f;
	// select sides
	doLeft	 = pOpt->direction.x> 1e-3f;
	doTop	 = pOpt->direction.y> 1e-3f;
	doRight  = pOpt->direction.x<-1e-3f;
	doBottom = pOpt->direction.y<-1e-3f;;
	CallThreads<T>();
}

CLuce::ThreadLightData::ThreadLightData(
	CLuce *_pLuce,long _startPixel,long _endPixel)
{
	pLuce		= _pLuce		;
	startPixel	= _startPixel	;
	endPixel	= _endPixel		;
}

template<typename T> 
void CLuce::CallThreads()
{
	// count pixels to do
	long nPixelToDo = 0;
	if( doLeft ) nPixelToDo+= height-2;
	if( doTop ) nPixelToDo+= width-2;
	if( doRight ) nPixelToDo+= height-2;
	if( doBottom ) nPixelToDo+= width-2;
	if( doLeft || doTop ) nPixelToDo++;
	if( doRight || doTop ) nPixelToDo++;
	if( doLeft || doBottom ) nPixelToDo++;
	if( doRight || doBottom ) nPixelToDo++;
	// init ThreadLightData
	std::vector<ThreadLightData> tmp;
	tmp.reserve(pOpt->numThreadsToUse);
	int start = 0;
	for(int i=0;i<pOpt->numThreadsToUse;i++)
	{
		int end = MulDiv(nPixelToDo, (i+1), pOpt->numThreadsToUse );
		tmp.push_back(ThreadLightData(this,start, end));
		start = end;
	}
	if( pOpt->numThreadsToUse > 1 )
	{
		std::vector<HANDLE> tt;
		tt.resize(pOpt->numThreadsToUse);
		for(int i=0;i<pOpt->numThreadsToUse;i++)
			tt[i] =CreateThread(0,0,ThreadLightFunc<T>,&tmp[i],0,0);
		WaitForMultipleObjects(pOpt->numThreadsToUse,&tt[0],TRUE,INFINITE);
	} else
	{
		ThreadLightFunc<T>(&tmp[0]);
	}
}

CLuce::ThreadAddSourceData::ThreadAddSourceData(CLuce *_pLuce,
		long _left,long _top,long _right,long _bottom,
		float* _srcDelta,float* _srcPositiveFactor,
		float* _srcNegativeFactor)
{
	pLuce				= _pLuce				;
	left				= _left					;
	top					= _top					;
	right				= _right				;
	bottom				= _bottom				;
	for(int i=0;i<5;i++)
	{
		srcDelta[i]				= _srcDelta[i]			;
		srcPositiveFactor[i]	= _srcPositiveFactor[i]	;
		srcNegativeFactor[i]	= _srcNegativeFactor[i]	;
	}
}
template<typename T> 
unsigned long CLuce::ThreadAddFunc(void* ptr)
{
	ThreadAddSourceData* pData = (ThreadAddSourceData*)ptr;
	unsigned char* pSrcRow(
		pData->pLuce->input.pBits + 
		pData->left*pData->pLuce->input.iPixelSize +
		pData->top*pData->pLuce->input.iStride);
	unsigned char* pDestRow(
		pData->pLuce->output.pBits + 
		pData->left*pData->pLuce->output.iPixelSize +
		pData->top*pData->pLuce->output.iStride);
	unsigned char* pAlphaRow(
		pData->pLuce->alpha.pBits + 
		pData->left*pData->pLuce->alpha.iPixelSize +
		pData->top*pData->pLuce->alpha.iStride);
	for(long y=pData->top;y<pData->bottom;y++)
	{
		unsigned char* pSrc(pSrcRow);
		unsigned char* pDest(pDestRow);
		unsigned char* pAlpha(pAlphaRow);
		for(long x=pData->left;x<pData->right;x++)
		{
			float a = 1;
			if( pAlpha != 0)
			{
				a = pData->pLuce->GetValue<T>(pAlpha,0);
			}
			for(long c=0; c<pData->pLuce->numchannels; c++)
			{
				float src = pData->pLuce->GetValue<T>(pSrc,c) - pData->srcDelta[c];
				if( c != pData->pLuce->alphaInInputChannel )
					src *= a;
				if( src < 0 ) 
					src *= pData->srcNegativeFactor[c];
				else
					src *= pData->srcPositiveFactor[c];
				float dst = pData->pLuce->GetValue<T>(pDest,c);
				pData->pLuce->SetValue<T>(pDest,c,src+dst);
			}
			pSrc += pData->pLuce->input.iPixelSize;
			pDest += pData->pLuce->output.iPixelSize;
			pAlpha += pData->pLuce->alpha.iPixelSize;
		}
		pSrcRow += pData->pLuce->input.iStride;
		pDestRow += pData->pLuce->output.iStride;
		pAlphaRow += pData->pLuce->alpha.iStride;
	}
	return 1;
}

template<typename T> 
void CLuce::AddSource()
{
	float delta[5]={0,0,0,0,0};
	float srcPositiveFactor[5] = {1.f,1.f,1.f,1.f,1.f};
	float srcNegativeFactor[5] = {1.f,1.f,1.f,1.f,1.f};
	if( pOpt->IsZeroMode() && pOpt->IsSrcApplyZero() )
	{
		for(long c=0;c<numchannels;c++)
		{
			delta[c] = pOpt->GetZero(c+currentZero);
			if( pOpt->IsSrcNormalizePositive() )
			{
				if( delta[c] < 1-1e-3f)
					srcPositiveFactor[c] /= (1.f-pOpt->GetZero(c+currentZero));
			}
			if( pOpt->IsSrcNormalizeNegative() )
			{
				if( delta[c] > 1e-3f)
					srcNegativeFactor[c] /= pOpt->GetZero(c+currentZero);
			}
		}
	}

	std::vector<ThreadAddSourceData> tmp;
	tmp.reserve(pOpt->numThreadsToUse);
	if( width >= height )
	{
		int start = 0;
		for(int i=0;i<pOpt->numThreadsToUse;i++)
		{
			int end = MulDiv(width, (i+1), pOpt->numThreadsToUse );
			tmp.push_back(ThreadAddSourceData(this,start,0,end,height,
				delta,srcPositiveFactor,srcNegativeFactor));
			start = end;
		}
	} else
	{
		int start = 0;
		for(int i=0;i<pOpt->numThreadsToUse;i++)
		{
			int end = MulDiv(height, (i+1), pOpt->numThreadsToUse );
			tmp.push_back(ThreadAddSourceData(this,0,start,width,end,
				delta,srcPositiveFactor,srcNegativeFactor));
			start = end;
		}
	}
	if( pOpt->numThreadsToUse > 1 )
	{
		std::vector<HANDLE> tt;
		tt.resize(pOpt->numThreadsToUse);
		for(int i=0;i<pOpt->numThreadsToUse;i++)
			tt[i]=CreateThread(0,0,ThreadAddFunc<T>,&tmp[i],0,0);
		WaitForMultipleObjects(pOpt->numThreadsToUse,&tt[0],TRUE,INFINITE);
	} else
	{
		ThreadAddFunc<T>(&tmp[0]);
	}
}

template<typename T> 
void CLuce::Do()
{
	if( pOpt == 0 ) 
		return;

#ifdef SHOWPERFORMANCE
	LARGE_INTEGER frequency,startValue,current;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&startValue);
#endif //SHOWPERFORMANCE
	positiveFactor = pOpt->GetPositiveIntensity();
	negativeFactor = pOpt->GetNegativeIntensity();
	if( pOpt->IsZeroMode() )
	{
		if( pOpt->IsNormalizePositive() )
		{
			if( pOpt->GetZero(currentZero) < 1-1e-3f)
				positiveFactor /= (1.f-pOpt->GetZero(currentZero));
		}
		if( pOpt->IsNormalizeNegative() )
		{
			if( pOpt->GetZero(currentZero) > 1e-3f)
				negativeFactor /= pOpt->GetZero(currentZero);
		}
	}
	// alphaInInputChannel
	if(	alphaInInputChannel>=0 && 
		alphaInInputChannel<numchannels)
	{
		alpha.iPixelSize = input.iPixelSize;
		alpha.iStride = input.iStride;
		alpha.pBits = input.pBits + alphaInInputChannel * input.iPixelSize;
	}
	if( alpha.pBits == NULL )
	{
		alpha.iPixelSize = 0;
		alpha.iStride = 0;
	}
	if( pOpt->IsPointLight() )
	{
		DoPoint<T>();
	} else
	{
		DoDirectional<T>();
	}

	if( pOpt->IsAddSource() )
	{
		AddSource<T>();
	}

	CheckProgress(1.00f);

#ifdef SHOWPERFORMANCE
	char buff[100];
    QueryPerformanceCounter(&current);
	sprintf(buff,"Luce duration: %f\n", 
		(double)(current.QuadPart - startValue.QuadPart) / (double)frequency.QuadPart);
	OutputDebugStr(buff);
#endif //SHOWPERFORMANCE
}


void CLuce::SetupLightPointers(long _lx,long _ly,
			unsigned char*& _lightInput,
			unsigned char*& _lightOutput,
			unsigned char*& _lightAlpha )
{
	_lightInput = &(input.pBits[_lx * input.iPixelSize + _ly * input.iStride]);
	_lightOutput = &(output.pBits[_lx * output.iPixelSize + _ly * output.iStride]);
	if( alpha.pBits )
	{
		_lightAlpha =  &(alpha.pBits[_lx * alpha.iPixelSize + _ly * alpha.iStride]);
	} else 
	{
		_lightAlpha = NULL;
	}
}

CLuce::CLuce()
{
	width = 0;
	height = 0;
	numchannels = 1;
	alphaInInputChannel = -1;
	input.pBits = NULL;
	output.pBits = NULL;
	alpha.pBits = NULL;
	pProgress = NULL;

	pOpt = 0;
	currentZero = 0;
};

void CLuce::Do8Bit() { Do<unsigned char>(); }
void CLuce::Do16Bit() { Do<unsigned short>(); }
void CLuce::Do32Bit() { Do<float>(); }

void CLuce::SetProgressFn(ProgressFn *fn,float minDelta)
{
	fProgressMinDeltaTime = minDelta;
	pProgress = fn;
}

void CLuce::CheckProgress(float v)
{
	if( pProgress==NULL ) return;
	pProgress(v);
}
