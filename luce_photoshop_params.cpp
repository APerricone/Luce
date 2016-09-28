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
#include "luce_photoshop_params.h"
#include "luce.h"
#include <float.h>
#define _USE_MATH_DEFINES
#include <math.h>


OSErr CPhotoshopLuce::ReadScriptParameters()
{
	OSErr err = noErr;

	PIDescriptorParameters* pDescParams = pFilterRecord->descriptorParameters;
	if (pDescParams == NULL) return nilHandleErr;
	
	ReadDescriptorProcs* pReadProcs = pDescParams->readDescriptorProcs;
	if (pReadProcs == NULL) return nilHandleErr;

	if (pDescParams->descriptor != NULL)
	{
		DescriptorKeyIDArray array = ALLKEYS;
		PIReadDescriptor token = pReadProcs->openReadDescriptorProc(pDescParams->descriptor, array);
		if( token )
		{
			DescriptorKeyID key = 0;
			DescriptorTypeID type = 0;
			int32 flags = 0;
			double x = DBL_MAX;
			double y = DBL_MAX;
			while(pReadProcs->getKeyProc(token, &key, &type, &flags) && !err)
			{
				switch (key)
				{
				case keyType:
					{
						DescriptorEnumID value;
						err = pReadProcs->getEnumeratedProc(token, &value);
						if (!err)
						{
							if( value == typePoint )
								pParams->SetPointLight();
							else
							if( value == typeDirectional )
								pParams->SetDirectionaLight();
						}
					}
					break;
				case keyXPos:
					err = pReadProcs->getFloatProc(token, &x);
					if( !err && y != DBL_MAX )
						pParams->SetCenter(x,y);
					break;
				case keyYPos:
					err = pReadProcs->getFloatProc(token, &y);
					if( !err && x != DBL_MAX )
						pParams->SetCenter(x,y);
					break;
				case keyAngle:
					{
						double angle;
						err = pReadProcs->getFloatProc(token, &angle);
						if( !err )
						{
							angle *= M_PI / 180.;
							pParams->SetDirection(
								(float)(-cos(angle)),
								(float)(sin(angle)));
						}
					}
					break;
				case keyAttenuation:
					{
						DescriptorEnumID value;
						err = pReadProcs->getEnumeratedProc(token, &value);
						if (!err)
						{
							if( value == typeLinear )
								pParams->SetLinearAttenuation();
							else
							if( value == typeQuadratic )
								pParams->SetQuadraticAttenuation();
						}
					}
					break;
				case keyAlgorithm:
					{
						DescriptorEnumID value;
						err = pReadProcs->getEnumeratedProc(token, &value);
						if (!err)
						{
							if( value == typeZero )
								pParams->SetZeroMode();
							else
							if( value == typeLightForm )
								pParams->SetLightformMode();
						}
					}
					break;
				case keyPositiveScale:
					{
						double scale;
						err = pReadProcs->getFloatProc(token, &scale);
						if( !err )
						{
							pParams->SetPositiveIntensity(float(scale/100.));
						}
					}
					break;
				case keyNegativeScale:
					{
						double scale;
						err = pReadProcs->getFloatProc(token, &scale);
						if( !err )
						{
							pParams->SetNegativeIntensity(float(scale/100.));
						}
					}
					break;
				case keyPositiveNormalize:
					{
						Boolean value;
						err = pReadProcs->getBooleanProc(token, &value);
						if( !err )
						{
							if(value)
								pParams->SetNormalizePositive();
							else
								pParams->SetMantainPositiveScale();
						}
					}
					break;
				case keyNegativeNormalize:
					{
						Boolean value;
						err = pReadProcs->getBooleanProc(token, &value);
						if( !err )
						{
							if(value)
								pParams->SetNormalizeNegative();
							else
								pParams->SetMantainNegativeScale();
						}
					}
					break;
				case keyAddSource:
					{
						Boolean value;
						err = pReadProcs->getBooleanProc(token, &value);
						if( !err )
						{
							if(value)
								pParams->SetAddSource();
							else
								pParams->SetNotAddSource();
						}
					}
					break;
				case keySourceZero:
					{
						Boolean value;
						err = pReadProcs->getBooleanProc(token, &value);
						if( !err )
						{
							if(value)
								pParams->SetSrcApplyZero();
							else
								pParams->SetSrcIgnoreZero();
						}
					}
					break;
				case keySourcePositiveNormalize:
					{
						Boolean value;
						err = pReadProcs->getBooleanProc(token, &value);
						if( !err )
						{
							if(value)
								pParams->SetSrcNormalizePositive();
							else
								pParams->SetSrcMantainPositiveScale();
						}
					}
					break;
				case keySourceNegativeNormalize:
					{
						Boolean value;
						err = pReadProcs->getBooleanProc(token, &value);
						if( !err )
						{
							if(value)
								pParams->SetSrcNormalizeNegative();
							else
								pParams->SetSrcMantainNegativeScale();
						}
					}
					break;
				case keyZero0:
				case keyZero1:
				case keyZero2:
				case keyZero3:
				case keyZero4:
					{
						int idx;
						switch (key)
						{
							case keyZero0: idx=0; break;
							case keyZero1: idx=1; break;
							case keyZero2: idx=2; break;
							case keyZero3: idx=3; break;
							case keyZero4: idx=4; break;
						}
						double value;
						err = pReadProcs->getFloatProc(token, &value);
						if( !err )
						{
							pParams->SetZero(idx,(float)(value));
						}
					}
					break;
				}
			}
			pFilterRecord->handleProcs->disposeProc(pDescParams->descriptor);
		}

		pDescParams->descriptor = NULL;
		bWillAskParams = pDescParams->playInfo == plugInDialogDisplay;
	}
	return err;
}

