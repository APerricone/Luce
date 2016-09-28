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
#include <cstddef>

//#define SHOWPERFORMANCE

struct LucePointDouble
{
	double x;
	double y;
};

struct LucePointFloat
{
	float x;
	float y;
};

class CLuce;
class LuceOptions
{
	friend class CLuce;
public:
	void InitDefault() 
	{
		pointLight = true;
		quadraticAttenuation = false;
		zeroMode = true;
		addSource = true;
		sourceZero = true;
		sourcePositiveNormalize = true;
		sourceNegativeNormalize = true;
		positiveNormalize = true;
		negativeNormalize = true;
		numThreadsToUse = 1;
		positiveIntensity = 1.0f;
		negativeIntensity = 1.0f;
		center.x = 0.5;
		center.y = 0.5;
		direction.x = -0.707f;
		direction.y = -0.707f;
		zeros[0] = 0.0f;
		zeros[1] = 0.0f;
		zeros[2] = 0.0f;
		zeros[3] = 0.0f;
		zeros[4] = 0.0f;
	}

	const LuceOptions& operator=(const LuceOptions& b)
	{
		pointLight				= b.pointLight				;
		quadraticAttenuation	= b.quadraticAttenuation	;
		zeroMode				= b.zeroMode				;
		addSource				= b.addSource				;
		sourceZero				= b.sourceZero				;
		sourcePositiveNormalize	= b.sourcePositiveNormalize	;
		sourceNegativeNormalize	= b.sourceNegativeNormalize	;
		positiveNormalize		= b.positiveNormalize		;
		negativeNormalize		= b.negativeNormalize		;
		positiveIntensity		= b.positiveIntensity		;
		negativeIntensity		= b.negativeIntensity		;
		numThreadsToUse			= b.numThreadsToUse			;
		center.x				= b.center.x				;
		center.y				= b.center.y				;
		direction.x				= b.direction.x				;
		direction.y				= b.direction.y				;
		zeros[0]				= b.zeros[0]				;
		zeros[1]				= b.zeros[1]				;
		zeros[2]				= b.zeros[2]				;
		zeros[3]				= b.zeros[3]				;
		zeros[4]				= b.zeros[4]				;
		return *this;
	}

	bool IsPointLight()				const { return pointLight; }
	bool IsDirectionaLight()		const { return !pointLight; }
	bool IsQuadraticAttenuation()	const { return quadraticAttenuation; }
	bool IsLinearAttenuation()		const { return !quadraticAttenuation; }
	bool IsZeroMode()				const { return zeroMode; }
	bool IsLightformMode()			const { return !zeroMode; }

	bool IsNormalizePositive()		const { return positiveNormalize; }
	bool IsMantainPositiveScale()	const { return !positiveNormalize; }
	bool IsNormalizeNegative()		const { return negativeNormalize; }
	bool IsMantainNegativeScale()	const { return !negativeNormalize; }
	float GetPositiveIntensity()	const { return positiveIntensity; }
	float GetNegativeIntensity()	const { return negativeIntensity; }

	bool IsAddSource()				const { return addSource; }
	bool IsNotAddSource()			const { return !addSource; }
	bool IsSrcApplyZero()			const { return sourceZero; }
	bool IsSrcIgnoreZero()			const { return !sourceZero; }
	bool IsSrcNormalizePositive()	const { return sourcePositiveNormalize; }
	bool IsSrcMantainPositiveScale()const { return !sourcePositiveNormalize; }
	bool IsSrcNormalizeNegative()	const { return sourceNegativeNormalize; }
	bool IsSrcMantainNegativeScale()const { return !sourceNegativeNormalize; }

	unsigned short GetNumThreadsToUse()		const { return numThreadsToUse; }

	const LucePointDouble& GetCenter()	const { return center; }
	const LucePointFloat& GetDirection() const { return direction; }
	float GetZero(int id)		const { return zeros[id]; }

	void SetPointLight()			{ pointLight = true; }
	void SetDirectionaLight()		{ pointLight = false; }
	void SetQuadraticAttenuation()	{ quadraticAttenuation  = true; }
	void SetLinearAttenuation()		{ quadraticAttenuation = false; }
	void SetZeroMode()				{ zeroMode = true; }
	void SetLightformMode()			{ zeroMode = false; }

	void SetNormalizePositive()		{ positiveNormalize = true; }
	void SetMantainPositiveScale()	{ positiveNormalize = false; }
	void SetNormalizeNegative()		{ negativeNormalize = true; }
	void SetMantainNegativeScale()	{ negativeNormalize = false; }
	void SetPositiveIntensity(float v)	{ positiveIntensity = v; }
	void SetNegativeIntensity(float v)	{ negativeIntensity = v; }