OSErr CPhotoshopLuce::WriteScriptParameters()
{
	PIDescriptorParameters* pDescParams = pFilterRecord->descriptorParameters;
	if (pDescParams == NULL) return nilHandleErr;
	
	WriteDescriptorProcs* pWriteProcs = pDescParams->writeDescriptorProcs;
	if (pWriteProcs== NULL) return nilHandleErr;
	PIWriteDescriptor token = pWriteProcs->openWriteDescriptorProc();
	if (token != NULL)
	{
		if( pParams->IsPointLight() ) 
		{
			pWriteProcs->putEnumeratedProc(token, keyType, typeLight, typePoint);
			double x = double(pParams->GetCenter().x);
			pWriteProcs->putFloatProc(token, keyXPos, &x);
			double y = double(pParams->GetCenter().y);
			pWriteProcs->putFloatProc(token, keyYPos, &y);
		}
		if( pParams->IsDirectionaLight() ) 
		{
			pWriteProcs->putEnumeratedProc(token, keyType, typeLight, typeDirectional);
			double a = atan2(pParams->GetDirection().y,-pParams->GetDirection().x);
			a = a * 180.f / M_PI;
			pWriteProcs->putFloatProc(token, keyAngle, &a);
		}
		if( pParams->IsLinearAttenuation() )
		{
			pWriteProcs->putEnumeratedProc(token, keyAttenuation, typeAttenuation, typeLinear);
		}
		if( pParams->IsQuadraticAttenuation() )
		{
			pWriteProcs->putEnumeratedProc(token, keyAttenuation, typeAttenuation, typeQuadratic);
		}
		if( pParams->IsZeroMode() )
		{
			pWriteProcs->putEnumeratedProc(token, keyAlgorithm, typeAlgorithm, typeZero);
		}
		if( pParams->IsLightformMode() )
		{
			pWriteProcs->putEnumeratedProc(token, keyAlgorithm, typeAlgorithm, typeLightForm);
		}
		double v;
		v = pParams->GetPositiveIntensity() * 100.;
		pWriteProcs->putFloatProc(token, keyPositiveScale, &v);
		v = pParams->GetNegativeIntensity() * 100.;
		pWriteProcs->putFloatProc(token, keyNegativeScale, &v);
		pWriteProcs->putBooleanProc(token, keyPositiveNormalize, pParams->IsNormalizePositive() );
		pWriteProcs->putBooleanProc(token, keyNegativeNormalize, pParams->IsNormalizeNegative() );
		pWriteProcs->putBooleanProc(token, keyAddSource, pParams->IsAddSource() );
		pWriteProcs->putBooleanProc(token, keySourceZero, pParams->IsSrcApplyZero() );
		pWriteProcs->putBooleanProc(token, keySourcePositiveNormalize, pParams->IsSrcNormalizePositive() );
		pWriteProcs->putBooleanProc(token, keySourceNegativeNormalize, pParams->IsSrcNormalizeNegative() );
		v = pParams->GetZero(0); pWriteProcs->putFloatProc(token, keyZero0, &v);
		v = pParams->GetZero(1); pWriteProcs->putFloatProc(token, keyZero1, &v);
		v = pParams->GetZero(2); pWriteProcs->putFloatProc(token, keyZero2, &v);
		v = pParams->GetZero(3); pWriteProcs->putFloatProc(token, keyZero3, &v);
		v = pParams->GetZero(4); pWriteProcs->putFloatProc(token, keyZero4, &v);

		pFilterRecord->handleProcs->disposeProc(pDescParams->descriptor);
		PIDescriptorHandle hDesc;
		pWriteProcs->closeWriteDescriptorProc(token, &hDesc);
		pDescParams->descriptor = hDesc;
	}
	pDescParams->recordInfo = plugInDialogOptional;

	return noErr;
}