	void SetAddSource()				{ addSource = true; }
	void SetNotAddSource()			{ addSource = false; }
	void SetSrcApplyZero()			{ sourceZero = true; }
	void SetSrcIgnoreZero()			{ sourceZero = false; }
	void SetSrcNormalizePositive()	{ sourcePositiveNormalize = true; }
	void SetSrcMantainPositiveScale(){ sourcePositiveNormalize = false; }
	void SetSrcNormalizeNegative()	{ sourceNegativeNormalize = true; }
	void SetSrcMantainNegativeScale(){ sourceNegativeNormalize = false; }

	void SetNumThreadsToUse(unsigned short n);

	void SetCenter(double x,double y) { center.x=x; center.y=y;  } 
	void SetDirection(float x,float y);
	void SetZero(int id,float v) { zeros[id] = v; }

private:
	//! If true it is a point light
	bool pointLight:1;
	//! If true the attenuation with distance is quadratic
	bool quadraticAttenuation:1;
	//! If true the algorithm is zeromode
	bool zeroMode:1;
	//! If true, on the values between zero and whire are normalize like black 
	//! to white
	bool positiveNormalize:1;
	//! If true, on the values between zero and whire are normalize like black 
	//! to white
	bool negativeNormalize:1;
	//! If true the source will be added to luce result
	bool addSource:1;
	//! If true, when add the source the zero color (and negatives) is converted 
	//! to black
	bool sourceZero:1;
	//! If true, when add the source the color between zero color and white is 
	//! scaled to be black to white
	bool sourcePositiveNormalize:1;
	//! If true, when add the source the color between black and zero color is 
	//! scaled to be black to white negative
	bool sourceNegativeNormalize:1;
	//! Multiplication factor for positive value (further normalize)
	float positiveIntensity;
	//! Multiplication factor for negative value (further normalize)
	float negativeIntensity;

	//! position in case of point light, it is percent of image 
	//! it is double to support large images
	LucePointDouble center;
	//! direction in case of directional light
	LucePointFloat direction;

	float zeros[5];

	//! If true the plugin will use the tinythread library
	unsigned short numThreadsToUse;
};


struct BitmapData
{
	//! difference for x+1
	ptrdiff_t iPixelSize;
	//! difference for y+1
	ptrdiff_t iStride;
	//! data
	unsigned char* pBits;
};

class CLuce
{
public:

	typedef void ProgressFn(float v);
	void SetProgressFn(ProgressFn *fn,float minDelta = 0.1f);

	CLuce();

	long width;
	long height;
	long numchannels;
	long alphaInInputChannel; //outside 0<=x<numchannels if the alpha is not in input.
	BitmapData input;
	BitmapData output;
	BitmapData alpha;

	const LuceOptions *pOpt;
	int currentZero;

	// Change data applying luce
	void Do8Bit();
	void Do16Bit();
	void Do32Bit();

private:
	// useful templates
	template<typename T> float GetValue(void* ptr,int idx);
	template<typename T> void SetValue(void* ptr,int idx,float v);
	template<typename T> void Line(long px,long py);
	template<typename T> void DoPoint();
	template<typename T> void DoDirectional();
	template<typename T> void CallThreads();
	template<typename T> void AddSource();
	template<typename T> void Do();

	// starting points
	unsigned char* lightInput;
	unsigned char* lightOutput;
	unsigned char* lightAlpha;
	long lx;
	long ly;
	void SetupLightPointers(long _lx,long _ly,
			unsigned char*& _lightInput,
			unsigned char*& _lightOutput,
			unsigned char*& _lightAlpha );

	// progress indicator support
	float fProgressMinDeltaTime;
	ProgressFn *pProgress;
	void CheckProgress(float v);
	
	// calculated scale
	float positiveFactor;
	float negativeFactor;

	// an extra scale factor for directional light
	float dirScale;
	
	// extra optimizations
	bool doLeft:1;
	bool doTop:1;
	bool doRight:1;
	bool doBottom:1;

	struct ThreadLightData
	{
		CLuce *pLuce;
		long startPixel;
		long endPixel; //actually minus+1
		ThreadLightData(CLuce *pLuce,long startPixel,long endPixel);
	};

	template<typename T> static unsigned long __stdcall ThreadLightFunc(void*);

	struct ThreadAddSourceData
	{
		CLuce *pLuce;
		long left,top,right,bottom;
		float srcDelta[5];
		float srcPositiveFactor[5];
		float srcNegativeFactor[5];
		ThreadAddSourceData(CLuce *pLuce,
			long left,long top,long right,long bottom,
			float* srcDelta,float* srcPositiveFactor,
			float* srcNegativeFactor);
	};
	template<typename T> static unsigned long __stdcall ThreadAddFunc(void*);

	struct Rect
	{
		long left,top;
		long right,bottom;
	};
};