OSErr CPhotoshopLuce::ReadRegistryParameters()
{
	OSErr err = noErr;
	AutoSuite<PSDescriptorRegistryProcs> 
		oRegistry(kPSDescriptorRegistrySuite, kPSDescriptorRegistrySuiteVersion);
	AutoSuite<PSActionDescriptorProcs> 
		oActions(kPSActionDescriptorSuite, kPSActionDescriptorSuiteVersion);
	
	if( !oRegistry.IsSupported() ) return errPlugInHostInsufficient;
	if( !oActions.IsSupported() ) return errPlugInHostInsufficient;

	PIActionDescriptor desc = NULL;
	err = oRegistry->Get(plugInUniqueID, &desc);
	if(err) return err;
	if(desc== NULL) return nilHandleErr;

	Boolean b;
	real64 r;
	DescriptorEnumTypeID typeId;
	DescriptorEnumID enumId;
	// type
	err = oActions->GetEnumerated(desc, keyType, &typeId, &enumId );
	if( err ) return err;
	if( enumId == typePoint )		pParams->SetPointLight();		else
	if( enumId == typeDirectional )	pParams->SetDirectionaLight();
	// position
	if( pParams->IsPointLight() )
	{
		double x,y;
		err = oActions->GetFloat(desc,keyXPos, &x);
		if( err ) return err;
		err = oActions->GetFloat(desc,keyYPos, &y);
		if( err ) return err;
		pParams->SetCenter(x,y);
	}
	// direction
	if( pParams->IsDirectionaLight() )
	{
		err = oActions->GetFloat(desc,keyAngle, &r);
		if( err ) return err;
		r *= M_PI / 180.;
		pParams->SetDirection((float)(-cos(r)),(float)(sin(r)));
	}
	// attenuation
	err = oActions->GetEnumerated(desc, keyAttenuation, &typeId, &enumId );
	if( err ) return err;
	if( enumId == typeLinear )		pParams->SetLinearAttenuation();	else
	if( enumId == typeQuadratic )	pParams->SetQuadraticAttenuation();
	// algorithm
	err = oActions->GetEnumerated(desc, keyAlgorithm, &typeId, &enumId );
	if( err ) return err;
	if( enumId == typeZero )		pParams->SetZeroMode();	else
	if( enumId == typeLightForm )	pParams->SetLightformMode();
	// scales
	err = oActions->GetFloat(desc,keyPositiveScale, &r);
	if( err ) return err;
	pParams->SetPositiveIntensity(float(r/100.));
	err = oActions->GetFloat(desc,keyNegativeScale, &r);
	if( err ) return err;
	pParams->SetNegativeIntensity(float(r/100.));
	// zero params
	err = oActions->GetBoolean(desc,keyPositiveNormalize, &b);
	if( err ) return err;
	if( b ) pParams->SetNormalizePositive();
	else	pParams->SetMantainPositiveScale();
	err = oActions->GetBoolean(desc,keyNegativeNormalize, &b);
	if( err ) return err;
	if( b ) pParams->SetNormalizeNegative();
	else	pParams->SetMantainNegativeScale();
	// add source options
	err = oActions->GetBoolean(desc,keyAddSource, &b);
	if( err ) return err;
	if( b ) pParams->SetAddSource();
	else	pParams->SetNotAddSource();
	err = oActions->GetBoolean(desc,keySourceZero, &b);
	if( err ) return err;
	if( b ) pParams->SetSrcApplyZero();
	else	pParams->SetSrcIgnoreZero();
	err = oActions->GetBoolean(desc,keySourcePositiveNormalize, &b);
	if( err ) return err;
	if( b ) pParams->SetSrcNormalizePositive();
	else	pParams->SetSrcMantainPositiveScale();
	err = oActions->GetBoolean(desc,keySourceNegativeNormalize, &b);
	if( err ) return err;
	if( b ) pParams->SetSrcNormalizeNegative();
	else	pParams->SetSrcMantainNegativeScale();
	// zero values
	err = oActions->GetFloat(desc,keyZero0, &r);
	if( err ) return err;
	pParams->SetZero(0,float(r));
	err = oActions->GetFloat(desc,keyZero1, &r);
	if( err ) return err;
	pParams->SetZero(1,float(r));
	err = oActions->GetFloat(desc,keyZero2, &r);
	if( err ) return err;
	pParams->SetZero(2,float(r));
	err = oActions->GetFloat(desc,keyZero3, &r);
	if( err ) return err;
	pParams->SetZero(3,float(r));
	err = oActions->GetFloat(desc,keyZero4, &r);
	if( err ) return err;
	pParams->SetZero(4,float(r));

	int32 i;
	err = oActions->GetInteger(desc,keyNumThread, &i);
	if( err ) return err;
	pParams->SetNumThreadsToUse((short)(i));

	return err;
}

OSErr CPhotoshopLuce::WriteRegistryParameters()
{
	OSErr err = noErr;

	AutoSuite<PSDescriptorRegistryProcs> 
		oRegistry(kPSDescriptorRegistrySuite, kPSDescriptorRegistrySuiteVersion);
	AutoSuite<PSActionDescriptorProcs> 
		oActions(kPSActionDescriptorSuite, kPSActionDescriptorSuiteVersion);

	if( !oRegistry.IsSupported() ) return errPlugInHostInsufficient;
	if( !oActions.IsSupported() ) return errPlugInHostInsufficient;

	PIActionDescriptor desc = NULL;
	err = oActions->Make(&desc);
	if (err) return err;

	if( pParams->IsPointLight() ) 
	{
		err = oActions->PutEnumerated(desc, keyType, typeLight, typePoint);
		if (err) return err;
		err = oActions->PutFloat(desc, keyXPos, double(pParams->GetCenter().x));
		if (err) return err;
		err = oActions->PutFloat(desc, keyYPos, double(pParams->GetCenter().y));
		if (err) return err;
	}
	if( pParams->IsDirectionaLight() ) 
	{
		oActions->PutEnumerated(desc, keyType, typeLight, typeDirectional);
		double a = atan2(pParams->GetDirection().y,-pParams->GetDirection().x);
		a = a * 180.f / M_PI;
		oActions->PutFloat(desc, keyAngle, a);
	}
	if( pParams->IsLinearAttenuation() )
	{
		oActions->PutEnumerated(desc, keyAttenuation, typeAttenuation, typeLinear);
	}
	if( pParams->IsQuadraticAttenuation() )
	{
		oActions->PutEnumerated(desc, keyAttenuation, typeAttenuation, typeQuadratic);
	}
	if( pParams->IsZeroMode() )
	{
		oActions->PutEnumerated(desc, keyAlgorithm, typeAlgorithm, typeZero);
	}
	if( pParams->IsLightformMode() )
	{
		oActions->PutEnumerated(desc, keyAlgorithm, typeAlgorithm, typeLightForm);
	}
	oActions->PutFloat(desc, keyPositiveScale, pParams->GetPositiveIntensity() * 100.);
	oActions->PutFloat(desc, keyNegativeScale, pParams->GetNegativeIntensity() * 100.);
	oActions->PutBoolean(desc, keyPositiveNormalize, pParams->IsNormalizePositive() );
	oActions->PutBoolean(desc, keyNegativeNormalize, pParams->IsNormalizeNegative() );
	oActions->PutBoolean(desc, keyAddSource, pParams->IsAddSource() );
	oActions->PutBoolean(desc, keySourceZero, pParams->IsSrcApplyZero() );
	oActions->PutBoolean(desc, keySourcePositiveNormalize, pParams->IsSrcNormalizePositive() );
	oActions->PutBoolean(desc, keySourceNegativeNormalize, pParams->IsSrcNormalizeNegative() );
	oActions->PutFloat(desc, keyZero0, pParams->GetZero(0));
	oActions->PutFloat(desc, keyZero1, pParams->GetZero(1));
	oActions->PutFloat(desc, keyZero2, pParams->GetZero(2));
	oActions->PutFloat(desc, keyZero3, pParams->GetZero(3));
	oActions->PutFloat(desc, keyZero4, pParams->GetZero(4));

	oActions->PutInteger(desc, keyNumThread, pParams->GetNumThreadsToUse());

	err = oRegistry->Register(plugInUniqueID, desc, true);
	return err;
}
